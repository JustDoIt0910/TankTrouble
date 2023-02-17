//
// Created by zr on 23-2-17.
//

#ifndef TANK_TROUBLE_DEFS_H
#define TANK_TROUBLE_DEFS_H

#define HORIZON_GRID_NUMBER         11
#define VERTICAL_GRID_NUMBER        7
#define GRID_SIZE                   60
#define WINDOW_WIDTH                (HORIZON_GRID_NUMBER * GRID_SIZE + (HORIZON_GRID_NUMBER + 1))
#define WINDOW_HEIGHT               (VERTICAL_GRID_NUMBER * GRID_SIZE + (VERTICAL_GRID_NUMBER + 1))
#define MAP_GRID_TO_REAL(gx, gy)    (gx * GRID_SIZE + gx), (gy * GRID_SIZE + gy)
#define MAX_BLOCKS_NUM              ((HORIZON_GRID_NUMBER - 1) * VERTICAL_GRID_NUMBER \
                                    + (VERTICAL_GRID_NUMBER - 1) * HORIZON_GRID_NUMBER)
#endif //TANK_TROUBLE_DEFS_H
