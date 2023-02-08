//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_OBJECT_H
#define TANK_TROUBLE_OBJECT_H
#include "util/cord.h"
#include <cairomm/context.h>
#include <vector>

typedef std::vector<double> Color;
const static Color BLACK = {0, 0, 0};
const static Color RED = {1.0, 0, 0};
const static Color GREEN = {0, 1.0, 0};

namespace tank
{
    enum ObjMovingStep
    {
        TANK_MOVING_STEP = 5,
        BULLET_MOVING_STEP = 10
    };

    class Object {
    public:
        Object(const util::Cord& p, double _angle, ObjMovingStep s, const Color& c):
            pos(p), angle(_angle), step(s), color(c) {}
        virtual void draw(const Cairo::RefPtr<Cairo::Context>&) = 0;
        virtual void move() = 0;
        virtual ~Object() = 0;
    private:
        util::Cord pos;
        double angle;
        ObjMovingStep step;
        Color color;
    };
}

#endif //TANK_TROUBLE_OBJECT_H
