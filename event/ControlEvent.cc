//
// Created by zr on 23-2-16.
//

#include "ControlEvent.h"

namespace TankTrouble
{
    ControlEvent::ControlEvent(ControlEvent::Operation op):
        ev::Event("ControlEvent", 1),
        op(op) {}

    ControlEvent::ControlEvent(): ControlEvent(StopForward){}

    ControlEvent::Operation ControlEvent::operation() const {return op;}
}