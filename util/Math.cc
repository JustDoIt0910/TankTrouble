//
// Created by zr on 23-2-8.
//
#include "Math.h"
#include "../defs.h"
#include <cmath>
#include <numeric>
#include <random>
#include <chrono>
#include <utility>
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
        //printf("(%f, %f, %f)\n", d1, d2, d);
        if(d1 < static_cast<double>(width) / 2 + r && d2 < static_cast<double>(height) / 2 + r && d < d3 + r)
            return true;
        return false;
    }

    void twoPointToGeneral(Vec p1, Vec p2, double* A, double* B, double* C)
    {
        double x1 = p1.x(); double y1 = p1.y();
        double x2 = p2.x(); double y2 = p2.y();
        *A = y2 - y1;
        *B = x1 - x2;
        *C = x2 * y1 - x1 * y2;
    }

    bool intersectionOfLines(double A1, double B1, double C1, double A2, double B2, double C2, Vec* p)
    {
        double m = A1 * B2 - A2 * B1;
        if(m == 0)
            return false;
        double x = (B1 * C2 - C1 * B2) / m;
        double y = (C1 * A2 - C2 * A1) / m;
        *p = Vec(x, y);
        return true;
    }

    bool intersectionOfSegments(Vec p1, Vec p2, Vec p3, Vec p4, Vec* i)
    {
        Vec n1(p1.y() - p2.y(), p2.x() - p1.x());
        double d1 = p1 * n1;
        double d3 = p3 * n1;
        double d4 = p4 * n1;
        if((d1 - d3) * (d1 - d4) > 0)
            return false;
        Vec n2(p4.y() - p3.y(), p3.x() - p4.x());
        d4 = p4 * n2;
        d1 = p1 * n2;
        double d2 = p2 * n2;
        if((d4 - d1) * (d4 - d2) > 0)
            return false;

        double A1, B1, C1, A2, B2, C2;
        twoPointToGeneral(p1, p2, &A1, &B1, &C1);
        twoPointToGeneral(p3, p4, &A2, &B2, &C2);
        return intersectionOfLines(A1, B1, C1, A2, B2, C2, i);
    }

    double angleFlipX(double angle) {return static_cast<int>(360 - angle) % 360;}

    double angleFlipY(double angle)
    {
        if(angle >= 0 && angle <= 180)
            return 180 - angle;
        else
            return 540 - angle;
    }

    std::pair<Vec, Vec> getUnitVectors(double angleDeg)
    {
        double angleRad = deg2Rad(angleDeg);
        Vec v1(cos(angleRad), -sin(angleRad));
        Vec v2(sin(angleRad), cos(angleRad));
        return std::make_pair(v1, v2);
    }

    bool checkRectRectCollision(double angle1, Vec center1, double W1, double H1,
                                double angle2, Vec center2, double W2, double H2)
    {
        std::pair<Vec, Vec> units1 = getUnitVectors(angle1);
        std::pair<Vec, Vec> units2 = getUnitVectors(angle2);
        Vec axis1 = units1.first; Vec axis2 = units1.second;
        Vec axis3 = units2.first; Vec axis4 = units2.second;
        Vec v = center1 - center2;
        double projV = std::abs(v * axis1);
        double projRadius = std::abs(axis3 * axis1) * H2 / 2 + std::abs(axis4 * axis1) * W2 / 2;
        if(projRadius + H1 / 2 <= projV)
            return false;
        projV = std::abs(v * axis2);
        projRadius = std::abs(axis3 * axis2) * H2 / 2 + std::abs(axis4 * axis2) * W2 / 2;
        if(projRadius + W1 / 2 <= projV)
            return false;

        projV = std::abs(v * axis3);
        projRadius = std::abs(axis1 * axis3) * H1 / 2 + std::abs(axis2 * axis3) * W1 / 2;
        if(projRadius + H2 / 2 <= projV)
            return false;
        projV = std::abs(v * axis4);
        projRadius = std::abs(axis1 * axis4) * H1 / 2 + std::abs(axis2 * axis4) * W1 / 2;
        if(projRadius + W2 / 2 <= projV)
            return false;
        return true;
    }

    double distanceOfTwoPoints(const Vec& p1, const Vec& p2)
    {
        Vec v = p2 - p1;
        return v.norm();
    }
}
