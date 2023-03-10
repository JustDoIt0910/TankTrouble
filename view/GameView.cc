//
// Created by zr on 23-3-3.
//

#include "GameView.h"
#include "defs.h"

namespace TankTrouble
{
    GameView::GameView(Controller* ctl):
            gameArea(ctl)
    {
        gameArea.set_size_request(GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT);
        put(gameArea, 0, 0);
        gameArea.show();
    }
}