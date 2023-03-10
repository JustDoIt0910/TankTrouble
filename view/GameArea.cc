//
// Created by zr on 23-2-8.
//

#include "GameArea.h"
#include "defs.h"
#include <glibmm/main.h>

namespace TankTrouble
{
    GameArea::GameArea(Controller* c): ctl(c)
    {
        Glib::signal_timeout().connect(
                sigc::mem_fun(*this, &GameArea::on_timeout),
                10);
    }

    static void drawOutline(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        GameArea::drawRect(cr, BLACK, util::Vec(0, 0), util::Vec(4, 0),
                           util::Vec(0, GAME_VIEW_HEIGHT), util::Vec(4, GAME_VIEW_HEIGHT));

        GameArea::drawRect(cr, BLACK, util::Vec(GAME_VIEW_WIDTH - 4, 0), util::Vec(GAME_VIEW_WIDTH, 0),
                           util::Vec(GAME_VIEW_WIDTH - 4, GAME_VIEW_HEIGHT), util::Vec(GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT));

        GameArea::drawRect(cr, BLACK, util::Vec(0, 0), util::Vec(GAME_VIEW_WIDTH, 0),
                           util::Vec(0, 4), util::Vec(GAME_VIEW_WIDTH - 1, 4));

        GameArea::drawRect(cr, BLACK, util::Vec(0, GAME_VIEW_HEIGHT - 4), util::Vec(GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT - 4),
                           util::Vec(0, GAME_VIEW_HEIGHT), util::Vec(GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT));
    }

    //test
    void drawAStarGrid(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        for(int i = 0; i < VERTICAL_A_STAR_GRID_NUMBER - 1; i++)
        {
            cr->move_to(0, (i + 1) * A_STAR_GRID_SIZE);
            cr->line_to(GAME_VIEW_WIDTH - 1, (i + 1) * A_STAR_GRID_SIZE);
        }
        for(int i = 0; i < HORIZON_A_STAR_GRID_NUMBER - 1; i++)
        {
            cr->move_to((i + 1) * A_STAR_GRID_SIZE, 0);
            cr->line_to((i + 1) * A_STAR_GRID_SIZE, GAME_VIEW_HEIGHT - 1);
        }
        cr->stroke();
    }

    bool GameArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
    {
        drawOutline(cr);
        Controller::BlockList* blocks = ctl->getBlocks();
        for(auto& block: *blocks)
            block.second.draw(cr);

        Controller::ObjectListPtr snapshot = ctl->getObjects();
        for(auto &entry: *snapshot)
            entry.second->draw(cr);

        return true;
    }

    void GameArea::drawRect(const Cairo::RefPtr<Cairo::Context>& cr,
                            Color color,
                            util::Vec tl, util::Vec tr, util::Vec bl, util::Vec br)
    {
        cr->set_source_rgb(color[0], color[1], color[2]);
        cr->set_line_width(1.0);
        cr->move_to(tl.x(), tl.y());
        cr->line_to(tr.x(), tr.y());
        cr->line_to(br.x(), br.y());
        cr->line_to(bl.x(), bl.y());
        cr->close_path();
        cr->fill();
    }

    bool GameArea::on_timeout()
    {
        auto win = get_window();
        if (win)
        {
            Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                             get_allocation().get_height());
            win->invalidate_rect(r, false);
        }
        return true;
    }
}