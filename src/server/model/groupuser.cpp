#include "groupuser.hpp"

void groupUser::setGroupRole(string grole)
{
    this->grouprole=grole;
}

string groupUser::getGroupRole()
{
    return this->grouprole;
}

void groupUser::setGroupId(int gid)
{
    this->groupid=gid;
}

int groupUser::getGroupId()
{
    return this->groupid;
}