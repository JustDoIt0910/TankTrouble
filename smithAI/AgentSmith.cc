//
// Created by zr on 23-2-20.
//

#include "AgentSmith.h"
#include "controller/LocalController.h"
#include "Shell.h"
#include "util/Math.h"
#include <iostream>

namespace TankTrouble
{
    const int AgentSmith::MAX_PREDICT_STEP = 150;
    const int AgentSmith::THREATENED_RANGE = 150;
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
        ballistics.clear();
        for(const auto& shell: shells)
        {
            Ballistic ballistic;
            int sid = shell.first;
            ballisticPredict(shell, ballistic, globalSteps);
            ballistics[sid] = ballistic;
        }
    }

    bool AgentSmith::safeToMove(uint64_t globalSteps, const Object::PosInfo& cur, int movingStatus)
    {
        Object::PosInfo next = Tank::getNextPosition(cur, movingStatus, 0, 0);
        return checkWillDie(globalSteps + 1, next) != DIE;
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

    bool AgentSmith::checkFeasible(uint64_t step, const Object::PosInfo& tryPos)
    {
        if(ctl->checkTankBlockCollision(tryPos, tryPos) != 0)
            return false;
        return checkWillDie(step, tryPos) == SAFE;
    }

    AgentSmith::CheckResult AgentSmith::checkWillDie(uint64_t step, const Object::PosInfo& pos)
    {
        auto axis = util::getUnitVectors(pos.angle);
        for(int id: potentialThreats)
        {
            Object::PosInfo p = getShellPosition(id, step);
            if(!p.isValid())
                return UNKNOWN;
            if(util::checkRectCircleCollision(axis.first, axis.second, pos.pos, p.pos,
                                              Tank::TANK_WIDTH, Tank::TANK_HEIGHT, Shell::RADIUS))
                return DIE;
        }
        return SAFE;
    }

    bool AgentSmith::segmentCmp(const BallisticSegment& s1, const BallisticSegment& s2)
    {
        if(s1.shellId == s2.shellId)
            return s1.seq < s2.seq;
        return s1.distanceToTarget < s2.distanceToTarget;
    }

    DodgeStrategy AgentSmith::tryRotation(uint64_t globalSteps)
    {
        Object::PosInfo tryPos = smithPos;
        uint64_t step = globalSteps + 1;
        BallisticSegment segment = threats[0];
        std::vector<DodgeStrategy> strategies;
        for(int r = 0; r < (180 - 1) / Tank::ROTATING_STEP; r++, step++)
        {
            tryPos.angle = static_cast<int>(360 + tryPos.angle - Tank::ROTATING_STEP) % 360;
            if(!checkFeasible(step, tryPos)) break;
            if(!util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                             tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
            {
                if(util::checkRectRectCollision(dangerSegment.angle, dangerSegment.center,
                                                2 * Shell::RADIUS, dangerSegment.length,
                                                tryPos.angle, tryPos.pos,
                                                Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                    continue;
                DodgeStrategy strategy(step - globalSteps);
                strategy.addCmd({DodgeStrategy::DODGE_CMD_ROTATE_CW, step - globalSteps, 0});
                strategies.push_back(strategy);
                break;
            }
        }
        tryPos = smithPos; step = globalSteps + 1;
        for(int r = 0; r < (180 - 1) / Tank::ROTATING_STEP; r++, step++)
        {
            tryPos.angle = static_cast<int>(tryPos.angle + Tank::ROTATING_STEP) % 360;
            if(!checkFeasible(step, tryPos)) break;
            if(!util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                             tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
            {
                if(util::checkRectRectCollision(dangerSegment.angle, dangerSegment.center,
                                                2 * Shell::RADIUS, dangerSegment.length,
                                                tryPos.angle, tryPos.pos,
                                                Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                    continue;
                DodgeStrategy strategy(step - globalSteps);
                strategy.addCmd({DodgeStrategy::DODGE_CMD_ROTATE_CCW, step - globalSteps, 0});
                strategies.push_back(strategy);
                break;
            }
        }
        if(strategies.empty())
            return {};
        sort(strategies.begin(), strategies.end());
        return strategies[0];
    }

    bool AgentSmith::tryMovingStraight(uint64_t globalSteps, int direction,
                                      const Object::PosInfo& cur, uint64_t* takingSteps)
    {
        uint64_t step = globalSteps + 1; Object::PosInfo tryPos = cur;
        bool success = false;
        BallisticSegment segment = threats[0];
        for(int s = 0; s < 50; s++, step++)
        {
            tryPos = Tank::getNextPosition(tryPos, direction, 0, 0);
            if(!checkFeasible(step, tryPos)) break;
            if(!util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                             tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
            {
                if(util::checkRectRectCollision(dangerSegment.angle, dangerSegment.center,
                                                2 * Shell::RADIUS, dangerSegment.length,
                                                tryPos.angle, tryPos.pos,
                                                Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
                    continue;
                success = true;
                break;
            }
        }
        *takingSteps = step - globalSteps;
        return success;
    }

    bool AgentSmith::movingCurve(uint64_t globalSteps, int direction, int rotation,
                                    const Object::PosInfo& cur, int angleGran,
                                    uint64_t* rotatingSteps, uint64_t* straightSteps)
    {
        BallisticSegment segment = threats[0];
        Object::PosInfo tryPos = cur;
        uint64_t step = globalSteps;
        uint64_t maxTryingStep = 0;
        uint64_t bestRotatingStep = 0;
        uint64_t bestStraightStep = 0;
        int cnt = 0;

        for(int i = 0; i < 90 / Tank::ROTATING_STEP; i++)
        {
           step++; cnt++;
           tryPos = Tank::getNextPosition(tryPos, direction | rotation, 0, 0);
           if(!checkFeasible(step, tryPos))
           {
                if(step - globalSteps > maxTryingStep)
                {
                    maxTryingStep = step - globalSteps;
                    bestRotatingStep = step - globalSteps;
                }
                break;
           }
           if(cnt % angleGran)
               continue;
           if(!util::checkRectRectCollision(segment.angle, segment.center, 2 * Shell::RADIUS, segment.length,
                                            tryPos.angle, tryPos.pos, Tank::TANK_WIDTH, Tank::TANK_HEIGHT))
           {
               *rotatingSteps = step - globalSteps;
               return true;
           }
           uint64_t s;
           if(tryMovingStraight(step, direction, tryPos, &s))
           {
               *rotatingSteps = step - globalSteps;
               *straightSteps = s;
               return true;
           }
           else if((step - globalSteps) + s > maxTryingStep)
           {
               maxTryingStep = (step - globalSteps) + s;
               bestRotatingStep = step - globalSteps;
               bestStraightStep = s;
           }
        }
        *rotatingSteps = bestRotatingStep;
        *straightSteps = bestStraightStep;
        return false;
    }

    DodgeStrategy AgentSmith::tryRotatingWithMoving(uint64_t globalSteps)
    {
        BallisticSegment segment = threats[0];
        util::Vec v1 = util::getUnitVector(segment.angle);
        util::Vec v2 = smithPos.pos - segment.start.second;
        util::Vec vt = util::getUnitVector(smithPos.angle);
        DodgeStrategy strategy;
        int direction = util::angleBetweenVectors(vt, v1) <= 90.0 ? MOVING_FORWARD
                :MOVING_BACKWARD;
        uint64_t bestRotatingStep = 0;
        uint64_t bestStraightStep = 0;
        DodgeStrategy::DodgeOperation bestRotationOp;
        DodgeStrategy::DodgeOperation bestMoveOp = (direction == MOVING_FORWARD) ?
                                               DodgeStrategy::DODGE_CMD_MOVE_FORWARD :
                                               DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
        uint64_t rs = 0;
        uint64_t ms = 0;
        DodgeStrategy::DodgeOperation rotateCwOp = (direction == MOVING_FORWARD) ?
                                                 DodgeStrategy::DODGE_CMD_FORWARD_CW :
                                                 DodgeStrategy::DODGE_CMD_BACKWARD_CW;
        DodgeStrategy::DodgeOperation rotateCcwOp = (direction == MOVING_FORWARD) ?
                                                   DodgeStrategy::DODGE_CMD_FORWARD_CCW :
                                                   DodgeStrategy::DODGE_CMD_BACKWARD_CCW;
        bestRotationOp = rotateCwOp;
        if(movingCurve(globalSteps, direction, ROTATING_CW, smithPos, 2, &rs, &ms))
        {
            strategy.addCmd({rotateCwOp, rs, 0});
            if(ms > 0)
                strategy.addCmd({bestMoveOp, ms, 0});
            return strategy;
        }
        bestRotatingStep = rs;
        bestStraightStep = ms;
        rs = 0; ms = 0;
        if(movingCurve(globalSteps, direction, ROTATING_CCW, smithPos, 2, &rs, &ms))
        {
            strategy.addCmd({rotateCcwOp, rs, 0});
            if(ms > 0)
                strategy.addCmd({bestMoveOp, ms, 0});
            return strategy;
        }
        if(rs + ms > bestRotatingStep + bestStraightStep)
        {
            bestRotationOp = rotateCcwOp;
            bestRotatingStep = rs;
            bestStraightStep = ms;
        }
        strategy.addCmd({bestRotationOp, bestRotatingStep, 0});
        if(bestStraightStep > 0)
            strategy.addCmd({bestMoveOp, bestStraightStep, 0});
        return strategy;
    }

    DodgeStrategy AgentSmith::dodgeToSide(uint64_t globalSteps, const Object::PosInfo& cur,
                                          int witchSide, int angleGran)
    {
        DodgeStrategy strategy;
        Object::PosInfo tryPos = cur;
        BallisticSegment segment = threats[0];
        util::Vec v1 = util::getUnitVector(segment.angle);
        util::Vec vt = util::getUnitVector(tryPos.angle);
        bool pointingToSameSide = (witchSide == RIGHT_SIDE && v1.cross(vt) > 0) ||
                                  (witchSide == LEFT_SIDE && v1.cross(vt) < 0);
        bool stop = false;
        int direction;
        if(v1.cross(vt) != 0)
            direction = pointingToSameSide ? MOVING_FORWARD : MOVING_BACKWARD;
        else direction = util::angleBetweenVectors(v1, vt) <= 90.0 ? MOVING_FORWARD : MOVING_BACKWARD;
        DodgeStrategy::DodgeOperation moveOp = (direction == MOVING_FORWARD) ?
                                               DodgeStrategy::DODGE_CMD_MOVE_FORWARD: DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
        uint64_t s;
        if(tryMovingStraight(globalSteps, direction, tryPos, &s))
        {
            strategy.addCmd({moveOp, s, 0});
            stop = true;
        }
        if(stop) return strategy;
        uint64_t step = globalSteps;
        int cnt = 0;
        for(int i = 0; i < 90 / Tank::ROTATING_STEP; i++)
        {
            step++; cnt++;
            tryPos.angle = static_cast<int>(tryPos.angle + Tank::ROTATING_STEP) % 360;
            if(!checkFeasible(step, tryPos)) break;
            if(cnt % angleGran) continue;
            util::Vec vn = util::getUnitVector(tryPos.angle);
            if(v1.cross(vn) * v1.cross(vt) > 0)
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_FORWARD:
                        DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
                direction = pointingToSameSide ? MOVING_FORWARD : MOVING_BACKWARD;
            }
            else if(v1.cross(vn) * v1.cross(vt) < 0)
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_BACKWARD:
                         DodgeStrategy::DODGE_CMD_MOVE_FORWARD;
                direction = pointingToSameSide ? MOVING_BACKWARD : MOVING_FORWARD;
            }
            else
            {
                if(v1.cross(vn) == 0)
                    continue;
                else
                {
                    direction = ((witchSide == RIGHT_SIDE && v1.cross(vn) > 0) ||
                            (witchSide == LEFT_SIDE && v1.cross(vn) < 0)) ? MOVING_FORWARD : MOVING_BACKWARD;
                    moveOp = (direction == MOVING_FORWARD) ?
                            DodgeStrategy::DODGE_CMD_MOVE_FORWARD: DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
                }
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
        for(int i = 0; i < 90 / Tank::ROTATING_STEP; i++)
        {
            step++; cnt++;
            tryPos.angle = static_cast<int>(360 + tryPos.angle - Tank::ROTATING_STEP) % 360;
            if(!checkFeasible(step, tryPos)) break;
            if(cnt % angleGran) continue;
            util::Vec vn = util::getUnitVector(tryPos.angle);
            if(v1.cross(vn) * v1.cross(vt) > 0)
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_FORWARD:
                         DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
                direction = pointingToSameSide ? MOVING_FORWARD : MOVING_BACKWARD;
            }
            else if(v1.cross(vn) * v1.cross(vt) < 0)
            {
                moveOp = pointingToSameSide ? DodgeStrategy::DODGE_CMD_MOVE_BACKWARD:
                         DodgeStrategy::DODGE_CMD_MOVE_FORWARD;
                direction = pointingToSameSide ? MOVING_BACKWARD : MOVING_FORWARD;
            }
            else
            {
                if(v1.cross(vn) == 0)
                    continue;
                else
                {
                    direction = ((witchSide == RIGHT_SIDE && v1.cross(vn) > 0) ||
                                 (witchSide == LEFT_SIDE && v1.cross(vn) < 0)) ? MOVING_FORWARD : MOVING_BACKWARD;
                    moveOp = (direction == MOVING_FORWARD) ?
                             DodgeStrategy::DODGE_CMD_MOVE_FORWARD: DodgeStrategy::DODGE_CMD_MOVE_BACKWARD;
                }
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

    DodgeStrategy AgentSmith::tryRotatingAndMoving(uint64_t globalSteps)
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
        return strategy;
    }

    void AgentSmith::getDodgeStrategy(const Object::PosInfo& pos, uint64_t globalSteps)
    {
        bool stop = false;
        auto* finalStrategy = new DodgeStrategy;
        smithPos = pos;
        potentialThreats.clear();
        threats.clear();
        {
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
        sort(threats.begin(), threats.end(), segmentCmp);

        BallisticSegment closest = threats[0];
        if(closest.shellId != prevThreat.shellId || closest.seq != prevThreat.seq)
        {
            dangerSegment = prevThreat;
            prevThreat = closest;
        }

        DodgeStrategy strategy = tryRotation(globalSteps);
        if(strategy.isValid())
        {
            *finalStrategy = strategy;
            stop = true;
        }
        if(!stop)
        {
            strategy = tryRotatingAndMoving(globalSteps);
            if(strategy.isValid())
            {
                *finalStrategy = strategy;
                stop = true;
            }
        }
        if(!stop)
            *finalStrategy = tryRotatingWithMoving(globalSteps);

        std::cout << *finalStrategy << threats[0].shellId << " " << threats[0].seq << std::endl;
        ctl->updateStrategy(finalStrategy);
    }

    void AgentSmith::initAStar(AStar::BlockList* blocks) {aStar->init(blocks);}

    void AgentSmith::attack(const Object::PosInfo& smith, const Object::PosInfo& enemy,
                            uint64_t globalSteps)
    {
        if(globalSteps - prevFireTime < 60)
            return;
        if(util::distanceOfTwoPoints(smith.pos, enemy.pos) <= ATTACKING_RANGE)
        {
            Object::PosInfo aimingPos = tryAiming(smith, enemy);
            if(aimingPos.isValid())
            {
                Strategy* strategy = new AttackStrategy(aimingPos);
                ctl->updateStrategy(strategy);
                strategy = new ContactStrategy(AStar::AStarResult());
                ctl->updateStrategy(strategy);
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
            ctl->updateStrategy(strategy);
        }
    }

    Object::PosInfo AgentSmith::tryAiming(const Object::PosInfo& smith, const Object::PosInfo& enemy)
    {
        Object::PosInfo tryPos = smith;
        double distance = util::distanceOfTwoPoints(smith.pos, enemy.pos);
        double angle = util::vector2Angle(enemy.pos - smith.pos);
        LocalController::BlockList* blocks = ctl->getBlocks();
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
        for(int i = 0; i < 360 / Tank::ROTATING_STEP; i++)
        {
            tryPos.angle = static_cast<int>(tryPos.angle + Tank::ROTATING_STEP) % 360;
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