//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_OBJECT_H
#define TANK_TROUBLE_OBJECT_H
#include "util/Vec.h"
#include <cairomm/context.h>
#include <vector>
#include <cfloat>

#define MOVING_STATIONARY 1
#define MOVING_FORWARD 2
#define MOVING_BACKWARD 4
#define ROTATING_CW 8
#define ROTATING_CCW 16

namespace TankTrouble
{
    enum ObjMovingStep {TANK_MOVING_STEP = 1, SHELL_MOVING_STEP = 1};
    enum ObjType {OBJ_TANK, OBJ_SHELL};
    typedef uint8_t MovingStatus;
    typedef std::vector<double> Color;
    const static Color BLACK = {0, 0, 0};
    const static Color RED = {1.0, 0.3, 0.3};
    const static Color GREY = {0.3, 0.3, 0.3};

    class Object {
    public:
        struct PosInfo
        {
            PosInfo(const util::Vec& p, double a): pos(p), angle(a){}

            PosInfo& operator=(const PosInfo& info) = default;

            PosInfo(): PosInfo(util::Vec(0.0, 0.0), 0){}

            static PosInfo invalid() {return PosInfo{util::Vec(DBL_MAX, DBL_MAX), DBL_MAX};}

            bool isValid() const
            {return (pos.x() != DBL_MAX && pos.y() != DBL_MAX && angle != DBL_MAX);}

            util::Vec pos;
            double angle;
        };

        Object(const util::Vec& pos, double angle, const Color& c, int id);
        virtual void draw(const Cairo::RefPtr<Cairo::Context>& cr) = 0;
        virtual PosInfo getNextPosition(int movingStep, int rotationStep) = 0;
        virtual void moveToNextPosition() = 0;
        void resetNextPosition(const PosInfo& next);
        PosInfo getCurrentPosition();
        virtual ObjType type() = 0;
        [[nodiscard]] int id() const;
        virtual ~Object() = default;

    protected:
        PosInfo posInfo;
        PosInfo nextPos;
        MovingStatus movingStatus;
        Color color;
        int _id;
    };
}

#endif //TANK_TROUBLE_OBJECT_H
