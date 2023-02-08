//
// Created by zr on 23-2-8.
//

#include "cord.h"

tank::util::Cord::Cord(double x, double y): _x(x), _y(y){}

tank::util::Cord& tank::util::Cord::operator=(const Cord& c)
        {_x = c._x; _y = c._y; }

double tank::util::Cord::x() const {return _x;}

double tank::util::Cord::y() const {return _y;}