//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_CONTROLLER_H
#define TANK_TROUBLE_CONTROLLER_H
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <condition_variable>
#include "reactor/EventLoop.h"
#include "Object.h"
#include "util/Vec.h"
#include "Tank.h"
#include "Block.h"
#include "defs.h"

#define UPWARDS             0
#define UPWARDS_LEFT        1
#define LEFT                2
#define DOWNWARDS_LEFT      3
#define DOWNWARDS           4
#define DOWNWARDS_RIGHT     5
#define RIGHT               6
#define UPWARDS_RIGHT       7

namespace TankTrouble
{
    class Controller {
    public:
        typedef std::unique_ptr<Object> ObjectPtr;
        typedef std::unordered_map<int, ObjectPtr> ObjectList;
        typedef std::shared_ptr<ObjectList> ObjectListPtr;

        typedef std::unordered_map<int, Block> BlockList;

        Controller();
        ~Controller();
        void start();
        ObjectListPtr getObjects();
        void dispatchEvent(ev::Event* event);
        BlockList* getBlocks();

    private:
        void run();
        void moveAll();
        void controlEventHandler(ev::Event* event);
        void fire(Tank* tank);
        int checkShellCollision(const Object::PosInfo& curPos, const Object::PosInfo& nextPos);
        int checkTankCollision(Tank* tank, const Object::PosInfo& curPos, const Object::PosInfo& nextPos);
        Object::PosInfo getBouncedPosition(const Object::PosInfo& cur, const Object::PosInfo& next, int blockId);

        void initBlocks(int num);

        ObjectList objects;
        ObjectListPtr snapshot;
        std::vector<int> deletedObjs;
        BlockList blocks;
        std::mutex mu;
        std::condition_variable cv;
        bool started;
        std::thread controlThread;
        ev::reactor::EventLoop* controlLoop;
        std::vector<int> possibleCollisionBlocks[HORIZON_GRID_NUMBER][VERTICAL_GRID_NUMBER][8];
        int tankNum;
    };
}

#endif //TANK_TROUBLE_CONTROLLER_H
