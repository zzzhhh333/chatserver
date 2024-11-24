#include "chatserver.hpp"
#include "chatservice.hpp"

#include <iostream>
#include <signal.h>
using namespace std;

void resetsign(int )
{
    chatservice::instance()->reset();
    exit(0);
}


int main(int argn,char* argv[])
{
    if(argn<3)
    {
        cerr << "comman invaild !! Example like : ./ChatServer 127.0.0.1 6000";
        exit(-1);
    }
    string ip=argv[1];
    int port=stoi(argv[2]);
    signal(SIGINT,resetsign);
    EventLoop loop;
    InetAddress addr(ip,port);
    chatserver server(&loop,addr,"CharServer");
    server.start();
    loop.loop();
    return 0;
}