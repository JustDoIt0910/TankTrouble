//
// Created by zr on 23-2-8.
//

#include "Tank.h"
#include "Shell.h"
#include "util/Math.h"

namespace TankTrouble
{
    Tank::Tank(const util::Cord& p, double angle, const Color& c):
        Object(p, angle, c),
        remainBullets(5)
    {
        recalculate();
    }

    void Tank::recalculate()
    {
        auto corners = util::getCornerCord(pos, angle, Tank::TANK_WIDTH, Tank::TANK_HEIGHT);
        topLeft = corners[0]; topRight = corners[1]; bottomLeft = corners[2]; bottomRight = corners[3];
    }

    void Tank::stop() {movingStatus = MOVING_STATIONARY;}

    void Tank::forward(bool enable)
    {
        if(enable)
        {
            movingStatus &= ~MOVING_BACKWARD;
            movingStatus |= MOVING_FORWARD;
        }
        else movingStatus &= ~MOVING_FORWARD;
    }

    void Tank::backward(bool enable)
    {
        if(enable)
        {
            movingStatus &= ~MOVING_FORWARD;
            movingStatus |= MOVING_BACKWARD;
        }
        else movingStatus &= ~MOVING_BACKWARD;
    }

    void Tank::rotateCW(bool enable)
    {
        if(enable)
        {
            movingStatus &= ~ROTATING_CCW;
            movingStatus |= ROTATING_CW;
        }
        else movingStatus &= ~ROTATING_CW;
    }

    void Tank::rotateCCW(bool enable)
    {
        if(enable)
        {
            movingStatus &= ~ROTATING_CW;
            movingStatus |= ROTATING_CCW;
        }
        else movingStatus &= ~ROTATING_CCW;
    }

    void Tank::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        cr->save();
        cr->set_source_rgb(color[0], color[1], color[2]);
        cr->set_line_width(1.0);
        cr->move_to(topLeft.x(), topLeft.y());
        cr->line_to(topRight.x(), topRight.y());
        cr->line_to(bottomRight.x(), bottomRight.y());
        cr->line_to(bottomLeft.x(), bottomLeft.y());
        cr->close_path();
        cr->fill();

        cr->set_source_rgb(color[0] - 0.2, color[1], color[2]);
        cr->arc(pos.x(), pos.y(), 6, 0.0, 2 * M_PI);
        cr->fill();

        cr->set_line_width(7.0);
        cr->move_to(pos.x(), pos.y());
        util::Cord to = util::polar2Cart(angle, 16, pos);
        cr->line_to(to.x(), to.y());
        cr->stroke();
        cr->restore();
    }

    void Tank::move()
    {
        if(movingStatus & ROTATING_CW)
        {
            angle = static_cast<int>(360 + angle - ROTATING_STEP) % 360;
            recalculate();
        }
        if(movingStatus & ROTATING_CCW)
        {
            angle = static_cast<int>(angle + ROTATING_STEP) % 360;
            recalculate();
        }
        if(movingStatus & MOVING_FORWARD)
        {
            pos = util::polar2Cart(angle, TANK_MOVING_STEP, pos);
            recalculate();
        }
        if(movingStatus & MOVING_BACKWARD)
        {
            pos = util::polar2Cart(angle + 180, TANK_MOVING_STEP, pos);
            recalculate();
        }
    }

    ObjType Tank::type() {return OBJ_TANK;}

    int Tank::remainShells() const {return remainBullets;}

    Shell* Tank::makeShell()
    {
        remainBullets--;
        util::Cord shellPos = util::polar2Cart(angle, 18, pos);
        return new Shell(shellPos, angle);
    }
}

