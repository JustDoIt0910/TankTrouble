//
// Created by zr on 23-2-18.
//

#include "Id.h"
#include "defs.h"
#include <cassert>

namespace TankTrouble::util
{
    int Id::globalTankId = 1;
    int Id::globalBlockId = 11;
    int Id::globalShellId = MAX_BLOCKS_NUM + 11;

    int Id::getTankId()
    {
        assert(globalTankId <= 10);
        return globalTankId++;
    }

    int Id::getBlockId() {return globalBlockId++;}

    int Id::getShellId() {return globalShellId++;}

    void Id::reset()
    {
        Id::globalTankId = 1;
        Id::globalBlockId = 11;
        Id::globalShellId = MAX_BLOCKS_NUM + 11;
    }
}