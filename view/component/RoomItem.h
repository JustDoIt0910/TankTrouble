//
// Created by zr on 23-3-10.
//

#ifndef TANK_TROUBLE_ROOM_ITEM_H
#define TANK_TROUBLE_ROOM_ITEM_H
#include <gtkmm.h>
#include <string>

namespace TankTrouble
{
    class RoomItem : public Gtk::Box
    {
    public:
        RoomItem(uint8_t roomId, const std::string& name,
                 uint8_t playerNum, uint8_t capacity,
                 bool enableJoin);
        sigc::signal<void, uint8_t> signal_join_clicked();

    private:
        void onJoinClicked();

        Gtk::Label roomNameLabel;
        Gtk::Label playersLabel;
        Gtk::Label statusLabel;
        Gtk::Button joinBtn;
        sigc::signal<void, uint8_t> click_join_s;
        uint8_t roomId;
    };
}


#endif //TANK_TROUBLE_ROOM_ITEM_H
