//
// Created by zr on 23-3-9.
//

#ifndef TANK_TROUBLE_ONLINE_CONTROLLER_H
#define TANK_TROUBLE_ONLINE_CONTROLLER_H
#include "Controller.h"
#include "Data.h"
#include "protocol/Codec.h"

using namespace ev::net;

namespace ev::net {class TcpClient;}

namespace TankTrouble
{
    class Window;

    class OnlineController: public Controller
    {
    public:
        OnlineController(Window* win, Inet4Address serverAddr);
        ~OnlineController() override;
        void start() override;
        void login(const std::string& name);
        void createNewRoom(const std::string& name, uint8_t cap);
        void joinRoom(uint8_t roomId);
        OnlineUser getUserInfo();
        std::vector<RoomInfo> getRoomInfos(uint8_t* currentRoomId, uint8_t* currentJoinStatus);

    private:
        void run();
        void sendLoginMessage(const TcpConnectionPtr& conn);
        void sendControlMessage(uint8_t action, uint8_t enable);
        void controlEventHandler(ev::Event *event);

        void onLoginSuccess(const TcpConnectionPtr& conn, Message message, ev::Timestamp);
        void onRoomsUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp);
        void onJoinRoomRespond(const TcpConnectionPtr& conn, Message message, ev::Timestamp);
        void onGameOn(const TcpConnectionPtr& conn, Message message, ev::Timestamp);
        void onBlocksUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp);
        void onObjectsUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp);
        void onScoresUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp);


        Window* interface;
        Inet4Address serverAddress;
        std::unique_ptr<TcpClient> client;
        Codec codec;
        std::string nickname;

        std::mutex userInfoMu;
        OnlineUser userInfo;

        std::mutex roomsInfoMu;
        std::vector<RoomInfo> roomsInfo;

        uint8_t joinedRoomId;
        uint8_t joinStatus;
    };
}

#endif //TANK_TROUBLE_ONLINE_CONTROLLER_H
