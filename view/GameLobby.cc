//
// Created by zr on 23-3-9.
//

#include "GameLobby.h"

#include <memory>
#include "controller/OnlineController.h"

namespace TankTrouble
{
    GameLobby::GameLobby(OnlineController *ctl):
        ctl(ctl),
        newRoomCap(2),
        joinedRoomId(0)
    {
        getUserInfo();
        roomList.set_orientation(Gtk::ORIENTATION_VERTICAL);
        newRoomEntry.set_max_length(50);
        put(newRoomEntry, 300, 20);
        newRoomBtn.set_label("新建房间");
        newRoomBtn.signal_clicked().connect(sigc::mem_fun(*this, &GameLobby::onCreateRoom));
        put(newRoomBtn, 630, 20);
        capOption1.set_label("2人");
        capOption2.set_label("3人");
        capOption3.set_label("4人");
        capOption2.join_group(capOption1);
        capOption3.join_group(capOption1);
        capOption1.signal_clicked().connect(sigc::mem_fun(*this, &GameLobby::capOption1Clicked));
        capOption2.signal_clicked().connect(sigc::mem_fun(*this, &GameLobby::capOption2Clicked));
        capOption3.signal_clicked().connect(sigc::mem_fun(*this, &GameLobby::capOption3Clicked));
        put(capOption1, 490, 25);
        put(capOption2, 530, 25);
        put(capOption3, 570, 25);
        show_all_children();
    }

    void GameLobby::getUserInfo()
    {
        OnlineUser userInfo = ctl->getUserInfo();
        char text[100] = {0};
        sprintf(text, "昵称: %s       历史分数 %u",
                userInfo.nickname_.c_str(), userInfo.score_);
        userInfoLabel.set_text(text);
        put(userInfoLabel, 30, 25);
    }

    void GameLobby::getRoomInfo()
    {
        uint8_t joinedRoomId_, joinStatus;
        roomInfos = std::move(ctl->getRoomInfos(&joinedRoomId_, &joinStatus));
        if(joinStatus == Codec::ERR_IS_IN_ROOM)
            std::cout << "ERR_IS_IN_ROOM" << std::endl;
        else if(joinStatus == Codec::ERR_ROOM_NOT_EXIST)
            std::cout << "ERR_ROOM_NOT_EXIST" << std::endl;
        else if(joinStatus == Codec::JOIN_ROOM_SUCCESS)
            joinedRoomId = joinedRoomId_;

        remove(roomList);
        for(auto& item: roomItems)
            roomList.remove(*item);
        roomItems.clear();
        for(const auto& roomInfo: roomInfos)
        {
            bool disableJoin = (joinedRoomId == roomInfo.roomId_);
            roomItems.push_back(std::make_unique<RoomItem>(
                    roomInfo.roomId_, roomInfo.roomName_,
                    roomInfo.playerNum_, roomInfo.roomCap_,
                    !disableJoin));
        }

        for(auto& item: roomItems)
        {
            item->signal_join_clicked().
                    connect(sigc::mem_fun(*this, &GameLobby::onJoinRoom));
            roomList.pack_start(*item, Gtk::PACK_EXPAND_WIDGET, 20);
        }

        put(roomList, 0, 50);
        show_all_children();
    }

    void GameLobby::onCreateRoom()
    {
        std::string name = newRoomEntry.get_text();
        if(name.empty())
            return;
        ctl->createNewRoom(name, newRoomCap);
    }

    void GameLobby::onJoinRoom(uint8_t roomId)
    {
        ctl->joinRoom(roomId);
    }

    void GameLobby::capOption1Clicked() {newRoomCap = 2;}
    void GameLobby::capOption2Clicked() {newRoomCap = 3;}
    void GameLobby::capOption3Clicked() {newRoomCap = 4;}
}