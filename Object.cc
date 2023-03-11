//
// Created by zr on 23-2-18.
//
#include "Object.h"

#include <utility>

namespace TankTrouble
{
    Object::Object(const util::Vec& pos, double angle, Color c, int id):
            posInfo(pos, angle),
            movingStatus(MOVING_STATIONARY),
            color(std::move(c)),
            _id(id){}

    void Object::resetNextPosition(const PosInfo& next) {nextPos = next;}

    Object::PosInfo Object::getCurrentPosition() {return posInfo;}

    int Object::id() const {return _id;}

    int Object::getMovingStatus() const {return movingStatus;}
}