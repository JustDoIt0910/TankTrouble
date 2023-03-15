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
  - LocalController.h LocalController.cc ------------------ 单机模式下的controller, 负责游戏所有逻辑
  - OnlineController.h OnlineController.cc -------------- 联网模式下的controller, 负责和服务器交互，更新数据
- ev ----------------------------- 参考muduo实现的极简事件驱动库，游戏逻辑在单独线程中驱动，独立于gui线程。联网模式下还要负责网络通信
- event ------------------------ 将游戏中的操作封装成事件，方便融合进事件驱动模型
- protocol -------------------- 通信协议部分，只在联网模式下用到
- smithAI --------------------- 人机的所有逻辑，包括危险躲避，索敌，攻击等
- util ---------------------------- 主要是数学工具，包含游戏中用到的几何、向量计算、碰撞检测等
- view -------------------------- 所有gui界面
- Controller.h Controller.cc----------- LocalController和OnlineController的基类
- Maze.h Maze.cc ------------------------ 地图生成算法
- Object.h Object.cc --------------------- 游戏中对象的多态基类
- Tank.h Tank.cc -------------------------- 坦克对象，继承Object
- Shell.h Shell.cc -------------------------- 炮弹对象，继承Object
- Window.h Window.cc ---------------- gui主类，管理所有views
- defs.h -------------------------------------- 游戏中一些宏定义