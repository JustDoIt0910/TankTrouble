//
// Created by zr on 23-3-6.
//

#include "Codec.h"

namespace TankTrouble
{
    Codec::Codec()
    {
        messages_[MSG_LOGIN] = MessageTemplate({
            new FieldTemplate<std::string>("nickname")
        });

        messages_[MSG_LOGIN_RESP] = MessageTemplate({
                new FieldTemplate<std::string>("nickname"),
                new FieldTemplate<uint32_t>("score")
        });

        messages_[MSG_NEW_ROOM] = MessageTemplate({
            new FieldTemplate<std::string>("room_name"),
            new FieldTemplate<uint8_t>("player_num")
        });

        messages_[MSG_ROOM_INFO] = MessageTemplate({
            new ArrayFieldTemplate<StructField<uint8_t, std::string, uint8_t, uint8_t>>("room_infos", {
                "room_id", "room_name", "room_cap", "room_players"
            })
        });

        messages_[MSG_JOIN_ROOM] = MessageTemplate({
            new FieldTemplate<uint8_t>("join_room_id")
        });

        messages_[MSG_JOIN_ROOM_RESP] = MessageTemplate({
            new FieldTemplate<uint8_t>("join_room_id"),
            new FieldTemplate<uint8_t>("operation_status")
        });

        messages_[MSG_GAME_ON] = MessageTemplate({
            new ArrayFieldTemplate<StructField<uint8_t, std::string>>("players_info", {
                 "player_id", "player_nickname"
            })
        });

        messages_[MSG_UPDATE_BLOCKS] = MessageTemplate({
            new ArrayFieldTemplate<StructField<uint8_t, uint64_t, uint64_t>>("blocks", {
                   "is_horizon", "center_x", "center_y"
            })
        });

        messages_[MSG_UPDATE_OBJECTS] = MessageTemplate({
            new ArrayFieldTemplate<StructField<uint8_t, uint64_t, uint64_t, uint64_t>>("tanks", {
                    "id", "center_x", "center_y", "angle"
            }),
            new ArrayFieldTemplate<StructField<uint64_t, uint64_t>>("shells", {
                    "x", "y"
            })
        });

        messages_[MSG_UPDATE_SCORES] = MessageTemplate({
            new ArrayFieldTemplate<StructField<uint8_t, uint32_t>>("scores", {
                   "player_id", "score"
            })
        });

        messages_[MSG_CONTROL] = MessageTemplate({
            new FieldTemplate<uint8_t>("action"),
            new FieldTemplate<uint8_t>("enable")
        });
    }

    void Codec::registerHandler(int messageType, MessageHandler handler)
    {
        handlers_[messageType] = std::move(handler);
    }

    Buffer Codec::packMessage(int messageType, const Message& message)
    {
        Buffer buf;
        FixHeader header(messageType, message.size());
        header.toByteArray(&buf);
        message.toByteArray(&buf);
        return std::move(buf);
    }

    void Codec::sendMessage(const TcpConnectionPtr& conn, int messageType, const Message& message)
    {
        Buffer buf = packMessage(messageType, message);
        conn->send(buf);
    }

    Message Codec::getEmptyMessage(int messageType)
    {
        if(messages_.find(messageType) == messages_.end())
            return {};
        return std::move(messages_[messageType].getMessage());
    }

    void Codec::handleMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              ev::Timestamp receiveTime)
    {
        while(true)
        {
            if(buf->readableBytes() < HeaderLen)
                break;
            FixHeader header = getHeader(buf);
            if(buf->readableBytes() >= HeaderLen + header.messageLen)
            {
                buf->retrieve(HeaderLen);
                Message message = messages_[header.messageType].getMessage();
                message.fill(buf);
                if(handlers_.find(header.messageType) != handlers_.end())
                {
                    MessageHandler handler = handlers_[header.messageType];
                    if(handler)
                        handler(conn, std::move(message), receiveTime);
                }
            }
            else break;
        }
    }
}