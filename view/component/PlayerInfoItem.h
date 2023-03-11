//
// Created by zr on 23-3-11.
//

#ifndef TANK_TROUBLE_PLAYER_INFO_ITEM_H
#define TANK_TROUBLE_PLAYER_INFO_ITEM_H
#include <gtkmm.h>

#include <utility>
#include "defs.h"

namespace TankTrouble
{
    class PlayerInfoItem : public Gtk::Fixed
    {
        class PlayerIcon : public Gtk::DrawingArea
        {
        public:
            explicit PlayerIcon(Color color_): color(std::move(color_))
            {set_size_request(50, 30);}

        private:
            bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override
            {
                cr->save();
                cr->set_source_rgb(color[0], color[1], color[2]);
                cr->move_to(5, 5);
                cr->line_to(33, 5);
                cr->line_to(33, 25);
                cr->line_to(5, 25);
                cr->close_path();
                cr->fill();
                cr->set_source_rgb(color[0] - 0.3, color[1] - 0.3, color[2] - 0.3);
                cr->arc(19, 15, 9, 0.0, 2 * M_PI);
                cr->fill();
                cr->set_line_width(9.0);
                cr->move_to(19, 15);
                cr->line_to(2, 15);
                cr->stroke();
                cr->restore();
                return true;
            }

            Color color;
        };

    public:
        explicit PlayerInfoItem(const std::string& nickname, int score, Color color = RED);

    private:
        PlayerIcon playerIcon;
        Gtk::Label nicknameLabel;
        Gtk::Label scoreLabel;
    };
}

#endif //TANK_TROUBLE_PLAYER_INFO_ITEM_H
