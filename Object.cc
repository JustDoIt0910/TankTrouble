//
// Created by zr on 23-2-18.
//
#include "Object.h"

namespace TankTrouble
{
    std::mutex Object::mu;
    int Object::globalId = 1;

    Object::Object(const util::Vec& pos, double angle, const Color& c):
            posInfo(pos, angle), movingStatus(MOVING_STATIONARY), color(c) {}

    void Object::resetNextPosition(const PosInfo& next) {nextPos = next;}

    int Object::getId()
    {
        std::lock_guard<std::mutex> lg(mu);
        return globalId++;
    }
}