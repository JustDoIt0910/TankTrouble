//
// Created by zr on 23-2-17.
//

#ifndef TANK_TROUBLE_DEFS_H
#define TANK_TROUBLE_DEFS_H
#include <vector>

#define HORIZON_GRID_NUMBER             11
#define VERTICAL_GRID_NUMBER            7
#define GRID_SIZE                       60.0
#define GAME_VIEW_WIDTH                 (HORIZON_GRID_NUMBER * GRID_SIZE)
#define GAME_VIEW_HEIGHT                (VERTICAL_GRID_NUMBER * GRID_SIZE)
#define MAP_REAL_TO_GRID(rx, ry)        ((rx) / (GRID_SIZE)), ((ry) / (GRID_SIZE))
#define MAP_GRID_TO_REAL(gx, gy)        static_cast<double>((gx) * GRID_SIZE + GRID_SIZE / 2), \
                                        static_cast<double>((gy) * GRID_SIZE + GRID_SIZE / 2)
#define MAP_REAL_X_TO_GRID_X(rx)        ((rx) / (GRID_SIZE))
#define MAP_REAL_Y_TO_GRID_Y(ry)        ((ry) / (GRID_SIZE))
#define MAX_BLOCKS_NUM                  ((HORIZON_GRID_NUMBER - 1) * VERTICAL_GRID_NUMBER \
                                        + (VERTICAL_GRID_NUMBER - 1) * HORIZON_GRID_NUMBER)

#define A_STAR_GRID_PER_GRID            1.0
#define A_STAR_GRID_SIZE                (GRID_SIZE / A_STAR_GRID_PER_GRID)
#define HORIZON_A_STAR_GRID_NUMBER      (A_STAR_GRID_PER_GRID * HORIZON_GRID_NUMBER)
#define VERTICAL_A_STAR_GRID_NUMBER     (A_STAR_GRID_PER_GRID * VERTICAL_GRID_NUMBER)
#define MAP_REAL_X_TO_A_STAR_X(rx)      (static_cast<int>((rx) / A_STAR_GRID_SIZE))
#define MAP_REAL_Y_TO_A_STAR_Y(ry)      (static_cast<int>((ry) / A_STAR_GRID_SIZE))
#define MAP_A_STAR_X_TO_REAL_X(ax)      static_cast<double>((ax) * A_STAR_GRID_SIZE + A_STAR_GRID_SIZE / 2)
#define MAP_A_STAR_Y_TO_REAL_Y(ay)      static_cast<double>((ay) * A_STAR_GRID_SIZE + A_STAR_GRID_SIZE / 2)

#define WINDOW_WIDTH                    GAME_VIEW_WIDTH + 100
#define WINDOW_HEIGHT                   GAME_VIEW_HEIGHT

typedef std::vector<double>             Color;
#define BLACK                           {0, 0, 0}
#define RED                             {1.0, 0.3, 0.3}
#define BLUE                            {0.3, 0.3, 1.0}
#define GREEN                           {0.3, 1.0, 0.3}
#define YELLOW                          {0.9, 0.9, 0.3}
#define GREY                            {0.3, 0.3, 0.3}

#endif //TANK_TROUBLE_DEFS_H
