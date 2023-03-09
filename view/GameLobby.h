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

            roomNameLabel.set_size_request(30, 20);
            roomNameLabel.set_text(name);
            pack_start(roomNameLabel, Gtk::PACK_EXPAND_PADDING, 50);

            char buf[10] = {0};
            sprintf(buf, "%u/%u", playerNum, capacity);
            playersLabel.set_size_request(30, 20);
            playersLabel.set_text(buf);
            pack_start(playersLabel, Gtk::PACK_EXPAND_PADDING, 50);

            std::string status = playerNum < capacity ? "空闲" : "对局中";
            statusLabel.set_size_request(30, 20);
            statusLabel.set_text(status);
            pack_start(statusLabel, Gtk::PACK_EXPAND_PADDING, 50);

            joinBtn.set_size_request(30, 20);
            joinBtn.set_label("加入");
            pack_start(joinBtn, Gtk::PACK_EXPAND_PADDING, 50);
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

        Gtk::RadioButton capOption1;
        void capOption1Clicked();
        Gtk::RadioButton capOption2;
        void capOption2Clicked();
        Gtk::RadioButton capOption3;
        void capOption3Clicked();
        uint8_t newRoomCap;

        Gtk::Box roomList;
        std::vector<RoomInfo> roomInfos;
        std::vector<RoomItem> roomItems;
    };
}

#endif //TANK_TROUBLE_GAME_LOBBY_H
