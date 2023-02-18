//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_CONTROLLER_H
#define TANK_TROUBLE_CONTROLLER_H
#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "Object.h"
#include "util/Vec.h"
#include "reactor/EventLoop.h"
#include "Tank.h"
#include "Block.h"

namespace TankTrouble
{

    class Controller {
    public:
        typedef std::unique_ptr<Object> ObjectPtr;
        typedef std::vector<ObjectPtr> ObjectList;
        typedef std::shared_ptr<ObjectList> ObjectListPtr;

        typedef std::vector<Block> BlockList;

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
