#include "offlinemessagemodel.hpp"
#include "db.h"

//插入离线数据信息
void offlineMessageModel::insert(int userid,string msg)
{
    char sql[1024]={0};
    sprintf(sql,"insert into OfflineMessage(userid,message) value(%d,'%s')",userid,msg.c_str());
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}


//查询对应用户的离线数据
vector<string> offlineMessageModel::query(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"select message from OfflineMessage where userid =%d",userid);
    MySql mysql;
    vector<string> vec;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res !=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                vec.push_back(row[0]);
            }
        }
        mysql_free_result(res);
    }
    return vec;
}

//删除对应用户的离线数据
void offlineMessageModel::remove(int userid)
{
    char sql[1024]={0};
    sprintf(sql,"delete from OfflineMessage where userid =%d",userid);
    MySql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

