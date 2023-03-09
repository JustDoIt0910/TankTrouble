//
// Created by zr on 23-3-8.
//

#ifndef TANK_TROUBLE_LOGIN_VIEW_H
#define TANK_TROUBLE_LOGIN_VIEW_H
#include <gtkmm.h>


namespace TankTrouble
{
    class LoginView : public Gtk::Fixed
    {
    public:
        LoginView();
        sigc::signal<void, std::string> signal_login_clicked();

    private:
        void login();

        Gtk::Button loginBtn;
        Gtk::Entry nicknameEntry;
        sigc::signal<void, std::string> login_s;
    };
}

#endif //TANK_TROUBLE_LOGIN_VIEW_H
