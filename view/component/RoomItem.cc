//
// Created by zr on 23-3-10.
//

#include "RoomItem.h"

namespace TankTrouble
{

    RoomItem::RoomItem(uint8_t roomId, const std::string& name, uint8_t playerNum, uint8_t capacity):
        roomId(roomId)
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
        joinBtn.signal_clicked().connect(sigc::mem_fun(*this, &RoomItem::onJoinClicked));
        pack_start(joinBtn, Gtk::PACK_EXPAND_PADDING, 50);
    }

    void RoomItem::onJoinClicked() {click_join_s.emit(roomId);}

    sigc::signal<void, uint8_t> RoomItem::signal_join_clicked() {return click_join_s;}
}