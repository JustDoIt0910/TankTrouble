//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_MATH_H
#define TANK_TROUBLE_MATH_H
#include <vector>
#include <utility>
#include "Cord.h"

namespace TankTrouble::util
{

    double rad2Deg(double rad);
    double deg2Rad(double deg);
    Cord polar2Cart(double theta, double p, Cord O = Cord(0, 0));
    std::vector<Cord> getCornerCord(const Cord& pos, double angle, int w, int h);
    std::vector<std::pair<Cord, Cord>> getRandomBlocks(int num);
}

#endif //TANK_TROUBLE_MATH_H
