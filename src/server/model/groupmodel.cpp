#include "groupmodel.hpp"
#include "db.h"

// 创建聊天组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup(groupname,groupdesc) value('%s','%s')", group.getGroupName().c_str(), group.getGroupDesc().c_str());

    MySql mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 将用户添加进对应聊天组中
void GroupModel::addGroup(int gid, int userid, string grole)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into GroupUser(groupid,userid,grouprole) value('%d','%d','%s')", gid, userid, grole.c_str());

    MySql mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户所在组的信息
vector<Group> GroupModel::queryGroups(int userid)
{
    /*
    1.先根据userid在groupuser表中查询出该用户所属群组信息
    2.再根据群组信息，查询属于该群组的所有用户的userid，并且和user表进行多表联合查询，查出用户的详细信息
     */
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on b.groupid=a.id  where b.userid = %d", userid);
    MySql mysql;
    vector<Group> vec;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(stoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);
                vec.push_back(group);
            }
        }
        mysql_free_result(res);
    }

    // 根据群组信息拿到群组里所有组员的信息
    for (auto &group : vec)
    {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid=a.id  where b.groupid = %d", group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                groupUser users;
                users.setId(stoi(row[0]));
                users.setName(row[1]);
                users.setState(row[2]);
                users.setGroupRole(row[3]);
                group.getUsers().push_back(users);
            }
            mysql_free_result(res);
        }
    }
    
    return vec;
}

// 查询指定群组中除自己以外其他所有人的userid
vector<int> GroupModel::queryGroupUsers(int groupid, int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where userid !=%d and groupid =%d", userid, groupid);
    MySql mysql;
    vector<int> vec;

    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
                vec.push_back(stoi(row[0]));
            mysql_free_result(res);
        }
    }
    return vec;
}