//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_CORD_H
#define TANK_TROUBLE_CORD_H

namespace TankTrouble::util
{
    class Vec {
    public:
        explicit Vec(double x = 0, double y = 0);
        Vec(const Vec&) = default;
        Vec& operator=(const Vec&) = default;
        double operator*(const Vec& v) const;
        Vec operator-(const Vec& v) const;
        [[nodiscard]] double norm() const;
        void swap(Vec& c);
        [[nodiscard]] double x() const;
        [[nodiscard]] double y() const;
    private:
        double _x, _y;
    };
}

#endif //TANK_TROUBLE_CORD_H
