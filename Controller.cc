//
// Created by zr on 23-3-9.
//
#include "Controller.h"

namespace TankTrouble
{
    Controller::Controller():
        started(false),
        controlLoop(nullptr),
        snapshot(new ObjectList) {}

    Controller::ObjectListPtr Controller::getObjects()
    {
        std::lock_guard<std::mutex> lg(mu);
        return snapshot;
    }

    void Controller::dispatchEvent(ev::Event* event) {controlLoop->dispatchEvent(event);}

    Controller::BlockList* Controller::getBlocks() {return &blocks;}

    Controller::~Controller()
    {
        controlLoop->quit();
        if(controlThread.joinable())
            controlThread.join();
    }
}