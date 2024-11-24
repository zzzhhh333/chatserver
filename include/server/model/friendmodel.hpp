#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include <vector>

#include "user.hpp"

using namespace std;

class friendModel
{
public:
    //添加好友
    void insert(int userid,int friendid);
    
    //返回用户好友列表信息
    vector<User> query(int userid);


private:
};


#endif