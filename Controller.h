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

        Controller();

        virtual void start() = 0;
        ObjectListPtr getObjects();
        void dispatchEvent(ev::Event* event);
        BlockList* getBlocks();
        virtual ~Controller();

    protected:
        ObjectList objects;
        ObjectListPtr snapshot;
        BlockList blocks;
        std::mutex mu;
        std::condition_variable cv;
        bool started;
        std::thread controlThread;
        ev::reactor::EventLoop* controlLoop;
    };
}

#endif //TANK_TROUBLE_CONTROLLER_H
