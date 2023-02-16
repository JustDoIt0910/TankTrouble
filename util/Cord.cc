//
// Created by zr on 23-2-8.
//

#include "Cord.h"

namespace TankTrouble::util
{
    Cord::Cord(double x, double y): _x(x), _y(y){}

    Cord& Cord::operator=(const Cord& c)
    {_x = c._x; _y = c._y; }

    double Cord::x() const {return _x;}

    double Cord::y() const {return _y;}
}
