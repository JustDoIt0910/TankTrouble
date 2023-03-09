//
// Created by zr on 23-3-9.
//

#include "OnlineController.h"
#include "Window.h"
#include "net/TcpClient.h"

namespace TankTrouble
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;

    OnlineController::OnlineController(Window* win, Inet4Address serverAddr):
        Controller(),
        interface(win),
        serverAddress(serverAddr)
    {
        codec.registerHandler(MSG_LOGIN_RESP,
                              std::bind(&OnlineController::onLoginSuccess, this, _1, _2, _3));
        codec.registerHandler(MSG_ROOM_INFO,
                              std::bind(&OnlineController::onRoomsUpdate, this, _1, _2, _3));
    }

    OnlineController::~OnlineController() = default;

    void OnlineController::start()
    {
        controlThread = std::thread(&OnlineController::run, this);
        std::unique_lock<std::mutex> lk(mu);
        cv.wait(lk, [this]() -> bool { return this->started; });
    }

    void OnlineController::run()
    {
        ev::reactor::EventLoop loop;
        controlLoop = &loop;
        client = std::make_unique<TcpClient>(controlLoop, serverAddress);
        client->setMessageCallback(std::bind(&Codec::handleMessage, &codec, _1, _2, _3));
        client->setConnectionCallback(std::bind(&OnlineController::sendLoginMessage, this, _1));
        {
            std::unique_lock<std::mutex> lk(mu);
            started = true;
            cv.notify_all();
        }
        loop.loop();
        controlLoop = nullptr;
    }

    void OnlineController::login(const std::string& name)
    {
        nickname = name;
        client->connect();
    }

    void OnlineController::createNewRoom(const std::string &name, uint8_t cap)
    {
        Message newRoom = codec.getEmptyMessage(MSG_NEW_ROOM);
        newRoom.setField<Field<std::string>>("room_name", name);
        newRoom.setField<Field<uint8_t>>("player_num", cap);
        Buffer buf = Codec::packMessage(MSG_NEW_ROOM, newRoom);
        client->send(buf);
    }

    void OnlineController::joinRoom(uint8_t roomId)
    {
        Message joinRoom = codec.getEmptyMessage(MSG_JOIN_ROOM);
        joinRoom.setField<Field<uint8_t>>("join_room_id", roomId);
        Buffer buf = Codec::packMessage(MSG_JOIN_ROOM, joinRoom);
        client->send(buf);
    }

    OnlineUser OnlineController::getUserInfo()
    {
        std::lock_guard<std::mutex> lg(userInfoMu);
        return userInfo;
    }

    std::vector<RoomInfo> OnlineController::getRoomInfos()
    {
        std::lock_guard<std::mutex> lg(roomInfosMu);
        return std::move(roomInfos);
    }

    void OnlineController::sendLoginMessage(const TcpConnectionPtr& conn)
    {
        Message login = codec.getEmptyMessage(MSG_LOGIN);
        login.setField<Field<std::string>>("nickname", nickname);
        Codec::sendMessage(conn, MSG_LOGIN, login);
    }

    /********************************** message handlers **************************************/

    void OnlineController::onLoginSuccess(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        std::string name = message.getField<Field<std::string>>("nickname").get();
        uint32_t score = message.getField<Field<uint32_t>>("score").get();
        if(name != nickname)
            return;
        std::lock_guard<std::mutex> lg(userInfoMu);
        userInfo.nickname_ = name;
        userInfo.score_ = score;
        interface->notifyLoginSuccess();
    }

    void OnlineController::onRoomsUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        auto rooms = message.getArray<StructField<uint8_t, std::string, uint8_t, uint8_t>>("room_infos");
        std::vector<RoomInfo> newRoomInfos;
        for(int i = 0; i < rooms.length(); i++)
        {
            auto id = rooms.get(i).get<uint8_t>("room_id");
            auto name = rooms.get(i).get<std::string>("room_name");
            auto cap = rooms.get(i).get<uint8_t>("room_cap");
            auto players = rooms.get(i).get<uint8_t>("room_players");
            RoomStatus status = players < cap ? Waiting : Playing;
            newRoomInfos.emplace_back(id, name, cap, players, status);
        }
        std::lock_guard<std::mutex> lg(roomInfosMu);
        roomInfos = std::move(newRoomInfos);
        interface->notifyRoomUpdate();
    }
}