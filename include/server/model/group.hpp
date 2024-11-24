#ifndef GROUP_H
#define GROUP_H
#include <string>
#include <vector>

#include "groupuser.hpp"

using namespace std;

class Group
{
public:
    Group(int id=-1,string gname="",string gdesc="");

    void setId(int id);
    void setGroupName(string gname);
    void setGroupDesc(string gdesc);
    void setUsers(vector<groupUser> user);
    int getId();
    string getGroupName();
    string getGroupDesc();
    vector<groupUser>& getUsers();
private:
    int id;
    string groupname;
    string groupdesc;
    vector<groupUser> users;
};


#endif