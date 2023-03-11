//
// Created by zr on 23-3-11.
//

#include "PlayerInfoItem.h"

namespace TankTrouble
{
    PlayerInfoItem::PlayerInfoItem(const std::string& nickname, int score, Color color):
        playerIcon(std::move(color))
    {
        nicknameLabel.set_label(nickname);
        scoreLabel.set_label(std::to_string(score));
        put(playerIcon, 10, 0);
        put(nicknameLabel, 10, 30);
        put(scoreLabel, 30, 50);
        show_all_children();
    }
}