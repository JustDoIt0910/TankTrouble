//
// Created by zr on 23-2-16.
//

#include "Shell.h"
#include "util/Math.h"

namespace TankTrouble
{
    Shell::Shell(const util::Vec& p, double angle):
        Object(p, angle, BLACK)
    {movingStatus = MOVING_FORWARD;}

    void Shell::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        cr->save();
        cr->set_source_rgb(color[0], color[1], color[2]);
        cr->arc(posInfo.pos.x(), posInfo.pos.y(), RADIUS, 0.0, 2 * M_PI);
        cr->fill();
        cr->restore();
    }

    Object::PosInfo Shell::getNextPosition(int movingStep, int rotationStep)
    {
        if(movingStep == 0)
            movingStep = SHELL_MOVING_STEP;
        Object::PosInfo next = posInfo;
        next.pos = util::polar2Cart(posInfo.angle, movingStep, posInfo.pos);
        nextPos = next;
        return next;
    }

    void Shell::moveToNextPosition() {posInfo = nextPos;}

    ObjType Shell::type() {return OBJ_SHELL;}
}