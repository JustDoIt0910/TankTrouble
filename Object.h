//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_OBJECT_H
#define TANK_TROUBLE_OBJECT_H
#include "util/Cord.h"
#include <cairomm/context.h>
#include <vector>

namespace TankTrouble
{
#define MOVING_STATIONARY 1
#define MOVING_FORWARD 2
#define MOVING_BACKWARD 4
#define ROTATING_CW 8
#define ROTATING_CCW 16

    enum ObjMovingStep {TANK_MOVING_STEP = 4, SHELL_MOVING_STEP = 6};
    enum ObjType {OBJ_TANK, OBJ_SHELL};
    typedef uint8_t MovingStatus;
    typedef std::vector<double> Color;
    const static Color BLACK = {0, 0, 0};
    const static Color RED = {1.0, 0, 0};
    const static Color GREEN = {0, 1.0, 0};

    class Object {
    public:
        Object(const util::Cord& p, double _angle, const Color& c):
            pos(p), angle(_angle), movingStatus(MOVING_STATIONARY), color(c) {}
        virtual void draw(const Cairo::RefPtr<Cairo::Context>&) = 0;
        virtual void move() = 0;
        virtual ObjType type() = 0;
        virtual ~Object() = default;

    protected:
        util::Cord pos;
        double angle;
        MovingStatus movingStatus;
        Color color;
    };
}

#endif //TANK_TROUBLE_OBJECT_H
