#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace placeholders;

using json=nlohmann::json;


chatserver::chatserver(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg)
            :_server(loop,listenAddr,nameArg),_loop(loop)
{
    _server.setConnectionCallback(bind(&chatserver::onConnection,this,_1));  
    _server.setMessageCallback(bind(&chatserver::onMessage,this,_1,_2,_3));

    _server.setThreadNum(4);
}

//开启服务器
void chatserver::start()
{
    _server.start();
}

//处理服务器连接
void chatserver::onConnection(const TcpConnectionPtr& conn)
{
    if(!conn->connected())
    {
        chatservice::instance()->ClientCloseExpection(conn);
        conn->shutdown();
    }
}

//处理服务器通信
void chatserver::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    string buff=buffer->retrieveAllAsString();
    //数据的反序列化
    json js=json::parse(buff);
    auto msgHanlder=chatservice::instance()->getMsgHandler(js["msgid"].get<int>());
    //回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHanlder(conn,js,time);
}