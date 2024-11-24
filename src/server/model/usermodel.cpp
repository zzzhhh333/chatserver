#include <iostream>

#include "usermodel.hpp"
#include "db.h"
using namespace std;


//插入用户信息
bool UserModel::insert(User& user)
{
    //1.组装sql语句
    char sql[1024]={0};
    sprintf(sql,"insert into User(name,password,state) value('%s','%s','%s')",
    user.getName().c_str(),user.getPassword().c_str(),user.getState().c_str());

    MySql mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}


//查询用户ID对应的用户信息
User UserModel::query(int id)
{
    char sql[1024]={0};
    sprintf(sql,"select * from User where id = %d",id);
    MySql mysql;
    User user;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res != nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);
            if (row != nullptr)
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

//更新用户信息
bool UserModel::updateState(User user)
{
    char sql[1024]={0};
    sprintf(sql,"update User set state ='%s' where User.id = %d",user.getState().c_str(),user.getId());
    MySql mysql;
    if(mysql.connect())
    {   
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    char sql[1024]="update User set state ='offline'";
    MySql mysql;
    if(mysql.connect())
    {   
        mysql.update(sql);
    }
}