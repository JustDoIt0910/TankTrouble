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
        static PosInfo getNextPosition(const PosInfo& cur, int movingStatus, int movingStep, int rotationStep);
        void moveToNextPosition() override;
        void stop();
        void forward(bool enable);
        void backward(bool enable);
        void rotateCW(bool enable);
        void rotateCCW(bool enable);
        bool isForwarding();
        bool isBackwarding();
        bool isRotatingCW();
        bool isRotatingCCW();
        ObjType type() override;
        [[nodiscard]] int remainShells() const;
        [[nodiscard]] Shell* makeShell();
        void getRemainShell();
        ~Tank() override = default;

        const static int TANK_WIDTH = 20;
        const static int TANK_HEIGHT = 28;
        const static int ROTATING_STEP = 3;

    private:
        void recalculate();

        util::Vec topLeft, topRight,
        bottomLeft, bottomRight;
        int remainBullets;
    };
}

#endif //TANK_TROUBLE_TANK_H
