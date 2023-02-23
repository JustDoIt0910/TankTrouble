//
// Created by zr on 23-2-21.
//

#ifndef TANK_TROUBLE_DODGE_STRATEGY_H
#define TANK_TROUBLE_DODGE_STRATEGY_H
#include <deque>
#include <stdint.h>
#include <string>
#include <iostream>
#include "../util/Vec.h"

namespace TankTrouble
{
    class Tank;

    class DodgeStrategy
    {
    public:
        enum DodgeOperation {DODGE_CMD_MOVE_FORWARD, DODGE_CMD_MOVE_BACKWARD,
                DODGE_CMD_ROTATE_CW, DODGE_CMD_ROTATE_CCW};
        struct DodgeCommand
        {
            DodgeOperation op;
            uint64_t step;
            uint64_t targetStep;

            friend std::ostream& operator<<(std::ostream& os, const DodgeCommand& cmd)
            {
                os << "Cmd{cmd = " + cmd.toString() + ", step = " << cmd.step << "}" << std::endl;
                return os;
            }

            [[nodiscard]] std::string toString() const
            {
                static const char* names[] = {"DODGE_CMD_MOVE_FORWARD", "DODGE_CMD_MOVE_BACKWARD",
                                              "DODGE_CMD_ROTATE_CW", "DODGE_CMD_ROTATE_CCW"};
                return names[this->op];
            }
        };
        explicit DodgeStrategy(uint64_t needStep): needStep(needStep){}
        DodgeStrategy(): DodgeStrategy(0){}
        void addCmd(const DodgeCommand& cmd);
        bool isEmpty();
        void popBack();
        void update(Tank* tank, uint64_t globalStep);
        bool operator<(const DodgeStrategy& strategy) const;
        DodgeStrategy& operator=(const DodgeStrategy& strategy) = default;
        bool isValid();
        friend std::ostream& operator<<(std::ostream& os, const DodgeStrategy& strategy);

    private:
        std::deque<DodgeCommand> cmds;
        uint64_t needStep;
    };
}

#endif //TANK_TROUBLE_DODGE_STRATEGY_H
