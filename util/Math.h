//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_MATH_H
#define TANK_TROUBLE_MATH_H
#include <vector>
#include <utility>
#include "Vec.h"

namespace TankTrouble::util
{
    double rad2Deg(double rad);
    double deg2Rad(double deg);
    Vec polar2Cart(double theta, double p, Vec O = Vec(0, 0));
    std::vector<Vec> getCornerVec(const Vec& pos, double angle, int w, int h);
    std::vector<std::pair<Vec, Vec>> getRandomBlocks(int num);
    //提供矩形两个检测轴的单位向量，矩形中心，圆心，矩形宽高，半径判断矩形和圆形是否碰撞(重叠)
    bool checkRectCircleCollision(const Vec& vec1, const Vec& vec2,
                                  const Vec& rectCenter, const Vec& circleCenter,
                                  int width, int height, int r);
}

#endif //TANK_TROUBLE_MATH_H
