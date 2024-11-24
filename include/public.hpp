#ifndef PUBLIC_H
#define PUBLIG_H

//server和client的公共文件


enum EnMsgType
{
    LOGIN_MSG=1, //登录信息
    LOGIN_MSG_ACK, //登录响应信息
    LOGINOUT_MSG,   //注销信息
    REG_MSG,  //注册信息
    REG_MSG_ACK,   //注册响应信息
    ONE_CHAT_MSG,   //一对一聊天信息
    ADD_FRIEND_MSG,  //添加好友信息


    CREATE_GROUP_MSG,    //创建群组信息
    ADD_GROUP_MSG,       //加入群组信息
    GROUP_CHAT_MSG      //群组聊天信息
};


#endif