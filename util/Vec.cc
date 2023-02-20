//
// Created by zr on 23-2-8.
//

#include "Vec.h"
#include <cmath>

namespace TankTrouble::util
{
    Vec::Vec(double x, double y): _x(x), _y(y){}

    double Vec::operator*(const Vec& v) const {return _x * v.x() + _y * v.y();}

    Vec Vec::operator-(const Vec& v) const {return Vec(_x - v.x(), _y - v.y());}

    double Vec::norm() const {return sqrt(pow(_x, 2) + pow(_y, 2));}

    void Vec::swap(Vec &c)
    {
        Vec t = c;
        c = *this;
        _x = t.x();
        _y = t.y();
    }

    double Vec::x() const {return _x;}

    double Vec::y() const {return _y;}
}