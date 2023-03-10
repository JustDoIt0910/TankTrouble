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
        serverAddress(serverAddr),
        joinedRoomId(0),
        joinStatus(0)
    {
        codec.registerHandler(MSG_LOGIN_RESP,
                              std::bind(&OnlineController::onLoginSuccess, this, _1, _2, _3));
        codec.registerHandler(MSG_ROOM_INFO,
                              std::bind(&OnlineController::onRoomsUpdate, this, _1, _2, _3));
        codec.registerHandler(MSG_JOIN_ROOM_RESP,
                              std::bind(&OnlineController::onJoinRoomRespond, this, _1, _2, _3));
        codec.registerHandler(MSG_GAME_ON,
                              std::bind(&OnlineController::onGameOn, this, _1, _2, _3));
        codec.registerHandler(MSG_UPDATE_BLOCKS,
                              std::bind(&OnlineController::onBlocksUpdate, this, _1, _2, _3));
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

    /********************************* Interface for main window *********************************/

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

    std::vector<RoomInfo> OnlineController::getRoomInfos(uint8_t* currentRoomId, uint8_t* currentJoinStatus)
    {
        std::lock_guard<std::mutex> lg(roomsInfoMu);
        *currentRoomId = joinedRoomId;
        *currentJoinStatus = joinStatus;
        joinedRoomId = 0;
        joinStatus = 0;
        return roomsInfo;
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
        std::lock_guard<std::mutex> lg(roomsInfoMu);
        roomsInfo = std::move(newRoomInfos);
        interface->notifyRoomUpdate();
    }

    void OnlineController::onJoinRoomRespond(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        uint8_t roomId = message.getField<Field<uint8_t>>("join_room_id").get();
        uint8_t code = message.getField<Field<uint8_t>>("operation_status").get();
        std::lock_guard<std::mutex> lg(roomsInfoMu);
        joinStatus = code;
        if(code == Codec::JOIN_ROOM_SUCCESS)
            joinedRoomId = roomId;
        else
            interface->notifyRoomUpdate();
    }

    void OnlineController::onGameOn(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        auto players = message.getArray<StructField<uint8_t, std::string>>("players_info");
        std::lock_guard<std::mutex> lg(playersInfoMu);
        for(int i = 0; i < players.length(); i++)
        {
            auto playerId = players.get(i).get<uint8_t>("player_id");
            auto playerNickname = players.get(i).get<std::string>("player_nickname");
            playersInfo[playerId] = PlayerInfo(playerNickname);
        }
        interface->notifyGameOn();
    }

    void OnlineController::onBlocksUpdate(const TcpConnectionPtr& conn, Message message, ev::Timestamp)
    {
        auto blocksData = message.getArray<StructField<uint8_t, uint64_t, uint64_t>>("blocks");
        std::lock_guard<std::mutex> lg(blocksMu);
        blocks.clear();
        for(int i = 0; i < blocksData.length(); i++)
        {
            bool horizon = (blocksData.get(i).get<uint8_t>("is_horizon") == 1);
            auto x_ = blocksData.get(i).get<uint64_t>("center_x");
            auto y_ = blocksData.get(i).get<uint64_t>("center_y");
            double x, y;
            memcpy(&x, &x_, sizeof(double));
            memcpy(&y, &y_, sizeof(double));
            blocks[i + 1] = Block(horizon, util::Vec(x, y));
        }
    }
}