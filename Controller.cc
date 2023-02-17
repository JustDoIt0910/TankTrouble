//
// Created by zr on 23-2-8.
//

#include "Controller.h"
#include "event/ControlEvent.h"
#include "Tank.h"
#include "Shell.h"
#include <thread>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <unistd.h>

namespace TankTrouble
{
    Controller::Controller():
        started(false),
        controlLoop(nullptr),
        snapshot(new ObjectList)
    {
        objects.push_back(std::unique_ptr<Object>(new Tank(util::Cord(100, 100), 90.0, RED)));
    }

    Controller::~Controller()
    {
        controlLoop->quit();
        if(controlThread.joinable())
            controlThread.join();
    }

    void Controller::start()
    {
        controlThread = std::thread(&Controller::run, this);
        std::unique_lock<std::mutex> lk(mu);
        cv.wait(lk, [this] () -> bool {return this->started;});
    }

    void Controller::run()
    {
        ev::reactor::EventLoop loop;
        controlLoop = &loop;
        {
            std::unique_lock<std::mutex> lk(mu);
            started = true;
            cv.notify_all();
        }
        auto* event = new ControlEvent;
        loop.addEventListener(event, [this](ev::Event* event){this->controlEventHandler(event);});
        loop.runEvery(0.05, [this]{this->moveAll();});
        loop.loop();
        controlLoop = nullptr;
    }

    void Controller::dispatchEvent(ev::Event *event) {controlLoop->dispatchEvent(event);}

    Controller::ObjectListPtr Controller::getObjects()
    {
        std::lock_guard<std::mutex> lg(mu);
        return snapshot;
    }

    Controller::BarrierList Controller::getBarriers()
    {

    }

    void Controller::controlEventHandler(ev::Event *event)
    {
        auto* ce = dynamic_cast<ControlEvent*>(event);
        Tank* me = dynamic_cast<Tank*>(objects[0].get());
        switch (ce->operation())
        {
            case ControlEvent::Forward: me->forward(true); break;
            case ControlEvent::Backward: me->backward(true); break;
            case ControlEvent::RotateCW: me->rotateCW(true); break;
            case ControlEvent::RotateCCW: me->rotateCCW(true); break;
            case ControlEvent::StopForward: me->forward(false); break;
            case ControlEvent::StopBackward: me->backward(false); break;
            case ControlEvent::StopRotateCW: me->rotateCW(false); break;
            case ControlEvent::StopRotateCCW: me->rotateCCW(false); break;
            case ControlEvent::Fire: fire(me); break;
        }
    }

    void Controller::fire(Tank *tank)
    {
        if(tank->remainShells() == 0)
            return;
        objects.emplace_back(std::unique_ptr<Object>(tank->makeShell()));
    }

    void Controller::moveAll()
    {
        for(auto& obj: objects)
            obj->move();
        std::lock_guard<std::mutex> lg(mu);
        if(!snapshot.unique())
            snapshot.reset(new ObjectList);
        assert(snapshot.unique());
        snapshot->clear();
        for(auto& obj: objects)
        {
            if(obj->type() == OBJ_TANK)
                snapshot->emplace_back(std::unique_ptr<Object>(
                        new Tank(*dynamic_cast<Tank*>(obj.get()))));
            else
                snapshot->emplace_back(std::unique_ptr<Object>(
                        new Shell(*dynamic_cast<Shell*>(obj.get()))));
        }
    }
}
