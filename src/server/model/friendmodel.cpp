#include "friendmodel.hpp"
#include "db.h"


void friendModel::insert(int userid,int friendid)
{
    char sql[1024];
    sprintf(sql,"insert into Friend(user_id,friend_id) value(%d,%d)",userid,friendid);

    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

vector<User> friendModel::query(int userid)
{
    
    char sql[1024]={0};
    sprintf(sql,"select a.id,a.name,a.state from User a inner join Friend b on b.friend_id=a.id where b.user_id =%d",userid);
    MySql mysql;
    vector<User> vec;
   
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res !=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setId(stoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
        }
        mysql_free_result(res);
    }
    return vec;

}