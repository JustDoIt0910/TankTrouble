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
        put(loginBtn, 340, 230);

        nicknameEntry.set_max_length(50);
        nicknameEntry.set_text("nickname");
        put(nicknameEntry, 300, 150);

        loginBtn.show();
        nicknameEntry.show();
    }
}