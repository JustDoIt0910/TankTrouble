//
// Created by zr on 23-2-16.
//

#ifndef TANK_TROUBLE_SHELL_H
#define TANK_TROUBLE_SHELL_H
#include "Object.h"

namespace TankTrouble
{
    class Shell : public Object
    {
    public:
        Shell(const util::Vec& p, double angle);
        Shell(const Shell& s) = default;
        ~Shell() override = default;
        ObjType type() override;
        void draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
        PosInfo getNextPosition(int movingStep, int rotationStep) override;
        void moveToNextPosition() override;

        const static int RADIUS = 3;

    private:
        int ttl;
    };
}

#endif //TANK_TROUBLE_SHELL_H
