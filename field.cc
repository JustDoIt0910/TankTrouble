//
// Created by zr on 23-2-8.
//

#include "field.h"
#include <glibmm/main.h>

tank::Field::Field(tank::Controller* c): ctl(c)
{
    Glib::signal_timeout().connect(
            sigc::mem_fun(*this, &tank::Field::on_timeout),
            50);
}

bool tank::Field::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    tank::Controller::ObjectListPtr objects = ctl->getObjects();
    for(auto &obj: *objects)
        obj->draw(cr);
    return true;
}

bool tank::Field::on_timeout()
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