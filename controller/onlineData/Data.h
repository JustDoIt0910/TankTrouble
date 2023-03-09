//
// Created by zr on 23-3-9.
//

#ifndef TANK_TROUBLE_DATA_H
#define TANK_TROUBLE_DATA_H
#include <string>

namespace TankTrouble
{
    struct OnlineUser
    {
        std::string nickname_;
        uint32_t score_;
    };

    enum RoomStatus {New, Waiting, Playing};
    struct RoomInfo
    {
        uint8_t roomId_;
        std::string roomName_;
        uint8_t roomCap_;
        uint8_t playerNum_;
        RoomStatus roomStatus_;

        RoomInfo(uint8_t id, const std::string& name,
                 uint8_t cap, uint8_t playerNum,
                 RoomStatus status):
                roomId_(id), roomName_(name), roomCap_(cap), playerNum_(playerNum),
                roomStatus_(status) {}
    };
}

#endif //TANK_TROUBLE_DATA_H
