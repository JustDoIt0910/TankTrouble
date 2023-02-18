//
// Created by zr on 23-2-18.
//

#include "Id.h"
#include <cassert>

namespace TankTrouble::util
{
    int Id::globalTankId = 1;
    std::mutex Id::tankIdMu;
    int Id::globalBlockId = 11;
    std::mutex Id::blockIdMu;

    int Id::getTankId()
    {
        std::lock_guard<std::mutex> lg(tankIdMu);
        assert(globalTankId <= 10);
        return globalTankId++;
    }

    int Id::getBlockId()
    {
        std::lock_guard<std::mutex> lg(blockIdMu);
        return globalBlockId++;
    }
}