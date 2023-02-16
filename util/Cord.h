//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_CORD_H
#define TANK_TROUBLE_CORD_H

namespace TankTrouble::util
{
    class Cord {
    public:
        explicit Cord(double x = 0, double y = 0);
        Cord(const Cord&) = default;
        Cord& operator=(const Cord&);
        [[nodiscard]] double x() const;
        [[nodiscard]] double y() const;
    private:
        double _x, _y;
    };
}

#endif //TANK_TROUBLE_CORD_H
