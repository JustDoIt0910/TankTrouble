//
// Created by zr on 23-3-9.
//

#ifndef TANK_TROUBLE_ONLINE_CONTROLLER_H
#define TANK_TROUBLE_ONLINE_CONTROLLER_H
#include "Controller.h"
#include "protocol/Codec.h"
#include "net/TcpClient.h"

using namespace ev::net;

namespace TankTrouble
{
    class OnlineController: public Controller
    {
    public:
        OnlineController();
        ~OnlineController() override;
        void start() override;

    private:
        void run();

        TcpClient client;
        Codec codec;
    };
}

#endif //TANK_TROUBLE_ONLINE_CONTROLLER_H
