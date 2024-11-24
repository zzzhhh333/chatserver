#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
using namespace muduo::net;
using namespace muduo;
class chatserver
{
public:
    chatserver(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg);
    void start();

private:
    TcpServer _server;
    EventLoop* _loop;

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
};









#endif