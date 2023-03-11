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
        Shell(int id, const util::Vec& p, double angle, int tankId);
        Shell(const Shell& s) = default;
        ~Shell() override = default;
        ObjType type() override;
        [[nodiscard]] int ttl() const;
        void draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
        PosInfo getNextPosition(int movingStep, int rotationStep) override;
        static PosInfo getNextPosition(const PosInfo& cur, int movingStep, int rotationStep);
        void moveToNextPosition() override;
        int countDown();
        [[nodiscard]] int tankId() const;

        const static double RADIUS;
        const static int INITIAL_TTL = 10;

    private:
        int _ttl;
        int _tankId;
    };
}

#endif //TANK_TROUBLE_SHELL_H
