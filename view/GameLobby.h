//
// Created by zr on 23-3-9.
//

#ifndef TANK_TROUBLE_GAME_LOBBY_H
#define TANK_TROUBLE_GAME_LOBBY_H
#include <gtkmm.h>

namespace TankTrouble
{
    class OnlineController;
    struct RoomInfo;

    class RoomItem : public Gtk::Box
    {
    public:
        RoomItem(const std::string& name, uint8_t playerNum, uint8_t capacity)
        {
            set_orientation(Gtk::ORIENTATION_HORIZONTAL);
            roomNameLabel.set_text(name);
            pack_start(roomNameLabel, Gtk::PACK_EXPAND_WIDGET, 20);
            char buf[10] = {0};
            sprintf(buf, "%u/%u", playerNum, capacity);
            playersLabel.set_text(buf);
            pack_start(playersLabel, Gtk::PACK_EXPAND_WIDGET, 20);
            std::string status = playerNum < capacity ? "空闲" : "对局中";
            statusLabel.set_text(status);
            pack_start(statusLabel, Gtk::PACK_EXPAND_WIDGET, 20);
            joinBtn.set_label("加入");
            pack_start(joinBtn, Gtk::PACK_EXPAND_WIDGET, 20);
            show_all_children();
        }

    private:
        Gtk::Label roomNameLabel;
        Gtk::Label playersLabel;
        Gtk::Label statusLabel;
        Gtk::Button joinBtn;
    };

    class GameLobby : public Gtk::Fixed
    {
    public:
        explicit GameLobby(OnlineController* ctl);
        void getUserInfo();
        void getRoomInfo();

    private:
        void onCreateRoom();

        OnlineController* ctl;
        Gtk::Label userInfoLabel;
        Gtk::Entry newRoomEntry;
        Gtk::Button newRoomBtn;
        Gtk::Box roomList;
        std::vector<RoomInfo> roomInfos;
        std::vector<RoomItem> roomItems;
    };
}

#endif //TANK_TROUBLE_GAME_LOBBY_H
