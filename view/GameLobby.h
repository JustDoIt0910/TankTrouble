//
// Created by zr on 23-3-9.
//

#ifndef TANK_TROUBLE_GAME_LOBBY_H
#define TANK_TROUBLE_GAME_LOBBY_H
#include <gtkmm.h>
#include "component/RoomItem.h"

namespace TankTrouble
{
    class OnlineController;
    struct RoomInfo;

    class GameLobby : public Gtk::Fixed
    {
    public:
        explicit GameLobby(OnlineController* ctl);
        void getUserInfo();
        void getRoomInfo();

    private:
        void onCreateRoom();
        void onJoinRoom(uint8_t roomId);

        OnlineController* ctl;
        Gtk::Label userInfoLabel;
        Gtk::Entry newRoomEntry;
        Gtk::Button newRoomBtn;

        Gtk::RadioButton capOption1;
        void capOption1Clicked();
        Gtk::RadioButton capOption2;
        void capOption2Clicked();
        Gtk::RadioButton capOption3;
        void capOption3Clicked();
        uint8_t newRoomCap;

        Gtk::Box roomList;
        std::vector<RoomInfo> roomInfos;
        std::vector<std::unique_ptr<RoomItem>> roomItems;
    };
}

#endif //TANK_TROUBLE_GAME_LOBBY_H
