#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include <vector>

#include "groupuser.hpp"
#include "group.hpp"

using namespace std;

class GroupModel
{
public:
    bool createGroup(Group& group);
    void addGroup(int gid,int userid,string grole);
    vector<int> queryGroupUsers(int groupid,int userid);
    vector<Group> queryGroups(int gid);

    const string create_state="creator";
    const string add_state="normal";
private:
    
};



#endif