//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_TANK_H
#define TANK_TROUBLE_TANK_H
#include "util/cord.h"
#include "object.h"

namespace tank
{
    enum MovingDir
    {
        MOVING_FORWARD,
        MOVING_BACKWARD,
        MOVING_STATIONARY
    };

    class Tank : public Object {
    public:
        Tank(const util::Cord&, double, const Color&);
        void draw(const Cairo::RefPtr<Cairo::Context>&) override;
        void move() override;
        ~Tank() override = default;

        const static int TANK_WIDTH = 20;
        const static int TANK_HEIGHT = 25;
    private:
        util::Cord topLeft, topRight,
        bottomLeft, bottomRight;
        int remainBullets;
        MovingDir dir;
    };
}

#endif //TANK_TROUBLE_TANK_H
