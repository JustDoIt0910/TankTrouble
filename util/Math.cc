//
// Created by zr on 23-2-8.
//
#include "Math.h"
#include "../defs.h"
#include <cmath>
#include <numeric>
#include <random>
#include <chrono>
#include <algorithm>

namespace TankTrouble::util
{
    double rad2Deg(double rad){return rad * 180 / M_PI;}

    double deg2Rad(double deg){return deg * M_PI / 180;}

    Vec polar2Cart(double theta, double p, Vec O)
    {
        double x = O.x() + cos(deg2Rad(theta)) * p;
        double y = O.y() - sin(deg2Rad(theta)) * p;
        return Vec(x, y);
    }

    std::vector<Vec> getCornerVec(const Vec& pos, double angle, int w, int h)
    {
        Vec tl, tr, bl, br;
        static double diagM2 = sqrt(pow(w, 2) + pow(h, 2)) / 2.0;
        double a = rad2Deg(atan2((double) w, (double) h));
        double a1 = a + angle;
        double a2 = angle - a;
        tl = polar2Cart(a1, diagM2, pos);
        tr = polar2Cart(a2, diagM2, pos);
        bl = Vec(2 * pos.x() - tr.x(), 2 * pos.y() - tr.y());
        br = Vec(2 * pos.x() - tl.x(), 2 * pos.y() - tl.y());
        return {tl, tr, bl, br};
    }

    std::vector<std::pair<Vec, Vec>> getRandomBlocks(int num)
    {
        std::vector<int> v(MAX_BLOCKS_NUM);
        std::iota(v.begin(), v.end(), 0);
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(v.begin(), v.end(), std::default_random_engine(seed));
        std::vector<std::pair<Vec, Vec>> res;
        for(int i = 0; i < num; i++)
        {
            int index = v[i];
            if(index < (HORIZON_GRID_NUMBER - 1) * VERTICAL_GRID_NUMBER)
            {
                //竖直的block
                int x = index % (HORIZON_GRID_NUMBER - 1) + 1;
                int y = index / (HORIZON_GRID_NUMBER - 1);
                Vec start(x, y); Vec end(x, y + 1);
                res.emplace_back(start, end);
            }
            else
            {
                //水平的block
                index -= (HORIZON_GRID_NUMBER - 1) * VERTICAL_GRID_NUMBER;
                int x = index % HORIZON_GRID_NUMBER;
                int y = index / HORIZON_GRID_NUMBER + 1;
                Vec start(x, y); Vec end(x + 1, y);
                res.emplace_back(start, end);
            }
        }
        return res;
    }

    bool checkRectCircleCollision(const Vec& vec1, const Vec& vec2,
                                  const Vec& rectCenter, const Vec& circleCenter,
                                  int width, int height, int r)
    {
        Vec v = Vec(circleCenter.x() - rectCenter.x(), circleCenter.y() - rectCenter.y());
        double d1 = std::abs(v * vec2);
        double d2 = std::abs(v * vec1);
        double d3 = sqrt(pow(static_cast<double>(width) / 2, 2) + pow(static_cast<double>(height) / 2, 2));
        double d = v.norm();
        printf("(%f, %f, %f)\n", d1, d2, d);
        if(d1 < static_cast<double>(width) / 2 + r && d2 < static_cast<double>(height) / 2 + r && d < d3 + r)
            return true;
        return false;
    }
}
