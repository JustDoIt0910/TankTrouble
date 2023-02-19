//
// Created by zr on 23-2-18.
//
#include "Object.h"

namespace TankTrouble
{
    Object::Object(const util::Vec& pos, double angle, const Color& c, int id):
            posInfo(pos, angle),
            movingStatus(MOVING_STATIONARY),
            color(c),
            _id(id){}

    void Object::resetNextPosition(const PosInfo& next) {nextPos = next;}

    Object::PosInfo Object::getCurrentPosition() {return posInfo;}

    int Object::id() const {return _id;}
}