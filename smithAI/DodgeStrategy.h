//
// Created by zr on 23-2-21.
//

#ifndef TANK_TROUBLE_DODGE_STRATEGY_H
#define TANK_TROUBLE_DODGE_STRATEGY_H
#include <deque>
#include <string>
#include <iostream>
#include "Object.h"
#include "util/Vec.h"
#include "Strategy.h"

namespace TankTrouble
{
    class Controller;
    class DodgeStrategy : public Strategy
    {
    public:
        enum DodgeOperation {DODGE_CMD_MOVE_FORWARD, DODGE_CMD_MOVE_BACKWARD,
                DODGE_CMD_ROTATE_CW, DODGE_CMD_ROTATE_CCW,
                DODGE_CMD_FORWARD_CW, DODGE_CMD_FORWARD_CCW,
                DODGE_CMD_BACKWARD_CW, DODGE_CMD_BACKWARD_CCW};
        struct DodgeCommand
        {
            DodgeOperation op;
            uint64_t step;
            uint64_t targetStep;

            friend std::ostream& operator<<(std::ostream& os, const DodgeCommand& cmd)
            {
                os << "Cmd{cmd = " + cmd.toString() + ", step = " << cmd.step << "} ";
                return os;
            }

            [[nodiscard]] std::string toString() const
            {
                static const char* names[] = {"DODGE_CMD_MOVE_FORWARD", "DODGE_CMD_MOVE_BACKWARD",
                                              "DODGE_CMD_ROTATE_CW", "DODGE_CMD_ROTATE_CCW",
                                              "DODGE_CMD_FORWARD_CW", "DODGE_CMD_FORWARD_CCW",
                                              "DODGE_CMD_BACKWARD_CW", "DODGE_CMD_BACKWARD_CCW"};
                return names[this->op];
            }
        };
        explicit DodgeStrategy(uint64_t needStep):
            Strategy(Strategy::Dodge), needStep(needStep){}
        DodgeStrategy(): DodgeStrategy(0){}
        void addCmd(const DodgeCommand& cmd);
        bool isEmpty();
        void popBack();
        bool update(Controller* ctl, Tank* tank, uint64_t globalStep) override;
        bool operator<(const DodgeStrategy& strategy) const;
        DodgeStrategy& operator=(const DodgeStrategy& strategy) = default;
        bool isValid();
        friend std::ostream& operator<<(std::ostream& os, const DodgeStrategy& strategy);

    private:
        std::deque<DodgeCommand> cmds;
        Object::PosInfo prevPos;
        uint64_t needStep;
    };
}

#endif //TANK_TROUBLE_DODGE_STRATEGY_H
