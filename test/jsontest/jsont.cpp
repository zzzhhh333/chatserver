#include <string>
#include "json.hpp"

using json=nlohmann::json;

#include <iostream>
#include <map>
#include <vector>
std::string func1()
{
    json js;
    js["msg"]="how are you";
    js["name"]="zhangsan";
    js["age"]=18;
    std::string sendbuff=js.dump();
    return sendbuff;
}

std::string func2()
{
    json js;
    std::vector<int>list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    js["list"]=list;

    std::map<int,std::string>mmap;
    mmap.insert({1,"喜羊羊"});
    mmap.insert({2,"沸羊羊"});
    mmap.insert({3,"懒羊羊"});

    js["sheep"]=mmap;
    return js.dump();

}
int main()
{
    std::string recvbuff=func1();
    json jsbuf=json::parse(recvbuff);
    //std::cout<<jsbuf["msg"]<<std::endl;
    //std::cout<<jsbuf["age"]<<std::endl;
    //std::cout<<jsbuf["name"]<<std::endl;
    std::cout<<jsbuf["list"]<<std::endl;
    std::cout<<jsbuf["sheep"]<<std::endl;
    std::map<int,std::string> m=jsbuf["sheep"];
    
    return 0;
}