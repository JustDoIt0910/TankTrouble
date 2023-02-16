//
// Created by zr on 23-2-8.
//

#include "View.h"
#include <glibmm/main.h>
#include <iostream>

namespace TankTrouble
{
    View::View(Controller* c): ctl(c)
    {
        Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &View::on_timeout),
                20);
    }

    bool View::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        Controller::ObjectListPtr snapshot = ctl->getObjects();
        for(auto &obj: *snapshot)
            obj->draw(cr);
        return true;
    }

    bool View::on_timeout()
    {
        auto win = get_window();
        if (win)
        {
            Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                             get_allocation().get_height());
            win->invalidate_rect(r, false);
        }
        return true;
    }
}