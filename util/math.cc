//
// Created by zr on 23-2-8.
//
#include "./math.h"
#include "../tank.h"
#include <cmath>

double tank::util::rad2Deg(double rad){return rad * 180 / M_PI;}

double tank::util::deg2Rad(double deg){return deg * M_PI / 180;}

tank::util::Cord tank::util::polar2Cart(double theta, double p, tank::util::Cord O)
{
    double x = O.x() + cos(deg2Rad(theta)) * p;
    double y = O.y() - sin(deg2Rad(theta)) * p;
    return Cord(x, y);
}

std::vector<tank::util::Cord> tank::util::getCornerCord(const tank::util::Cord& pos, double angle, int w, int h)
{
    Cord tl, tr, bl, br;
    double cx = pos.x();
    double cy = pos.y();
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
