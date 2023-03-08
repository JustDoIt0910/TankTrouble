//
// Created by zr on 23-3-9.
//

#include "OnlineController.h"

namespace TankTrouble
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    OnlineController::OnlineController():
        Controller(),
        client(controlLoop, Inet4Address("127.0.0.1", 9999))
    {
        client.setMessageCallback(std::bind(&Codec::handleMessage, &codec, _1, _2, _3));
    }

    OnlineController::~OnlineController()
    {
        controlLoop->quit();
        if(controlThread.joinable())
            controlThread.join();
    }

    void OnlineController::start()
    {
        controlThread = std::thread(&OnlineController::run, this);
        std::unique_lock<std::mutex> lk(mu);
        cv.wait(lk, [this]() -> bool { return this->started; });
    }
}