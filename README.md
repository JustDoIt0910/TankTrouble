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

​		对于单机模式，游戏逻辑主要在LocalController中，网络模式中这部分被移到了服务器的GameRoom中，控制一个房间里的游戏进行。

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

​		pos 是中心坐标，angle是相对于x轴正半轴的顺时针旋转角(°)，所有与游戏对象相关的数学计算也都离不开这两个参数。

​		除此之外一个对象一定有其移动状态，对于坦克来说，有前进，后退，顺时针旋转，逆时针旋转等，对于炮弹则只有前进状态。Tank和Shell都实现Object中的虚函数draw(), getCurrentPosition(), getNextPosition()等等，view层拿到的是一个std::unique_ptr<Object> 的多态列表，它只需要对每个对象调用draw()就好了，而不用管对方是什么。Controller同理，不需要知道对象类型就可以拿到位置，状态等信息。



2. **关于线程模型**

   我选择把Controller放到独立于gui主线程的另一个线程中，由我自己的事件驱动库驱动，主要有几个考虑

   - 符合线程的单一职责原则
   - gui线程中不应该执行相对耗时的计算，单机模式中人机的计算量还是比较大的，都塞进gui线程中或许会降低界面响应，网络模式更不用说了，gui线程处理网络通信肯定是不合适的。
   - 可以让view和controller完全解耦，view层不需要知道，也不应该知道当前是LocalController还是OnlineController在向它提供数据，它的职责仅仅是拿到数据，绘制，有键盘事件。向controller报告，view层的代码不需要为两不同Controller作修改。

   所以LocalController和OnlineController都继承Controller，对外暴露统一的接口。区别在于游戏对象是自己管理还是从服务器拿的而已。



### 游戏主要逻辑的实现

​		主逻辑很简单，先由Maze生成随机地图，根据地图初始化游戏中的墙(blocks)，LocalController管理一个多态对象列表，设置定时任务，每隔一定时间将列表中的所有对象移动到下一个位置。这个间隔越短对象移动越快(当然也和移动步长有关)。稍微复杂一点的地方是碰撞检测，炮弹与墙和坦克的碰撞，计算反弹，以及坦克与墙的碰撞等

​		几何图形的碰撞判断都在Math.cc中实现

- **炮弹与墙的碰撞检测和反弹计算**

  ​	其实游戏中的墙是由一根根固定长度和宽度的Block组合而成，所以炮弹与墙碰撞可以简化为圆形和矩形的重叠判断

![p1](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p1.png)

​		图中v1, v2是矩形两个方向的单位向量，当然不要求矩形是水平或者垂直，只是因为游戏中所有Block都是横平竖直的。计算v在两个方向向量上的投影长度，当v在v2上投影长度小于W/2，在v1上投影小于H/2，且矩形中心与圆形中心距离小于R + r时，一定会发生重叠。
