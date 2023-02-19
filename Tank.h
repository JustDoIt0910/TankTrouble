//
// Created by zr on 23-2-8.
//

#ifndef TANK_TROUBLE_TANK_H
#define TANK_TROUBLE_TANK_H
#include "util/Vec.h"
#include "Object.h"

namespace TankTrouble
{
    class Shell;

    class Tank : public Object {
    public:
        Tank(const util::Vec&, double, const Color& color);
        Tank(const Tank& t) = default;
        void draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
        PosInfo getNextPosition(int movingStep, int rotationStep) override;
        void moveToNextPosition() override;
        void forward(bool enable);
        void backward(bool enable);
        void rotateCW(bool enable);
        void rotateCCW(bool enable);
        ObjType type() override;
        [[nodiscard]] int remainShells() const;
        [[nodiscard]] Shell* makeShell();
        void getRemainShell();
        ~Tank() override = default;

        const static int TANK_WIDTH = 20;
        const static int TANK_HEIGHT = 25;
        const static int ROTATING_STEP = 4;

    private:
        void recalculate();

        util::Vec topLeft, topRight,
        bottomLeft, bottomRight;
        int remainBullets;
    };
}

#endif //TANK_TROUBLE_TANK_H
