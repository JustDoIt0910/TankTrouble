# TankTrouble: 用c++17打造支持单机/联网的坦克动荡游戏(linux版)

**本仓库是 TankTrouble 的linux PC端，TankTrouble 服务器在另一个仓库中**

[TankTroubleServer](https://github.com/JustDoIt0910/TankTroubleServer)



**入口可以选择玩单机还是联网**

![entry](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/entry.png)



**单机模式下是和一个躲子弹超强的人机打(Agent Smith, 起这个名字是因为它让我想起来黑客帝国里那个会躲子弹的 Agent Smith)**

![single](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/single.png)



**在线模式登录以后进入大厅，可以看到已有的房间，也可以新开房间，支持2/3/4人的房间，人数再多没必要，毕竟地图大小固定了。房间人满后游戏自动开始**

![lobby](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/gameLobby.png)



![online](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/online.png)



### 项目结构

- controller ------------------ 可以理解为后端，负责游戏逻辑，数据更新
  - LocalController.h LocalController.cc --------------- 单机模式下的controller, 负责游戏所有逻辑
  - OnlineController.h OnlineController.cc ----------- 联网模式下的controller, 负责和服务器交互，更新数据
- ev ----------------------------- 参考muduo实现的极简事件驱动网络库，游戏逻辑在单独线程中驱动，独立于gui线程。联网模式下还要负责网络通信
- event ---------------------------------- 将游戏中的操作封装成事件，方便融合进事件驱动模型
- protocol ------------------------------ 通信协议部分，只在联网模式下用到
- smithAI -------------------------------- 人机的所有逻辑，包括危险躲避，索敌，攻击等
- util -------------------------------------- 主要是数学工具，包含游戏中用到的几何、向量计算、碰撞检测等
- view ------------------------------------ 类比前端，所有gui界面
- Controller.h Controller.cc------- LocalController和OnlineController的基类
- Maze.h Maze.cc -------------------- 地图生成算法
- Object.h Object.cc ---------------- 游戏中对象的多态基类
- Tank.h Tank.cc --------------------- 坦克对象，继承Object
- Shell.h Shell.cc --------------------- 炮弹对象，继承Object
- Block.h Block.cc ------------------- 墙，不继承Object，根据地图生成，单独管理
- Window.h Window.cc ----------- gui主类，管理所有views
- defs.h --------------------------------- 游戏中一些宏定义



### 一些项目结构的说明

对于单机模式，游戏逻辑主要在LocalController中，网络模式中这部分被移到了服务器的GameRoom中，控制一个房间里的游戏进行。

1. **数据层面如何定义一个游戏中的对象**

游戏对象要能让view层将其正确的绘制，也要能让Controller获取它的当前位置信息，移动状态，下一刻的位置等

我用一个PosInfo 结构表示一个对象的位置信息

```c++
 struct PosInfo
 {
     PosInfo(const util::Vec& p, double a): pos(p), angle(a){}
     PosInfo& operator=(const PosInfo& info) = default;
     bool operator==(const PosInfo& info) const {return (pos == info.pos && angle == info.angle);}
     PosInfo(): PosInfo(util::Vec(0.0, 0.0), 0){}
     static PosInfo invalid() {return PosInfo{util::Vec(DBL_MAX, DBL_MAX), DBL_MAX};}
     bool isValid() const
     {return (pos.x() != DBL_MAX && pos.y() != DBL_MAX && angle != DBL_MAX);}

     util::Vec pos;

     double angle;
 };
```

pos 是中心坐标，angle是相对于x轴正半轴的顺时针旋转角(°)，所有与游戏对象相关的数学计算也都离不开这两个参数。

除此之外一个对象一定有其移动状态，对于坦克来说，有前进，后退，顺时针旋转，逆时针旋转等，对于炮弹则只有前进状态。Tank和Shell都实现Object中的虚函数draw(), getCurrentPosition(), getNextPosition()等等，view层拿到的是一个std::unique_ptr<Object> 的多态列表，它只需要对每个对象调用draw()就好了，而不用管对方是什么。Controller同理，不需要知道对象类型就可以拿到位置，状态等信息。



2. **关于线程模型**

   我选择把Controller放到独立于gui主线程的另一个线程中，由我自己的事件驱动库驱动，主要有几个考虑

   - 符合线程的单一职责原则
   - gui线程中不应该执行相对耗时的计算，单机模式中人机的计算量还是比较大的，都塞进gui线程中或许会降低界面响应，网络模式更不用说了，gui线程处理网络通信肯定是不合适的。
   - 可以让view和controller完全解耦，view层不需要知道，也不应该知道当前是LocalController还是OnlineController在向它提供数据，它的职责仅仅是拿到数据，绘制，有键盘事件。向controller报告，view层的代码不需要为不同Controller作修改。

   所以LocalController和OnlineController都继承Controller，对外暴露统一的接口。区别在于游戏对象是自己管理还是从服务器拿的而已。



### 游戏主要逻辑的实现

主逻辑很简单，先由Maze生成随机地图，根据地图初始化游戏中的墙(blocks)，LocalController管理一个多态对象列表，设置定时任务，每隔一定时间将列表中的所有对象移动到下一个位置。这个间隔越短对象移动越快(当然也和移动步长有关)。稍微复杂一点的地方是碰撞检测，炮弹与墙和坦克的碰撞，计算反弹，以及坦克与墙的碰撞等

​		几何图形的碰撞判断都在Math.cc中实现

- **炮弹与墙的碰撞检测和反弹计算**

  其实游戏中的墙是由一根根固定长度和宽度的Block组合而成，所以炮弹与墙碰撞可以简化为圆形和矩形的重叠判断

![p1](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p1.png)

图中v1, v2是矩形两个方向的单位向量，当然不要求矩形是水平或者垂直，只是因为游戏中所有Block都是横平竖直的。计算v在两个方向向量上的投影长度，当v在v2上投影长度小于W/2 + r，在v1上投影小于H/2 + r，且矩形中心与圆形中心距离小于R + r时，一定会发生重叠。

那么怎么计算反弹方向呢？我用到一点包围盒的思想，将矩形外围看作有一个由四条线段b1, b2, b3, b4组成的包围盒, 线段与矩形外围的距离是炮弹的半径。

![p2](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p2.png)

在某一次碰撞检测中，可以得到一颗Shell当前的位置和下一步的位置，这两个位置的中心会构成一条线段，将这条线段依次与包围盒的4个边求交点，有交点的那条包围盒线段就是Shell下一步将会碰到的矩形边，根据这条边是水平还是竖直，就可以得到反弹后的方向了。

- **坦克与墙的碰撞检测**

  可以简化为矩形与矩形的重叠检测。这里用到的是投影法。关于投影法判断矩形重叠，这篇文章讲的比较清楚

  [投影法判断旋转矩形重叠](https://blog.csdn.net/tom_221x/article/details/38457757)

- **一点优化措施**

  如果每个对象的每一次移动，都要与地图中所有Block进行碰撞检测，那未免有些蠢，其实一个对象在位置和运动方向确定的情况下，只有一小部分墙壁是可能发生碰撞的，即需要判断的。为此我将地图划分为许多单元格，如图

  ![p3](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p3.png)

  这种情况下只需要判断左上角四个Block就好了。所以可以维护两个可能碰撞表，记录在某个格子，向某个方向运动时的可能碰撞列表(Tank 没有方向这一维，因为它几何形状比较不规则)，在生成地图后，初始化blocks时打表。

  ```c++
  std::vector<int> shellPossibleCollisionBlocks[HORIZON_GRID_NUMBER][VERTICAL_GRID_NUMBER][8];
  std::vector<int> tankPossibleCollisionBlocks[HORIZON_GRID_NUMBER][VERTICAL_GRID_NUMBER];
  ```



### 关于人机

人机是最有趣的部分了。它的逻辑分为危险躲避，接近敌人，瞄准攻击三部分。AgentSmith.cc中的代码负责作出躲避、接近、攻击的最优决策。但是从决策到对象真正移动之间还需要一个执行者，就是smithAI下的各种Strategy了，DodgeStrategy是躲避决策执行者，ContactStrategy是接近决策执行者，AttackStrategy是攻击决策执行者，决策以不同的形式保存在strategy中。

对于DodgeStrategy, 一次完整的躲避决策由一个命令队列表示，比如

​											{ROTATE_CW, 3}, {MOVE_FORWARD, 15}

表示先顺时针旋转三个步长，再前进15个步长。AgentSmith只负责生成这些命令，由DodgeStrategy负责在正确的时间改变坦克的移动状态，也就是执行命令

ContactStrategy和AttackStrategy也类似，只不过一个存的是A*算法生成的路径点，一个存的是瞄准角度而已。总之AgentSmith和Strategy的关系就类似于高级指挥官与基层军官的关系。

- **Smith的躲避决策算法**

  - **威胁检测**

    Smith会定期对距离自己一定范围之内炮弹进行弹道预测，因为炮弹会反弹，需要将弹道分成一段一段的结构考虑。定义"弹道段"结构BallisticSegment

    ```c++
     struct BallisticSegment
     {
         int shellId; //炮弹的id
         int seq;		//段序号，表示当前段是整条弹道中的第几段
         KeyPoint start; //起点坐标
         KeyPoint end; //终点坐标
         util::Vec center;
         double angle; //炮弹在这段弹道上的移动方向
         double length; //这段长度
         double distanceToTarget; //段的起点与smith的距离，这个值决定躲避的优先级
         
         BallisticSegment(int id, int seq, KeyPoint s, KeyPoint e, double len, double a, double dis):
         	shellId(id), seq(seq), start(std::move(s)), end(std::move(e)),
         	length(len), angle(a), distanceToTarget(dis)
             {center = util::Vec((start.second.x() + end.second.x()) / 2,
                                 (start.second.y() + end.second.y()) / 2);}
         
         static BallisticSegment invalid(){return {-1, -1, KeyPoint(), KeyPoint(), 0, 0, 0};}
         
         bool isValid() const {return shellId != -1;}
     };
    ```

    ```c++
    typedef std::vector<BallisticSegment> Ballistic; //一条完整弹道
    typedef std::unordered_map<int, Ballistic> Ballistics; // 炮弹id -> 弹道
    ```

    如何判断某条“弹道段”对自己有威胁呢？可以将一条BallisticSegment视作一个宽度为炮弹直径的长方形，如果它与smith有重叠，那smith就处于这颗炮弹的“炮线”上，这个段将会被放入威胁列表中。

    最后smith会选择躲避起始点距离自己最近的那个段，因为这颗炮弹应该是最早到达的。

    下边这个例子中，两条蓝色段属于一条弹道，两条红色段属于另一条弹道。浅蓝色和深红色的段是有威胁段，由于红色段距离近，所以深红色段是当前要躲避的段。

    ![p4](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p4.png)

    这么看smith似乎很短视，每个时刻只能躲避一颗炮弹，但是实际测试发现这种局部最优解的效果其实是很不错的。

  - **决策生成**

    定位了威胁，接下来就是确定要如何移动才能使得smith与威胁段不相交。除了暴力模拟我没有想出来更好的方法，但是可以通过一些前置的计算来尽量保证模拟中的第一种可行方案即为最优方案，这样可以尽早结束模拟。smith会依次尝试以下三种移动方式：

    - **不移动，只旋转**

      当炮线比较靠近边缘时，旋转一般是耗时最短的选择

    - **先旋转，再移动**

    - **边转弯边移动**