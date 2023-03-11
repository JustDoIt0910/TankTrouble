//
// Created by zr on 23-3-3.
//

#ifndef TANK_TROUBLE_CONTROLLER_H
#define TANK_TROUBLE_CONTROLLER_H

#include <memory>
#include <unordered_map>
#include <map>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "Tank.h"
#include "Block.h"
#include "controller/Data.h"
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
        std::vector<PlayerInfo> getPlaysInfo();

        virtual ~Controller();

    protected:
        ObjectListPtr snapshot;

        BlockList blocks;
        std::mutex blocksMu;

        std::mutex playersInfoMu;
        std::map<int, PlayerInfo> playersInfo;

        std::mutex mu;
        std::condition_variable cv;
        bool started;
        std::thread controlThread;
        ev::reactor::EventLoop* controlLoop;
    };
}

#endif //TANK_TROUBLE_CONTROLLER_H
