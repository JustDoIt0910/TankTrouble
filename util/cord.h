//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_CORD_H
#define TANK_TROUBLE_CORD_H

namespace tank::util
{
    class Cord {
    public:
        Cord(double x = 0, double y = 0);
        Cord(const Cord&);
        Cord& operator=(const Cord&);
        double x();
        double y();
    private:
        double _x, _y;
    };
}

#endif //TANK_TROUBLE_CORD_H
