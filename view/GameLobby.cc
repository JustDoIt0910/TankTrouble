//
// Created by zr on 23-3-9.
//

#include "GameLobby.h"
#include "controller/OnlineController.h"

namespace TankTrouble
{
    GameLobby::GameLobby(OnlineController *ctl):
        ctl(ctl)
    {
        getUserInfo();
        roomList.set_orientation(Gtk::ORIENTATION_VERTICAL);
        newRoomEntry.set_max_length(50);
        put(newRoomEntry, 430, 20);
        newRoomBtn.set_label("新建房间");
        newRoomBtn.signal_clicked().connect(sigc::mem_fun(*this, &GameLobby::onCreateRoom));
        put(newRoomBtn, 600, 20);
        show_all_children();
    }

    void GameLobby::getUserInfo()
    {
        OnlineUser userInfo = ctl->getUserInfo();
        char text[100] = {0};
        sprintf(text, "昵称: %s       历史分数 %u",
                userInfo.nickname_.c_str(), userInfo.score_);
        userInfoLabel.set_text(text);
        put(userInfoLabel, 30, 30);
    }

    void GameLobby::getRoomInfo()
    {
        roomInfos = std::move(ctl->getRoomInfos());
        for(auto& item: roomItems)
            roomList.remove(item);
        roomItems.clear();
        for(const auto& roomInfo: roomInfos)
            roomItems.emplace_back(roomInfo.roomName_, roomInfo.playerNum_, roomInfo.roomCap_);
        for(auto& item: roomItems)
            roomList.pack_start(item, Gtk::PACK_EXPAND_WIDGET, 20);
        remove(roomList);
        put(roomList, 0, 50);
        show_all_children();
    }

    void GameLobby::onCreateRoom()
    {
        std::string name = newRoomEntry.get_text();
        ctl->createNewRoom(name);
    }
}