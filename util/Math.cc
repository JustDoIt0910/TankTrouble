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

    Cord polar2Cart(double theta, double p, Cord O)
    {
        double x = O.x() + cos(deg2Rad(theta)) * p;
        double y = O.y() - sin(deg2Rad(theta)) * p;
        return Cord(x, y);
    }

    std::vector<Cord> getCornerCord(const Cord& pos, double angle, int w, int h)
    {
        Cord tl, tr, bl, br;
        static double diagM2 = sqrt(pow(w, 2) + pow(h, 2)) / 2.0;
        double a = rad2Deg(atan2((double) w, (double) h));
        double a1 = a + angle;
        double a2 = angle - a;
        tl = polar2Cart(a1, diagM2, pos);
        tr = polar2Cart(a2, diagM2, pos);
        bl = Cord(2 * pos.x() - tr.x(), 2 * pos.y() - tr.y());
        br = Cord(2 * pos.x() - tl.x(), 2 * pos.y() - tl.y());
        return {tl, tr, bl, br};
    }

    std::vector<std::pair<Cord, Cord>> getRandomBlocks(int num)
    {
        std::vector<int> v(MAX_BLOCKS_NUM);
        std::iota(v.begin(), v.end(), 0);
        unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(v.begin(), v.end(), std::default_random_engine(seed));
        std::vector<std::pair<Cord, Cord>> res;
        for(int i = 0; i < num; i++)
        {
            int index = v[i];
            if(i < (HORIZON_GRID_NUMBER - 1) * VERTICAL_GRID_NUMBER)
            {
                //竖直的block
                int x = index % (HORIZON_GRID_NUMBER - 2) + 1;
                int y = index / (HORIZON_GRID_NUMBER - 2);
                Cord start(x, y); Cord end(x, y + 1);
                res.emplace_back(start, end);
            }
            else
            {
                //水平的block
                index -= (HORIZON_GRID_NUMBER - 1) * VERTICAL_GRID_NUMBER;
                int x = index % HORIZON_GRID_NUMBER;
                int y = index / HORIZON_GRID_NUMBER + 1;
                Cord start(x, y); Cord end(x + 1, y);
                res.emplace_back(start, end);
            }
        }
        return res;
    }
}
