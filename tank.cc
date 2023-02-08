//
// Created by zr on 23-2-8.
//

#include "tank.h"

tank::Tank::Tank(const util::Cord& p, double angle, const Color& c):
    Object(p, angle, tank::TANK_MOVING_STEP, c), remainBullets(5), dir(MOVING_STATIONARY)
{

}

void draw(const Cairo::RefPtr<Cairo::Context>&)
{

}

void move()
{

}