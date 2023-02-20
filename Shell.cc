//
// Created by zr on 23-2-16.
//

#include "Shell.h"
#include "util/Math.h"
#include "util/Id.h"

namespace TankTrouble
{
    Shell::Shell(const util::Vec& p, double angle, int tankId):
        Object(p, angle, BLACK, util::Id::getShellId()),
        _tankId(tankId),
        _ttl(INITIAL_TTL)
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
        Object::PosInfo next = getNextPosition(posInfo, movingStep, rotationStep);
        nextPos = next;
        return next;
    }

    Object::PosInfo Shell::getNextPosition(const Object::PosInfo& cur, int movingStep, int rotationStep)
    {
        if(movingStep == 0)
            movingStep = SHELL_MOVING_STEP;
        Object::PosInfo next = cur;
        next.pos = util::polar2Cart(cur.angle, movingStep, cur.pos);
        return next;
    }

    void Shell::moveToNextPosition() {posInfo = nextPos;}

    ObjType Shell::type() {return OBJ_SHELL;}

    int Shell::countDown() {return _ttl--;}

    int Shell::tankId() const {return _tankId;}

    int Shell::ttl() const {return _ttl;}
}