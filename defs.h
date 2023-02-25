//
// Created by zr on 23-2-17.
//

#ifndef TANK_TROUBLE_DEFS_H
#define TANK_TROUBLE_DEFS_H

#define HORIZON_GRID_NUMBER             11
#define VERTICAL_GRID_NUMBER            7
#define GRID_SIZE                       60
#define WINDOW_WIDTH                    (HORIZON_GRID_NUMBER * GRID_SIZE)
#define WINDOW_HEIGHT                   (VERTICAL_GRID_NUMBER * GRID_SIZE)
#define MAP_GRID_TO_REAL(gx, gy)        ((gx) * GRID_SIZE), ((gy) * GRID_SIZE)
#define MAP_REAL_TO_GRID(rx, ry)        ((rx) / (GRID_SIZE)), ((ry) / (GRID_SIZE))
#define MAX_BLOCKS_NUM                  ((HORIZON_GRID_NUMBER - 1) * VERTICAL_GRID_NUMBER \
                                        + (VERTICAL_GRID_NUMBER - 1) * HORIZON_GRID_NUMBER)

#define A_STAR_GRID_PER_GRID            2.0
#define A_STAR_GRID_SIZE                (GRID_SIZE / A_STAR_GRID_PER_GRID)
#define HORIZON_A_STAR_GRID_NUMBER      (A_STAR_GRID_PER_GRID * HORIZON_GRID_NUMBER)
#define VERTICAL_A_STAR_GRID_NUMBER     (A_STAR_GRID_PER_GRID * VERTICAL_GRID_NUMBER)
#define MAP_REAL_X_TO_A_STAR_X(rx)      (static_cast<int>((rx) / A_STAR_GRID_SIZE))
#define MAP_REAL_Y_TO_A_STAR_Y(ry)      (static_cast<int>((ry) / A_STAR_GRID_SIZE))
#define MAP_A_STAR_X_TO_REAL_X(ax)      static_cast<double>((ax) * A_STAR_GRID_SIZE + A_STAR_GRID_SIZE / 2)
#define MAP_A_STAR_Y_TO_REAL_Y(ay)      static_cast<double>((ay) * A_STAR_GRID_SIZE + A_STAR_GRID_SIZE / 2)

#endif //TANK_TROUBLE_DEFS_H
