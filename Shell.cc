//
// Created by zr on 23-2-16.
//

#include "Shell.h"
#include "util/Math.h"

namespace TankTrouble
{
    Shell::Shell(const util::Cord& p, double angle):
        Object(p, angle, BLACK)
    {movingStatus = MOVING_FORWARD;}

    void Shell::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        cr->save();
        cr->set_source_rgb(color[0], color[1], color[2]);
        cr->arc(pos.x(), pos.y(), RADIUS, 0.0, 2 * M_PI);
        cr->fill();
        cr->restore();
    }

    void Shell::move() {pos = util::polar2Cart(angle, SHELL_MOVING_STEP, pos);}

    ObjType Shell::type() {return OBJ_SHELL;}
}