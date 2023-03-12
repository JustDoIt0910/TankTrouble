//
// Created by zr on 23-3-3.
//

#include "GameView.h"
#include "defs.h"

namespace TankTrouble
{
    GameView::GameView(Controller* ctl):
            ctl(ctl),
            gameArea(ctl)
    {
        gameArea.set_size_request(GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT);
        put(gameArea, 0, 0);
        quitBtn.set_label("退出");
        quitBtn.set_size_request(50, 40);
        quitBtn.set_can_focus(false);
        quitBtn.signal_clicked().connect(sigc::mem_fun(*this, &GameView::onQuit));
        put(quitBtn, WINDOW_WIDTH - 60, WINDOW_HEIGHT - 50);
        Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &GameView::getPlayersInfo),
                30);
        show_all_children();
    }

    bool GameView::getPlayersInfo()
    {
        for(auto& item : playerInfoItems)
            remove(item);
        playerInfoItems.clear();
        std::vector<PlayerInfo> players = std::move(ctl->getPlaysInfo());
        for(const PlayerInfo& player: players)
            playerInfoItems.emplace_back(player.nickname_, player.score_, player.color_);
        double gap = (GAME_VIEW_HEIGHT - 50) / static_cast<double>(playerInfoItems.size());
        for(int i = 0; i < playerInfoItems.size(); i++)
        {
            put(playerInfoItems[i], GAME_VIEW_WIDTH, i * gap + 50);
            playerInfoItems[i].show();
        }
        return true;
    }

    sigc::signal<void> GameView::signal_quit_game() {return quit_s;}

    void GameView::onQuit()
    {
        ctl->quitGame();
        quit_s.emit();
    }
}