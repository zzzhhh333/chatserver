#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
using namespace muduo::net;
using namespace muduo;
using namespace std;
using namespace placeholders;
class ChatServer
{
public:
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg)
            :_server(loop,listenAddr,nameArg),loop(loop)
    {
        //给服务器注册用户连接的创建和断开的回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));

        //给服务器注册用户读写事件的回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));

        //设置服务器的线程数量
        _server.setThreadNum(4);

    }   
    void start()
    {
        _server.start();
    }

private:
    TcpServer _server;
    EventLoop* loop;

    //用户连接的创建和断开
    void onConnection(const TcpConnectionPtr&con)
    {
        if(con->connected())
            cout<<con->peerAddress().toIpPort()<<"->"<<con->localAddress().toIpPort()<<"state:online"<<endl;
        else
        {
            cout << con->peerAddress().toIpPort() << "->" << con->localAddress().toIpPort() << "state:offline" << endl;
            con->shutdown(); //close(fd)
            //loop->quit()
        }
    }

    //用户的读写事件
    void onMessage(const TcpConnectionPtr &con,Buffer *buf,Timestamp time)
    {
        string buff=buf->retrieveAllAsString();
        cout<<"recv data:"<<buff<<"time :"<<time.toString()<<endl;
        con->send(buff);
    }
};



int main()
{
    EventLoop loop;
    InetAddress addr("192.168.184.149",6000);
    ChatServer server(&loop,addr,"chatserver");
    server.start();
    loop.loop();  //epoll_wait 以阻塞方式等待新用户连接，以连接用户的读写事件等
    return 0;
}

