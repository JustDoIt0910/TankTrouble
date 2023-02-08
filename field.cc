//
// Created by zr on 23-2-8.
//

#include "field.h"

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