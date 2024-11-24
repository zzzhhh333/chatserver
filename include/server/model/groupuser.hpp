#ifndef GROUPUSER_H
#define GROUPUSER_H
#include "user.hpp"
#include <string>

using namespace std;

//继承User的属性
class groupUser:public User
{
public:
    string getGroupRole();
    int getGroupId();
    void setGroupRole(string grole);
    void setGroupId(int gid);
private:
    string grouprole;
    int groupid;
};


#endif