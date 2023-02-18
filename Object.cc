//
// Created by zr on 23-2-18.
//
#include "Object.h"

namespace TankTrouble
{
    Object::Object(const util::Vec& pos, double angle, const Color& c):
            posInfo(pos, angle), movingStatus(MOVING_STATIONARY), color(c) {}

    void Object::resetNextPosition(const PosInfo& next) {nextPos = next;}

    Object::PosInfo Object::getCurrentPosition() {return posInfo;}
}