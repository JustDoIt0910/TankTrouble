//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_MATH_H
#define TANK_TROUBLE_MATH_H
#include <vector>
#include "cord.h"

namespace tank::util {

    double rad2Deg(double rad);
    double deg2Rad(double deg);
    util::Cord polar2Cart(double theta, double p, Cord O = Cord(0, 0));
    std::vector<Cord> getCornerCord(const tank::util::Cord& pos, double angle, int w, int h);

}

#endif //TANK_TROUBLE_MATH_H
