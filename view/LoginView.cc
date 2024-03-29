//
// Created by zr on 23-3-8.
//

#include "LoginView.h"

namespace TankTrouble
{
    LoginView::LoginView()
    {
        loginBtn.set_label("登录");
        loginBtn.set_size_request(80, 40);
        loginBtn.signal_clicked().connect(sigc::mem_fun(*this, &LoginView::login));
        put(loginBtn, 340, 230);

        nicknameEntry.set_max_length(50);
        nicknameEntry.set_editable(true);
        put(nicknameEntry, 300, 150);

        loginBtn.show();
        nicknameEntry.show();
    }

    sigc::signal<void, std::string> LoginView::signal_login_clicked() {return login_s;}

    void LoginView::login()
    {
        std::string nickname = nicknameEntry.get_text();
        if(!nickname.empty())
            login_s.emit(nickname);
    }
}