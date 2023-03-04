//
// Created by zr on 23-3-3.
//

#include "EntryView.h"
#include "defs.h"

namespace TankTrouble
{
    EntryView::EntryView():
        bg("entry.jpg")
    {
        localBtn.set_label("单人游戏");
        localBtn.set_size_request(80, 60);
        localBtn.signal_clicked().connect(
                sigc::mem_fun(*this, &EntryView::choose_local));
        onlineBtn.set_label("多人对战");
        onlineBtn.set_size_request(80, 60);
        onlineBtn.signal_clicked().connect(
                sigc::mem_fun(*this, &EntryView::choose_online));
        bg.set_size_request(WINDOW_WIDTH, WINDOW_HEIGHT);

        put(bg, 0, 0);
        put(localBtn, 170, 250);
        put(onlineBtn, 370, 250);

        bg.show();
        localBtn.show();
        onlineBtn.show();
    }

    void EntryView::choose_local() {choose_local_s.emit();}

    sigc::signal<void> EntryView::signal_choose_local() {return choose_local_s;}

    void EntryView::choose_online() {choose_online_s.emit();}

    sigc::signal<void> EntryView::signal_choose_online() {return choose_online_s;}
}