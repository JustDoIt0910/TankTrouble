//
// Created by zr on 23-3-6.
//

#ifndef TANK_TROUBLE_SERVER_CODEC_H
#define TANK_TROUBLE_SERVER_CODEC_H
#include <unordered_map>
#include <functional>
#include "Messages.h"
#include "net/TcpConnection.h"

#define MSG_LOGIN               0x10
#define MSG_LOGIN_RESP          0x11
#define MSG_NEW_ROOM            0x20
#define MSG_GET_ROOM            0x21
#define MSG_ROOM_INFO           0x22


namespace TankTrouble
{
    class Codec
    {
    public:
        typedef std::function<void(const TcpConnectionPtr& conn,
                Message,
                ev::Timestamp receiveTime)> MessageHandler;
        Codec();
        void handleMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           ev::Timestamp receiveTime);
        static void sendMessage(const TcpConnectionPtr& conn, int messageType, const Message& message);
        Message getEmptyMessage(int messageType);
        void registerHandler(int messageType, MessageHandler handler);

    private:
        std::unordered_map<int, MessageTemplate> messages_;
        std::unordered_map<int, MessageHandler> handlers_;
    };
}

#endif //TANK_TROUBLE_SERVER_CODEC_H
