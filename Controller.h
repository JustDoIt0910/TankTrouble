//
// Created by zr on 23-3-3.
//

#ifndef TANK_TROUBLE_CONTROLLER_H
#define TANK_TROUBLE_CONTROLLER_H

#include <memory>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "Tank.h"
#include "Block.h"
#include "reactor/EventLoop.h"

namespace TankTrouble
{
    class Controller
    {
    public:
        typedef std::unique_ptr<Object> ObjectPtr;
        typedef std::unordered_map<int, ObjectPtr> ObjectList;
        typedef std::shared_ptr<ObjectList> ObjectListPtr;

        typedef std::unordered_map<int, Block> BlockList;

        Controller():
            started(false),
            controlLoop(nullptr),
            snapshot(new ObjectList) {}

        virtual void start() = 0;
        virtual ObjectListPtr getObjects() = 0;
        virtual void dispatchEvent(ev::Event* event) = 0;
        virtual BlockList* getBlocks() = 0;
        virtual ~Controller() = default;

    protected:
        ObjectList objects;
        ObjectListPtr snapshot;
        std::vector<int> deletedObjs;
        BlockList blocks;
        std::mutex mu;
        std::condition_variable cv;
        bool started;
        std::thread controlThread;
        ev::reactor::EventLoop* controlLoop;
    };
}

#endif //TANK_TROUBLE_CONTROLLER_H
