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
        Shell(const util::Cord& p, double angle);
        Shell(const Shell& s) = default;
        ~Shell() override = default;
        ObjType type() override;
        void draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
        void move() override;

        const static int RADIUS = 3;
    };
}

#endif //TANK_TROUBLE_SHELL_H
