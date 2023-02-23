//
// Created by zr on 23-2-21.
//

#include "DodgeStrategy.h"
#include "../Tank.h"
#include <iostream>

namespace TankTrouble
{
    std::ostream& operator<<(std::ostream& os, const DodgeStrategy& strategy)
    {
        for(const auto& cmd: strategy.cmds)
            os << cmd;
        return os;
    }

    void DodgeStrategy::addCmd(const DodgeCommand& cmd) {cmds.push_back(cmd);}

    void DodgeStrategy::popBack() {cmds.pop_back();}

    bool DodgeStrategy::isEmpty() {return cmds.empty();}

    bool DodgeStrategy::isValid() {return !cmds.empty();}

    bool DodgeStrategy::operator<(const DodgeStrategy& strategy) const{return needStep < strategy.needStep;}

    void DodgeStrategy::update(Tank* tank, uint64_t globalStep)
    {
        tank->stop();
        if(cmds.empty())
            return;
        DodgeCommand cmd = cmds.front();
        cmds.pop_front();
        if(cmd.op == DODGE_CMD_ROTATE_CW)
        {
            if(cmd.targetStep == 0)
            {
                cmd.targetStep = globalStep + cmd.step;
                cmds.push_front(cmd);
            }
            if(globalStep < cmd.targetStep)
            {
                tank->rotateCW(true);
                cmds.push_front(cmd);
            }
        }
        else if(cmd.op == DODGE_CMD_ROTATE_CCW)
        {
            if(cmd.targetStep == 0)
            {
                cmd.targetStep = globalStep + cmd.step;
                cmds.push_front(cmd);
            }
            if(globalStep < cmd.targetStep)
            {
                tank->rotateCCW(true);
                cmds.push_front(cmd);
            }
        }
        else if(cmd.op == DODGE_CMD_MOVE_FORWARD)
        {
            if(cmd.targetStep == 0)
            {
                cmd.targetStep = globalStep + cmd.step;
                cmds.push_front(cmd);
            }
            if(globalStep < cmd.targetStep)
            {
                tank->forward(true);
                cmds.push_front(cmd);
            }
        }
        else
        {
            if(cmd.targetStep == 0)
            {
                cmd.targetStep = globalStep + cmd.step;
                cmds.push_front(cmd);
            }
            if(globalStep < cmd.targetStep)
            {
                tank->backward(true);
                cmds.push_front(cmd);
            }
        }
    }
}