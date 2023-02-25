//
// Created by zr on 23-2-20.
//

#include "AgentSmith.h"
#include "Controller.h"
#include "Shell.h"
#include "util/Math.h"
#include "event/StrategyUpdateEvent.h"
#include <iostream>

namespace TankTrouble
{
    const int AgentSmith::MAX_PREDICT_STEP = 150;
    const int AgentSmith::THREATENED_RANGE = 150;
    const int AgentSmith::MAX_DODGING_SHELLS = 1;
    const int AgentSmith::ATTACKING_RANGE = 120;

    AgentSmith::PredictingShellList AgentSmith::getIncomingShells(const Object::PosInfo& cur)
    {
        AgentSmith::PredictingShellList shells;
        for(const auto& entry: ctl->objects)
            if(entry.second->type() == OBJ_SHELL)
            {
                auto* s = dynamic_cast<Shell*>(entry.second.get());
                if(s->tankId() == 2 && s->ttl() == Shell::INITIAL_TTL)
                    continue;
                Object::PosInfo shellPos = entry.second->getCurrentPosition();
                if(util::distanceOfTwoPoints(shellPos.pos, cur.pos) <= THREATENED_RANGE)
                    shells[entry.first] = shellPos;
            }
        return shells;
    }

    void AgentSmith::ballisticPredict(PredictingShell shell, Ballistic& ballistic, uint64_t globalSteps)
    {
        int seq = 1;
        Object::PosInfo cur = shell.second;
        KeyPoint startKp(globalSteps, cur.pos);
        for(int i = 0; i < MAX_PREDICT_STEP - 1; i++)
        {
            Object::PosInfo next = Shell::getNextPosition(cur, 0, 0);
            int collisionId = ctl->checkShellBlockCollision(cur, next);
            if(collisionId)
            {
                next = ctl->getBouncedPosition(cur, next, collisionId);
                KeyPoint endKp(globalSteps + i + 1, next.pos);
                BallisticSegment segment(shell.first, seq++, startKp, endKp,
                                         util::distanceOfTwoPoints(startKp.second, endKp.second),
                                         cur.angle, 0.0);
                ballistic.push_back(segment);
                startKp = endKp;
            }
            cur = next;
        }
        KeyPoint endKp(globalSteps + MAX_PREDICT_STEP, cur.pos);
        BallisticSegment segment(shell.first, seq++, startKp, endKp,
                                 util::distanceOfTwoPoints(startKp.second, endKp.second),
                                 cur.angle, 0.0);
        ballistic.push_back(segment);
    }

    void AgentSmith::ballisticsPredict(const PredictingShellList& shells, uint64_t globalSteps)
    {
        int seq;
        std::lock_guard<std::mutex> lg(mu);
        ballistics.clear();
        for(const auto& shell: shells)
        {
            Ballistic ballistic;
            int sid = shell.first;
            ballisticPredict(shell, ballistic, globalSteps);
            ballistics[sid] = ballistic;
        }
    }

    Object::PosInfo AgentSmith::getShellPosition(int id, uint64_t step)
    {
        Ballistic ballistic = ballistics[id];
        for(const auto& segment : ballistic)
        {
            if(segment.end.first < step)
                continue;
            util::Vec start = segment.start.second;
            util::Vec end = segment.end.second;
            uint64_t span = segment.end.first - segment.start.first;
            util::Vec pos(static_cast<double>(step - segment.start.first) * (end.x() - start.x()) / static_cast<double>(span) + start.x(),
                          static_cast<double>(step - segment.start.first) * (end.y() - start.y()) / static_cast<double>(span) + start.y());
            return {pos, segment.angle};
        }
        return Object::PosInfo::invalid();
    }

    bool AgentSmith::danger(int shellId, const Object::PosInfo& cur, uint64_t step)
    {
        Object::PosInfo shellPos = getShellPosition(shellId, step);
        util::Vec v1 = cur.pos - shellPos.pos;
        util::Vec v2 = util::getUnitVector(shellPos.angle);
        if(util::angleBetweenVectors(v1, v2) < 90 && util::distanceOfTwoPoints(cur.pos, shellPos.pos) < 50)
            return true;
        return false;
    }

    AgentSmith::CheckResult AgentSmith::checkFeasible(uint64_t step, const Object::PosInfo& tryPos)
    {
        if(ctl->checkTankBlockCollision(tryPos, tryPos) != 0)
            return UNFEASIBLE;
        if(previousMostUrgent.isValid() &&
                util::checkRectRectCollision(previousMostUrgent.angle, previousMostUrgent.center,
                                             2 * Shell::RADIUS, previousMostUrgent.length,
                                             tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT)&&
                danger(previousMostUrgent.shellId, tryPos, step))
            return UNFEASIBLE;
        auto axis = util::getUnitVectors(tryPos.angle);
        for(int id: potentialThreats)
        {
            Object::PosInfo p = getShellPosition(id, step);
            if(!p.isValid())
                return UNKNOWN;
            if(util::checkRectCircleCollision(axis.first, axis.second, tryPos.pos, p.pos,
                                              Tank::TANK_WIDTH, Tank::TANK_HEIGHT, Shell::RADIUS))
            return UNFEASIBLE;
        }
        return FEASIBLE;
    }

    bool AgentSmith::segmentCmp(const BallisticSegment& s1, const BallisticSegment& s2)
    {
        if(s1.shellId == s2.shellId)
            return s1.seq < s2.seq;
        return s1.distanceToTarget < s2.distanceToTarget;
    }

    void AgentSmith::tryRotation(uint64_t globalSteps)
    {
        Object::PosInfo tryPos = smithPos;
        bool success; uint64_t step = globalSteps + 1;
        for(int r = 0; r < (180 - 1) / 4; r++, step++)
        {
            success = true;
            tryPos.angle = static_cast<int>(360 + tryPos.angle - 4) % 360;
            CheckResult check = checkFeasible(step, tryPos);
            if(check != FEASIBLE) break;
            for(int i = 0; i < std::min(MAX_DODGING_SHELLS, static_cast<int>(threats.size())); i++)
            {
                BallisticSegment segment = threats[i];
                if(!util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                                 tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                {
                    DodgeStrategy strategy(step - globalSteps);
                    strategy.addCmd({DodgeStrategy::DODGE_CMD_ROTATE_CW, step - globalSteps, 0});
                    strategies[i + 1].push_back(strategy);
                }
                else success = false;
            }
            if(success)
                break;
        }
        tryPos = smithPos; step = globalSteps + 1;
        for(int r = 0; r < (180 - 1) / 4; r++, step++)
        {
            success = true;
            tryPos.angle = static_cast<int>(tryPos.angle + 4) % 360;
            CheckResult check = checkFeasible(step, tryPos);
            if(check != FEASIBLE) break;
            for(int i = 0; i < std::min(MAX_DODGING_SHELLS, static_cast<int>(threats.size())); i++)
            {
                BallisticSegment segment = threats[i];
                if(!util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                                 tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                {
                    DodgeStrategy strategy(step - globalSteps);
                    strategy.addCmd({DodgeStrategy::DODGE_CMD_ROTATE_CCW, step - globalSteps, 0});
                    strategies[i + 1].push_back(strategy);
                }
                else success = false;
            }
            if(success)
                break;
        }
    }

    int AgentSmith::tryMovingStraight(uint64_t globalSteps, int direction,
                                      const Object::PosInfo& cur, uint64_t* takingSteps)
    {
        uint64_t step = globalSteps + 1; Object::PosInfo tryPos = cur;
        int maxDodging = 0;
        int threatNum = std::min(MAX_DODGING_SHELLS, static_cast<int>(threats.size()));
        bool stop = false;
        for(int s = 0; s < 30 && !stop; s++, step++)
        {
            tryPos = Tank::getNextPosition(tryPos, direction, 0, 0);
            CheckResult check = checkFeasible(step, tryPos);
            if(check != FEASIBLE) break;
            for(int i = 0; i < threatNum; i++)
            {
                BallisticSegment segment = threats[i];
                if(!util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                                 tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                {
                    maxDodging++;
                    if(maxDodging == threatNum)
                        stop = true;
                }
            }
        }
        *takingSteps = step - globalSteps;
        return maxDodging;
    }

    DodgeStrategy AgentSmith::dodgeToSide(uint64_t globalSteps, const Object::PosInfo& cur,
                                          int witchSide, int angleGran)
    {
        DodgeStrategy strategy;
        Object::PosInfo tryPos = cur;
        BallisticSegment segment = threats[0];
        util::Vec v1 = util::getUnitVector(segment.angle);
        util::Vec vt = util::getUnitVector(tryPos.angle);
        double angle = util::angleBetweenVectors(v1, vt);
        bool pointingToSameSide = (witchSide == RIGHT_SIDE && v1.cross(vt) > 0) ||
                                  (witchSide == LEFT_SIDE && v1.cross(vt) < 0);
        bool stop = false;
        int direction = pointingToSameSide ? MOVING_FORWARD : MOVING_BACKWARD;
        DodgeStrategy::DodgeOperation moveOp = pointingToSameSide ?
                                               DodgeStrategy::DODGE_CMD_MOVE_FORWARD: DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
        uint64_t s;
        if(tryMovingStraight(globalSteps, direction, tryPos, &s) == 1)
        {
            strategy.addCmd({moveOp, s, 0});
            stop = true;
        }
        if(stop) return strategy;
        uint64_t step = globalSteps;
        int cnt = 0;
        for(int i = 0; i < 90 / 4; i++)
        {
            step++; cnt++;
            tryPos.angle = static_cast<int>(tryPos.angle + 4) % 360;
            if(checkFeasible(step, tryPos) == UNFEASIBLE)
                break;
            if(cnt % angleGran) continue;
            util::Vec vn = util::getUnitVector(tryPos.angle);
            if(v1.cross(vn) * v1.cross(vt) > 0)
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_FORWARD:
                        DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
                direction = pointingToSameSide ? MOVING_FORWARD : MOVING_BACKWARD;
            }
            else
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_BACKWARD:
                         DodgeStrategy::DODGE_CMD_MOVE_FORWARD;
                direction = pointingToSameSide ? MOVING_BACKWARD : MOVING_FORWARD;
            }
            if(tryMovingStraight(step, direction, tryPos, &s))
            {
                strategy.addCmd({DodgeStrategy::DODGE_CMD_ROTATE_CCW, step - globalSteps, 0});
                strategy.addCmd({moveOp, s, 0});
                stop = true;
                break;
            }
        }
        if(stop) return strategy;
        step = globalSteps; cnt = 0;
        tryPos = cur;
        for(int i = 0; i < 90 / 4; i++)
        {
            step++; cnt++;
            tryPos.angle = static_cast<int>(360 + tryPos.angle - 4) % 360;
            if(checkFeasible(step, tryPos) == UNFEASIBLE)
                break;
            if(cnt % angleGran) continue;
            util::Vec vn = util::getUnitVector(tryPos.angle);
            if(v1.cross(vn) * v1.cross(vt) > 0)
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_FORWARD:
                         DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
                direction = pointingToSameSide ? MOVING_FORWARD : MOVING_BACKWARD;
            }
            else
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_BACKWARD:
                         DodgeStrategy::DODGE_CMD_MOVE_FORWARD;
                direction = pointingToSameSide ? MOVING_BACKWARD : MOVING_FORWARD;
            }
            if(tryMovingStraight(step, direction, tryPos, &s))
            {
                strategy.addCmd({DodgeStrategy::DODGE_CMD_ROTATE_CW, step - globalSteps, 0});
                strategy.addCmd({moveOp, s, 0});
                stop = true;
                break;
            }
        }
        return strategy;
    }

    void AgentSmith::tryMoving(uint64_t globalSteps)
    {
        if(std::min(MAX_DODGING_SHELLS, static_cast<int>(threats.size())) == 1)
        {
            BallisticSegment segment = threats[0];
            util::Vec v1 = util::getUnitVector(segment.angle);
            util::Vec v2 = smithPos.pos - segment.start.second;
            DodgeStrategy strategy;
            if(v1.cross(v2) >= 0)
            {
                strategy = dodgeToSide(globalSteps, smithPos, RIGHT_SIDE, 2);
                if(!strategy.isValid())
                    strategy = dodgeToSide(globalSteps, smithPos, LEFT_SIDE, 2);
            }
            else
            {
                strategy = dodgeToSide(globalSteps, smithPos, LEFT_SIDE, 2);
                if(!strategy.isValid())
                    strategy = dodgeToSide(globalSteps, smithPos, RIGHT_SIDE, 2);
            }
            if(!strategy.isValid())
            {
                util::Vec vt = util::getUnitVector(smithPos.angle);
                DodgeStrategy::DodgeOperation moveOp = util::angleBetweenVectors(v1, vt) < 90.0 ? DodgeStrategy::DODGE_CMD_MOVE_FORWARD :
                         DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
                strategy.addCmd({moveOp, 1000, 0});
            }
            strategies[1].push_back(strategy);
        }
    }

    void AgentSmith::getDodgeStrategy(const Object::PosInfo& pos, uint64_t globalSteps)
    {
        ev::Timestamp before = ev::Timestamp::now();
        bool hasStrategy = false;
        bool stop = false;
        auto* strategy = new DodgeStrategy;
        smithPos = pos;
        strategies.clear();
        potentialThreats.clear();
        threats.clear();
        {
            std::lock_guard<std::mutex> lg(mu);
            for(const auto& entry: ballistics)
            {
                for(auto segment : entry.second)
                {
                    if(util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                                    smithPos.angle, smithPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                    {
                        segment.distanceToTarget = util::distanceOfTwoPoints(segment.start.second, smithPos.pos);
                        threats.push_back(segment);
                    }
                    potentialThreats.insert(segment.shellId);
                }
            }
        }
        if(threats.empty())
            return;
        if(threats[0].shellId != currentMostUrgent.shellId || threats[0].seq != currentMostUrgent.seq)
        {
            if(threats[0].shellId != previousMostUrgent.shellId)
                previousMostUrgent = currentMostUrgent;
            currentMostUrgent = threats[0];
        }
        if(potentialThreats.find(previousMostUrgent.shellId) == potentialThreats.end())
            previousMostUrgent = BallisticSegment::invalid();

        int threatNum = std::min(static_cast<int>(threats.size()), MAX_DODGING_SHELLS);
        std::sort(threats.begin(), threats.end(), segmentCmp);
        tryRotation(globalSteps);
        if(!strategies[threatNum].empty())
        {
            sort(strategies[threatNum].begin(), strategies[threatNum].end());
            *strategy = strategies[threatNum][0];
            hasStrategy = true;
            stop = true;
        }
        if(!stop)
        {
            tryMoving(globalSteps);
            if(!strategies[threatNum].empty())
            {
                *strategy = strategies[threatNum][0];
                hasStrategy = true;
                stop = true;
            }
        }
        if(hasStrategy)
        {
            std::cout << *strategy << std::endl;
            auto* event = new StrategyUpdateEvent(strategy);
            ctl->dispatchEvent(event);
        }
        ev::Timestamp after = ev::Timestamp::now();
        //std::cout << (after - before) / 1000.0 << std::endl;
    }

    void AgentSmith::initAStar(AStar::BlockList* blocks) {aStar->init(blocks);}

    void AgentSmith::attack(const Object::PosInfo& smith, const Object::PosInfo& enemy,
                            uint64_t globalSteps)
    {
        if(globalSteps - prevFireTime < 100)
            return;
        if(util::distanceOfTwoPoints(smith.pos, enemy.pos) <= ATTACKING_RANGE)
        {
            Object::PosInfo aimingPos = tryAiming(smith, enemy);
            if(aimingPos.isValid())
            {
                Strategy* strategy = new AttackStrategy(aimingPos);
                auto* event = new StrategyUpdateEvent(strategy);
                ctl->dispatchEvent(event);
                strategy = new ContactStrategy(AStar::AStarResult());
                event = new StrategyUpdateEvent(strategy);
                ctl->dispatchEvent(event);
                prevFireTime = globalSteps;
                return;
            }
        }
        int smithX = MAP_REAL_X_TO_A_STAR_X(smith.pos.x());
        int smithY = MAP_REAL_Y_TO_A_STAR_Y(smith.pos.y());
        int enemyX = MAP_REAL_X_TO_A_STAR_X(enemy.pos.x());
        int enemyY = MAP_REAL_Y_TO_A_STAR_Y(enemy.pos.y());
        AStar::AStarResult route = aStar->findRoute(smithX, smithY, enemyX, enemyY);
        if(!route.empty())
        {
            auto* strategy = new ContactStrategy(route);
            auto* event = new StrategyUpdateEvent(strategy);
            ctl->dispatchEvent(event);
        }
    }

    Object::PosInfo AgentSmith::tryAiming(const Object::PosInfo& smith, const Object::PosInfo& enemy)
    {
        Object::PosInfo tryPos = smith;
        double distance = util::distanceOfTwoPoints(smith.pos, enemy.pos);
        double angle = util::vector2Angle(enemy.pos - smith.pos);
        Controller::BlockList* blocks = ctl->getBlocks();
        bool directShoot = true;
        for(const auto& block: *blocks)
        {
            if(util::distanceOfTwoPoints(block.second.start(), smith.pos) > distance &&
                    util::distanceOfTwoPoints(block.second.end(), smith.pos) > distance)
                continue;
            double blockAngle = block.second.isHorizon() ? 180.0 : 90.0;
            if(util::checkRectRectCollision(angle, (enemy.pos + smith.pos) / 2, 2 * Shell::RADIUS, distance,
                                            blockAngle, block.second.center(), block.second.width(), block.second.height()))
            {
                directShoot = false;
                break;
            }
        }
        if(directShoot)
        {
            tryPos.angle = angle;
            return tryPos;
        }
        for(int i = 0; i < 360 / 4; i++)
        {
            tryPos.angle = static_cast<int>(tryPos.angle + 4) % 360;
            if(std::abs(tryPos.angle - 0) < 10 || std::abs(tryPos.angle - 90) < 10 ||
            std::abs(tryPos.angle - 180) < 10 || std::abs(tryPos.angle - 270) < 10)
                continue;
            PredictingShell shell = std::make_pair(-1, tryPos);
            Ballistic ballistic;
            ballisticPredict(shell, ballistic, 0);
            for(const BallisticSegment& segment: ballistic)
            {
                if(util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                                enemy.angle, enemy.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                    return tryPos;
            }
        }
        tryPos = Object::PosInfo::invalid();
        return tryPos;
    }
}