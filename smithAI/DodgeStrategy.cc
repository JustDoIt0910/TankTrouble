//
// Created by zr on 23-2-21.
//

#include "DodgeStrategy.h"
#include "Tank.h"
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

    bool DodgeStrategy::update(LocalController* ctl, Tank* tank, uint64_t globalStep)
    {
        bool isForwarding = tank->isForwarding();
        bool isBackwarding = tank->isBackwarding();
        if(cmds.empty())
            return false;
        tank->stop();
        DodgeCommand cmd = cmds.front();
        cmds.pop_front();
        Object::PosInfo cur = tank->getCurrentPosition();
        if(cmd.targetStep == 0)
        {
            cmd.targetStep = globalStep + cmd.step;
            cmds.push_front(cmd);
        }
        switch (cmd.op)
        {
            case DODGE_CMD_ROTATE_CW:
                if(globalStep < cmd.targetStep)
                {
                    tank->rotateCW(true);
                    cmds.push_front(cmd);
                }
                break;
            case DODGE_CMD_ROTATE_CCW:
                if(globalStep < cmd.targetStep)
                {
                    tank->rotateCCW(true);
                    cmds.push_front(cmd);
                }
                break;
            case DODGE_CMD_MOVE_FORWARD:
                if(globalStep < cmd.targetStep)
                {
                    tank->forward(true);
                    cmds.push_front(cmd);
                }
                break;
            case DODGE_CMD_MOVE_BACKWARD:
                if(globalStep < cmd.targetStep)
                {
                    tank->backward(true);
                    cmds.push_front(cmd);
                }
                break;
            case DODGE_CMD_FORWARD_CW:
                if(globalStep < cmd.targetStep)
                {
                    tank->rotateCW(true);
                    tank->forward(true);
                    cmds.push_front(cmd);
                }
                break;
            case DODGE_CMD_FORWARD_CCW:
                if(globalStep < cmd.targetStep)
                {
                    tank->rotateCCW(true);
                    tank->forward(true);
                    cmds.push_front(cmd);
                }
                break;
            case DODGE_CMD_BACKWARD_CW:
                if(globalStep < cmd.targetStep)
                {
                    tank->rotateCW(true);
                    tank->backward(true);
                    cmds.push_front(cmd);
                }
                break;
            case DODGE_CMD_BACKWARD_CCW:
                if(globalStep < cmd.targetStep)
                {
                    tank->rotateCCW(true);
                    tank->backward(true);
                    cmds.push_front(cmd);
                }
                break;
        }
        if(isForwarding && cur.pos == prevPos.pos)
        {
            cmds.clear();
            cmd.op = DODGE_CMD_MOVE_BACKWARD;
            cmd.targetStep = globalStep + 10;
            cmds.push_front(cmd);
            tank->backward(true);
        }
        else if(isBackwarding && cur.pos == prevPos.pos)
        {
            cmds.clear();
            cmd.op = DODGE_CMD_MOVE_FORWARD;
            cmd.targetStep = globalStep + 10;
            cmds.push_front(cmd);
            tank->forward(true);
        }
        prevPos = cur;
        return true;
    }
}