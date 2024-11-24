#include "group.hpp"

Group::Group(int id,string gname,string gdesc)
    :id(id),groupname(gname),groupdesc(gdesc)
{
    
}

int Group::getId()
{
    return this->id;
}

string Group::getGroupName()
{
    return this->groupname;
}

string Group::getGroupDesc()
{
    return this->groupdesc;
}

vector<groupUser>& Group::getUsers()
{
    return this->users;
}

void Group::setId(int id)
{
    this->id=id;
}

void Group::setGroupName(string gname)
{
    this->groupname=gname;
}

void Group::setGroupDesc(string gdesc)
{
    this->groupdesc=gdesc;
}

void Group::setUsers(vector<groupUser> users)
{
    this->users=users;
}
