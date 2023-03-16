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

- #### Smith的躲避决策算法

  LocalController中有个变量对于人机至关重要，globalSteps，64位无符号整数，初始为0，每个移动周期会自增1，smith需要它来推算未来某个时间点炮弹的位置，strategy也需要知道globalSteps才能确定某个命令是否已经执行了足够的步数。

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

      ![p5](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p5.png)

    - **先旋转，再移动**

      仅靠旋转躲不开时，就要尝试移动到炮线的一侧去。那是向左侧躲避最优还是向右侧最优？这就需要一点向量计算了。

      ![p6](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p6.png)

      v是炮弹运动方向向量，v2是从炮弹中心坐标指向坦克中心坐标的向量，根据v与v2的叉乘的正负，可以知道v2在v的左侧还是右侧。明显左边情况向左侧躲避最优，右边情况向右侧躲避最优。

      **注：并不是只要能躲开当前威胁就可行，在模拟每一步时也要计算坦克会不会碰墙，会不会撞上炮弹，如果会，pass掉当前方案，尝试旋转到下一个角度再移动**

    - **边转弯边移动**

      兜底的方案，同样也是模拟。

  - **决策执行**

    每一轮移动中，DodgeStrategy从命令队列头取出命令，判断该命令是否已经执行了足够的步数，如果是，将其弹出，将坦克移动状态设置为下一命令指定的移动状态，这样持续直到命令队列为空。

    **注：smith的决策更新的速度是比较快的，这常常导致一个决策没有执行完就被覆盖成新的，不过这并不是坏事，因为它每次做出的都是局部最优解，一般不会与上一次决策冲突，即使冲突也是因为发现了更优的策略，这会使smith的躲避更加灵活**。

- #### Smith的路线规划

  使用经典的A*算法，在smithAI/AStar.h smithAI/AStar.cc 中实现。生成的路径点存放在ContactStrategy中，由后者判断何时转弯等等。

- #### Smith攻击策略

  进入攻击范围后smith会开始寻找瞄准角度，也是模拟+弹道预测。



### 随机地图生成

使用类似prim最小生成树的算法，初始状态地图中每个格子之间都是墙隔开，从最左上角的格子开始，每次循环随机选一面相邻的墙，打通它，打通一面墙就相当于把墙对面的格子加入了最小生成树中。当然已经被联通的格子不可以再次联通，最终会生成一个迷宫，且每次是随机的。



### 关于ev事件驱动库

这是我在阅读陈硕老师的muduo库后，实现的极简版的事件驱动网络库，实现中用到的一些线程同步工具等尽量使用标准库现有的组件，而没有自己再造，只提供了IPv4的支持。部分地方有小的简化与改动，比如没有向原版那样在线程间传递裸的fd，而是通过Socket类的移动语义转移fd所有权，只支持epoll等等。

reactor ------------------- 事件循环相关，EventLoop, Channel, Epoller, 定时器等等

net ------------------------- 网络相关

utils ----------------------- 时间戳，线程工具，线程池



### 在线模式

1. **网络协议选择**

   对于登录，登出，创建/加入房间，房间、玩家信息的同步，这些操作需要可靠性保证，否则会出现数据不一致，而对实时性要求不是很高，使用TCP，而游戏场景的更新消息对实时性要求极高，使用TCP会导致画面流畅程度极度依赖网络稳定性，显然是不合适的，对于这种实时消息重传并没有意义，少量丢包也是可以允许的，所以我使用一个单独的udp socket来接收游戏场景消息。

   udp是无连接的，server要通过udp向client发消息，就要知道client的公网ip和port，所以在登录成功后需要一个 udp 握手流程：

   ![p7](https://github.com/JustDoIt0910/MarkDownPictures/blob/main/TankTrouble/p7.png)

   client发送握手包后启动定时器，超时没有收到握手回应则重传。

2. **protocol的设计**

   因为我不想再引入类似 protobuf 的第三方序列化库，所以自己封装了一种简单的消息序列化与反序列化机制，在 Message.h 中实现，其实就是为不同类型的字段提供一个包装结构 Field，一个 Message 里保存一个或多个 Field，在序列化时，Message 依次调用每个 Field 的 toByteArray() 方法，每个 Field 将自己的 data转成字节流append到Buffer中，反序列化时，Message依次调用每个 Field 的 fill()  方法，每个Field根据自己data数据类型的长度，各取所需。对于多字节整数，序列化与反序列化过程中都会自动进行字节序转换。

   比如新开房间的消息是这样定义的：

   ```c++
   messages_[MSG_NEW_ROOM] = MessageTemplate({
               new FieldTemplate<std::string>("room_name"),
               new FieldTemplate<uint8_t>("player_num")
           });
   ```

   这里的FieldTemplate, MessageTemplate是用来生成对应类型的Field和Message的，通过调用MessageTemplate 的 getMessage() 方法，就可以得到一个空的Message。为什么需要MessageTemplate呢？因为这样我可以把所有message定义在 Codec 的一个map中，也就是这里的messages_，它的键是消息类型，定义在 Codec.h 中，值是 MessageTemplate。当我需要发送一条新建房间消息，可以这么写：

   ```c++
   Message newRoom = codec.getEmptyMessage(MSG_NEW_ROOM);
   newRoom.setField<Field<std::string>>("room_name", name);
   newRoom.setField<Field<uint8_t>>("player_num", cap);
   Buffer buf = Codec::packMessage(MSG_NEW_ROOM, newRoom);
   client->send(buf);
   ```

   虽然这会使 MessageTemplate 暴露给多个线程，但由于对 MessageTemplate 都是只读访问，所以是安全的。

   每条消息有固定的 header，解码器读取 header 得知消息类型和长度，header 由 Codec 根据Message的长度自动补全。

   ```c++
    struct FixHeader
    {
        uint8_t messageType;
        uint16_t messageLen;
   
        FixHeader() = default;
        FixHeader(uint8_t mt, uint16_t ml): messageType(mt), messageLen(ml) {}
   
        void toByteArray(Buffer* buf) const
        {
            buf->appendInt8(static_cast<int8_t>(messageType));
            buf->appendInt16(static_cast<int16_t>(messageLen));
        }
    };
   ```

   序列化后的数据是这样子的

   **|Type (1byte)|Length (2bytes) | Field1 data |Field2 data |...**

   字符串字段以0结尾

   **|Type (1byte)|Length (2bytes) | Field1(string) data |0x00|Field2 data |...**

   如果payload是数组，序列化时第一个字节是数组元素个数

   **|Type (1byte)|Length (2bytes) | 0x02 |elem1 data|elem2 data |**