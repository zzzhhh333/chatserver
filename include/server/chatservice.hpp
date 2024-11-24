#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "redis.hpp"
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"

using namespace std;
using namespace muduo::net;
using namespace muduo;

using json=nlohmann::json;
//表示处理信息的回调方法
using MsgHandler=function<void(const TcpConnectionPtr&, json& ,Timestamp)>;
//聊天服务器业务类
class chatservice
{
public:
    //获取单例
    static chatservice* instance();

    //获取消息对应的处理器
    MsgHandler getMsgHandler(int msgid);

    //redis的回调事件
    void HandlerRedisSubscribeMessage(int userid,string message);

    //处理用户的异常退出事件
    void ClientCloseExpection(const TcpConnectionPtr& conn);
    //重置用户的状态
    void reset();
    
    //处理登录业务
    void login(const TcpConnectionPtr& conn, json& js,Timestamp time);
    //处理注销业务
    void loginout(const TcpConnectionPtr& conn, json& js,Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr& conn, json& js,Timestamp time);
    //处理用户一对一的聊天业务
    void oneChat(const TcpConnectionPtr& conn, json& js,Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json& js,Timestamp time);
    //创建群组业务
    void createGroup(const TcpConnectionPtr& conn, json& js,Timestamp time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr& conn, json& js,Timestamp time);
    //群组聊天业务
    void groupChat(const TcpConnectionPtr& conn, json& js,Timestamp time);

private:
    //将构造函数私有化
    chatservice();
    
    //储存消息ID和与其对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap;

    //储存在线用户的通信连接
    unordered_map<int,TcpConnectionPtr> _userConMap;
   
    //定义互斥锁，保证_userConMap的线程安全
    mutex _connMutex;

    //设置对数据库进行相关操作的实例
    UserModel _userModel;
    offlineMessageModel _offlinemsgModel;
    friendModel _friendModel;
    GroupModel _groupModel;

    //设置redis的实例
    Redis _redis;
};


#endif