#include <muduo/base/Logging.h>
#include "chatservice.hpp"
#include "public.hpp"
#include "user.hpp"

// 获取单例
chatservice *chatservice::instance()
{
    static chatservice chatservice;
    return &chatservice;
}

// 注册消息以及对应的Handler回调操作
chatservice::chatservice()
{
    _msgHandlerMap.insert({LOGIN_MSG, bind(&chatservice::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, bind(&chatservice::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, bind(&chatservice::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, bind(&chatservice::addFriend, this, _1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, bind(&chatservice::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, bind(&chatservice::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, bind(&chatservice::groupChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, bind(&chatservice::loginout, this, _1, _2, _3)});

    //连接redis服务器
    if (_redis.connect())
    {
        //设置上报信息的回调
        _redis.init_notify_handler(bind(&chatservice::HandlerRedisSubscribeMessage,this,_1,_2));
    }
}

// 获取消息对应的处理器
MsgHandler chatservice::getMsgHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " " << "can not find msgid";
        };
    }
    return _msgHandlerMap[msgid];
}

// 处理登录业务
void chatservice::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string password = js["password"];

    User user = _userModel.query(id);

    if (user.getId() == id && user.getPassword() == password)
    {
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该账号已经登录，请重新输入账号";
            conn->send(response.dump());
        }
        else
        {
            // 登录成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConMap.insert({id, conn});
            }
            // 登录成功后更新用户状态信息
            user.setState("online");
            _userModel.updateState(user);

            // 在redis上订阅该用户的通道（id）
            _redis.subscribe(id);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = id;
            response["name"] = user.getName();
            response["state"] = user.getState();
            response["password"] = password;

            // 查询该用户是否有离线消息
            vector<string> vec;
            vec = _offlinemsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 拿到离线消息后立刻删除
                _offlinemsgModel.remove(id);
            }

            // 返回该用户的好友信息
            vector<User> vecUser;
            vecUser = _friendModel.query(id);
            if (!vecUser.empty())
            {
                vector<string> vec2;
                for (auto &user : vecUser)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 返回用户的群组信息
            vector<Group> vecGroup;
            vecGroup = _groupModel.queryGroups(id);
            if (!vecGroup.empty())
            {
                vector<string> vec2;
                for (auto &group : vecGroup)
                {
                    json jsg;
                    vector<string> vec3;
                    jsg["gid"] = group.getId();
                    jsg["gname"] = group.getGroupName();
                    jsg["gdesc"] = group.getGroupDesc();
                    vector<groupUser> guservec = group.getUsers();
                    for (auto &users : guservec)
                    {
                        json groupuserjs;
                        groupuserjs["id"] = users.getId();
                        groupuserjs["name"] = users.getName();
                        groupuserjs["state"] = users.getState();
                        groupuserjs["role"] = users.getGroupRole();
                        vec3.push_back(groupuserjs.dump());
                    }
                    jsg["gusers"] = vec3;
                    vec2.push_back(jsg.dump());
                }
                response["group"] = vec2;
            }

            conn->send(response.dump());
        }
    }
    else
    {
        // 账号不存在或者密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "账号或密码不正确，请重新输入";
        conn->send(response.dump());
    }
}

// 处理注销业务
void chatservice::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConMap.find(userid);
        if (it != _userConMap.end())
        {
            _userConMap.erase(it);
        }
    }

    // 在redis上取消订阅
    _redis.unsubscribe(userid);

    // 更新用户的状态
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

// 处理注册业务
void chatservice::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state = _userModel.insert(user);
    json response;
    if (state) // 注册成功
    {
        response["id"] = user.getId();
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        conn->send(response.dump());
    }
    else // 注册失败
    {
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn->send(response.dump());
    }
}

// 处理用户的异常退出事件
void chatservice::ClientCloseExpection(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConMap.begin(); it != _userConMap.end(); it++)
        {
            if (it->second == conn)
            {
                // 从map表删除对应用户的信息
                user.setId(it->first);
                _userConMap.erase(it);
                break;
            }
        }
    }

    // 在redis上取消订阅
    _redis.unsubscribe(user.getId());

    // 更新用户状态信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void chatservice::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConMap.find(toid);
        // toid在线 消息转发
        if (it != _userConMap.end())
        {
            it->second->send(js.dump());
            return;
        }
    }
    User user;
    user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }
    // toid离线 存储消息
    _offlinemsgModel.insert(toid, js.dump());
}

// 重置用户状态
void chatservice::reset()
{
    _userModel.resetState();
}

// 添加好友业务
void chatservice::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int friendid = js["friendid"].get<int>();
    // 将信息插入到Friend表中
    _friendModel.insert(userid, friendid);
}

// 创建群组业务
void chatservice::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    Group group;
    int userid = js["userid"].get<int>();
    string gname = js["groupname"];
    string gdesc = js["groupdesc"];
    group.setGroupName(gname);
    group.setGroupDesc(gdesc);
    // 创建群组
    if (_groupModel.createGroup(group))
    {
        // 储存创建人的信息
        _groupModel.addGroup(group.getId(), userid, _groupModel.create_state);
    }
}

// 加入群组业务
void chatservice::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int gid = js["groupid"].get<int>();
    int userid = js["userid"].get<int>();
    _groupModel.addGroup(gid, userid, _groupModel.add_state);
}

// 群组聊天业务
void chatservice::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> vecUsersId;
    vecUsersId = _groupModel.queryGroupUsers(groupid, userid);
    lock_guard<mutex> lock(_connMutex);
    for (int i : vecUsersId)
    {
        auto it = _userConMap.find(i);
        // 如果好友在线就转发消息
        if (it != _userConMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            User user;
            user = _userModel.query(i);
            if (user.getState() == "online")
            {
                _redis.publish(i, js.dump());
            }
            else
            {
                // 否则将离线消息存储
                _offlinemsgModel.insert(i, js.dump());
            }
        }
    }
}

void chatservice::HandlerRedisSubscribeMessage(int userid,string message)
{
    lock_guard<mutex> lock(_connMutex);
    auto it=_userConMap.find(userid);
    if(it!=_userConMap.end())
    {
        it->second->send(message);
    }
    else
    {
        _offlinemsgModel.insert(userid,message);
    }
}