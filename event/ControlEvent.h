//
// Created by zr on 23-2-16.
//

#ifndef TANK_TROUBLE_CONTROL_EVENT_H
#define TANK_TROUBLE_CONTROL_EVENT_H
#include "reactor/Event.h"

namespace TankTrouble
{
    class ControlEvent : public ev::Event
    {
    public:
        //前进，停止，后退，顺时针调头，逆时针调头
        enum Operation
        {
            Forward, Backward, RotateCW, RotateCCW,
            StopForward, StopBackward, StopRotateCW, StopRotateCCW,
            Fire,
            //testing
            SmithRotateCW,
            SmithStopRotateCW
        };

        explicit ControlEvent(Operation op);
        ControlEvent();
        ~ControlEvent() override = default;
        [[nodiscard]] Operation operation() const;

    private:
        Operation op;
    };
}

#endif //TANK_TROUBLE_CONTROL_EVENT_H
