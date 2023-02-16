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
#include "util/Cord.h"
#include "reactor/EventLoop.h"
#include "Tank.h"

namespace TankTrouble
{

    class Controller {
    public:
        typedef std::unique_ptr<Object> ObjectPtr;
        typedef std::vector<ObjectPtr> ObjectList;
        typedef std::shared_ptr<ObjectList> ObjectListPtr;

        typedef std::vector<std::vector<util::Cord>> BarrierList;

        Controller();
        ~Controller();
        void start();
        ObjectListPtr getObjects();
        void dispatchEvent(ev::Event* event);
        BarrierList getBarriers();

    private:
        void run();
        void moveAll();
        void controlEventHandler(ev::Event* event);
        void fire(Tank* tank);

        ObjectList objects;
        ObjectListPtr snapshot;
        std::mutex mu;
        std::condition_variable cv;
        bool started;
        std::thread controlThread;
        ev::reactor::EventLoop* controlLoop;
    };

}

#endif //TANK_TROUBLE_CONTROLLER_H
