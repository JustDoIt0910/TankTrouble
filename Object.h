//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_OBJECT_H
#define TANK_TROUBLE_OBJECT_H
#include "util/Vec.h"
#include <cairomm/context.h>
#include <vector>
#include <mutex>

namespace TankTrouble
{
#define MOVING_STATIONARY 1
#define MOVING_FORWARD 2
#define MOVING_BACKWARD 4
#define ROTATING_CW 8
#define ROTATING_CCW 16

    enum ObjMovingStep {TANK_MOVING_STEP = 3, SHELL_MOVING_STEP = 6};
    enum ObjType {OBJ_TANK, OBJ_SHELL};
    typedef uint8_t MovingStatus;
    typedef std::vector<double> Color;
    const static Color BLACK = {0, 0, 0};
    const static Color RED = {1.0, 0, 0};
    const static Color GREEN = {0, 1.0, 0};

    class Object {
    public:
        struct PosInfo
        {
            PosInfo(const util::Vec& p, double a): pos(p), angle(a){}
            PosInfo& operator=(const PosInfo& info) = default;
            PosInfo(): PosInfo(util::Vec(0.0, 0.0), 0){}
            util::Vec pos;
            double angle;
        };

        Object(const util::Vec& pos, double angle, const Color& c);
        virtual void draw(const Cairo::RefPtr<Cairo::Context>& cr) = 0;
        virtual PosInfo getNextPosition(int movingStep, int rotationStep) = 0;
        virtual void moveToNextPosition() = 0;
        void resetNextPosition(const PosInfo& next);
        virtual ObjType type() = 0;
        virtual ~Object() = default;

        static int getId();
        static int globalId;
        static std::mutex mu;

    protected:
        PosInfo posInfo;
        PosInfo nextPos;
        MovingStatus movingStatus;
        Color color;
    };
}

#endif //TANK_TROUBLE_OBJECT_H
