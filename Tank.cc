//
// Created by zr on 23-2-8.
//

#include "Tank.h"
#include "view/GameArea.h"
#include "Shell.h"
#include "util/Math.h"
#include "util/Id.h"

namespace TankTrouble
{
    Tank::Tank(const util::Vec& p, double angle, const Color& c):
        Object(p, angle, c, util::Id::getTankId()),
        remainBullets(5)
    {
        recalculate();
    }

    void Tank::recalculate()
    {
        auto corners = util::getCornerVec(posInfo.pos, posInfo.angle,
                                          Tank::TANK_WIDTH, Tank::TANK_HEIGHT);
        topLeft = corners[0]; topRight = corners[1]; bottomLeft = corners[2]; bottomRight = corners[3];
    }

    void Tank::stop()
    {
        movingStatus = 0;
        movingStatus |= MOVING_STATIONARY;
    }

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

    bool Tank::isForwarding() {return movingStatus & MOVING_FORWARD;}

    bool Tank::isBackwarding() {return movingStatus & MOVING_BACKWARD;}

    bool Tank::isRotatingCW() {return movingStatus & ROTATING_CW;}

    bool Tank::isRotatingCCW() {return movingStatus & ROTATING_CCW;}

    void Tank::draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        cr->save();
        GameArea::drawRect(cr, color, topLeft, topRight, bottomLeft, bottomRight);
        cr->close_path();
        cr->fill();

        cr->set_source_rgb(color[0] - 0.3, color[1] - 0.3, color[2] - 0.3);
        cr->arc(posInfo.pos.x(), posInfo.pos.y(), 9, 0.0, 2 * M_PI);
        cr->fill();

        cr->set_line_width(9.0);
        cr->move_to(posInfo.pos.x(), posInfo.pos.y());
        util::Vec to = util::polar2Cart(posInfo.angle, 17, posInfo.pos);
        cr->line_to(to.x(), to.y());
        cr->stroke();
        cr->restore();
    }

    Object::PosInfo Tank::getNextPosition(int movingStep, int rotationStep)
    {
        Object::PosInfo next = getNextPosition(posInfo, movingStatus, movingStep, rotationStep);
        nextPos = next;
        return next;
    }

    Object::PosInfo Tank::getNextPosition(const Object::PosInfo& cur, int movingStatus, int movingStep, int rotationStep)
    {
        if(movingStep == 0)
            movingStep = TANK_MOVING_STEP;
        if(rotationStep == 0)
            rotationStep = Tank::ROTATING_STEP;
        Object::PosInfo next = cur;
        if(movingStatus & ROTATING_CW)
            next.angle = static_cast<int>(360 + cur.angle - rotationStep) % 360;
        if(movingStatus & ROTATING_CCW)
            next.angle = static_cast<int>(cur.angle + rotationStep) % 360;
        if(movingStatus & MOVING_FORWARD)
            next.pos = util::polar2Cart(next.angle, movingStep, cur.pos);
        if(movingStatus & MOVING_BACKWARD)
            next.pos = util::polar2Cart(next.angle + 180, movingStep, cur.pos);
        return next;
    }

    void Tank::moveToNextPosition()
    {
        posInfo = nextPos;
        recalculate();
    }

    ObjType Tank::type() {return OBJ_TANK;}

    int Tank::remainShells() const {return remainBullets;}

    Shell* Tank::makeShell()
    {
        remainBullets--;
        util::Vec shellPos = util::polar2Cart(posInfo.angle, 15, posInfo.pos);
        return new Shell(shellPos, posInfo.angle, _id);
    }

    void Tank::getRemainShell() {remainBullets++;}
}
