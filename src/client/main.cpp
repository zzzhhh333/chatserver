#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>
#include <muduo/net/TcpClient.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <semaphore.h>

#include "json.hpp"
#include "public.hpp"
#include "user.hpp"
#include "group.hpp"
#include "groupuser.hpp"

sem_t sem;

using namespace std;
using json = nlohmann::json;

User g_currentuser;                   // 当前用户信息
vector<User> g_currentuserfriendlist; // 当前用户好友信息
vector<Group> g_currentusergrouplist; // 当前用户群组信息
static bool isRunning = true;
static bool isLogining = false;

void ShowCurrentUserInfo();   // 显示当前用户信息
string getCurrentTime();      // 获取当前系统时间
void readTaskHandler(int fd); // 读取任务线程
void mainMenu(int fd);        // 聊天主菜单页面
void help(int fd = -1, string str = "");
void chat(int fd, string str);
void addfriend(int fd, string str);
void creategroup(int fd, string str);
void addgroup(int fd, string str);
void groupchat(int fd, string str);
void loginout(int fd = -1, string str = "");

const int MAX_UserNameLength = 10;

int main(int argv, char **args)
{
    if (argv < 3)
    {
        cerr << "comman invaild !! Example like : ./chatClient 127.0.0.1 6000";
        exit(-1);
    }

    // 从命令行接收服务器端的IP和端口
    char *ip = args[1];
    uint16_t port = stoi(args[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        cerr << "socket error";
        exit(-1);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    int res = connect(clientfd, (struct sockaddr *)&addr, sizeof(addr));
    if (res == -1)
    {
        cerr << "connect failed";
        close(clientfd);
        exit(-1);
    }

    // 初始化信号量
    sem_init(&sem, 0, 0);

    thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    while (1)
    {
        cout << "----------------" << endl;
        cout << "1.登录" << endl;
        cout << "2.注册" << endl;
        cout << "3.退出" << endl;
        cout << "----------------" << endl;

        int choice;
        cin >> choice;
        cin.get();

        isLogining = false;
        switch (choice)
        {

        // 登录业务
        case 1:
        {
            int id;
            string pwd = "";

            cout << "请输入id和密码:" << endl;
            cout << "ID:";
            cin >> id;
            cin.get();
            cout << "密码:";
            getline(cin, pwd);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if (len == -1)
            {
                cerr << "send error!" << endl;
                close(clientfd);
                exit(-1);
            }
            else
            {
                sem_wait(&sem);

                if (isLogining)
                {
                    // 进入聊天主菜单页面
                    isRunning = true;
                    mainMenu(clientfd);
                }
            }
        }
        break;

        // 注册业务
        case 2:
        {
            string name = "";
            string pwd = "";
            string repwd = "";
            char buff[1024] = {0};
            while (1)
            {
                cout << "请输入用户名和密码：" << endl;
                cout << "用户名：" << endl;
                getline(cin, name);
                cout << "密  码：" << endl;
                getline(cin, pwd);
                cout << "确认密码：" << endl;
                getline(cin, repwd);
                if (pwd != repwd)
                {
                    cout << "两次密码输入不相同，请重试!" << endl;
                    continue;
                }
                if (name.length() <= MAX_UserNameLength) // 判断用户名的长度是否合法
                {
                    break;
                }
                else
                {
                    cout << "用户名过长，请重新输入！" << endl;
                }
            }

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;

            string usermsg = js.dump();

            int res = send(clientfd, usermsg.c_str(), strlen(usermsg.c_str()) + 1, 0);
            if (res == -1)
            {
                cerr << "send error" << endl;
                close(clientfd);
                exit(-1);
            }
            else
            {
                sem_wait(&sem);
            }
        }
        break;

        // 退出客户端
        case 3:
        {
            close(clientfd);
            exit(0);
        }
        break;
        default:
            break;
        }
    }
    return 0;
}

// 显示当前用户信息
void ShowCurrentUserInfo()
{
    cout << "=========我的信息=========" << endl;
    cout << "用户ID:" << g_currentuser.getId() << endl;
    cout << "用户名:" << g_currentuser.getName() << endl;
    cout << "状态:" << g_currentuser.getState() << endl;

    cout << "---------好友列表---------" << endl;
    if (!g_currentuserfriendlist.empty())
    {
        for (auto &user : g_currentuserfriendlist)
        {
            cout << "ID:" << user.getId() << " " << "用户名:" << user.getName() << "状态:" << user.getState() << endl;
        }
    }
    else
    {
        cout << "当前还没有好友哦...,快去添加好友吧" << endl;
    }

    cout << "*********我的群组*********" << endl;
    if (!g_currentusergrouplist.empty())
    {
        for (auto &group : g_currentusergrouplist)
        {
            cout << "群组号:" << group.getId() << "群组名:" << group.getGroupName() << "群组描述:" << group.getGroupDesc();
            cout << "群成员信息:" << endl;
            for (auto &user : group.getUsers())
            {
                cout << "群员ID:" << user.getId() << "群员昵称:" << user.getName() << "群员状态:" << user.getState() << "群员属性:" << user.getGroupRole() << endl;
            }
            cout << "---------------------------" << endl;
        }
        cout << "**********************" << endl;
    }
    else
    {
        cout << "当前还没有群组哦....快去创建或加入别人的群组吧" << endl;
    }
}

// 获取当前系统时间
string getCurrentTime()
{
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // 转为字符串
    std::stringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%d-%H-%M-%S");
    std::string str_time = ss.str();
    return str_time;
}

// 处理注册业务
void doRegister(json response)
{
    int err = response["errno"].get<int>();
    if (err == 0) // 注册成功
    {
        int userid = response["id"];
        int msgid = response["msgid"];
        cout << "注册成功!!!请记好您的用户id:" << userid << endl;
    }
    else // 注册失败
    {
        cerr << "用户名已经存在，注册失败!" << endl;
    }
}

// 处理登录业务
void doLogin(json response)
{
    int err = response["errno"].get<int>();
    if (err != 0) // 登录失败
    {
        string errmsg = response["errmsg"];
        cout << errmsg << endl;
    }
    else // 登录成功
    {
        isLogining = true;
        // 初始化
        User user(-1, "", "", "offline");
        g_currentuser = user;

        // 保存当前用户信息
        g_currentuser.setId(response["id"]);
        g_currentuser.setName(response["name"]);
        g_currentuser.setState(response["state"]);
        g_currentuser.setPassword(response["password"]);

        // 保存当前用户好友信息
        if (response.contains("friends"))
        {
            // 初始化
            g_currentuserfriendlist.clear();

            vector<string> vecfriends = response["friends"];
            User user;
            for (auto &vec : vecfriends)
            {
                json js1 = json::parse(vec);
                user.setId(js1["id"].get<int>());
                user.setName(js1["name"]);
                user.setState(js1["state"]);
                g_currentuserfriendlist.push_back(user);
            }
        }

        // 保存当前用户群组信息
        if (response.contains("group"))
        {
            vector<string> vec = response["group"];

            vector<Group> grouplist;
            groupUser guser;
            for (auto &group : vec)
            {
                Group group1;
                json js1 = json::parse(group);
                group1.setId(js1["gid"].get<int>());
                group1.setGroupName(js1["gname"]);
                group1.setGroupDesc(js1["gdesc"]);
                vector<string> vec1 = js1["gusers"];
                for (auto &user : vec1)
                {
                    json js2 = json::parse(user);
                    guser.setId(js2["id"].get<int>());
                    guser.setName(js2["name"]);
                    guser.setState(js2["state"]);
                    guser.setGroupRole(js2["role"]);
                    group1.getUsers().push_back(guser);
                }
                grouplist.push_back(group1);
            }
            // 初始化
            g_currentusergrouplist.clear();

            g_currentusergrouplist = grouplist;
        }

        // 显示当前用户的信息
        ShowCurrentUserInfo();

        // 显示离线消息
        if (response.contains("offlinemsg"))
        {
            vector<string> offlinemsg = response["offlinemsg"];
            for (auto &msg : offlinemsg)
            {
                json offlinejs = json::parse(msg);
                // userid from username to otherid msg
                // id from:    to:
                if (offlinejs["msgid"] == ONE_CHAT_MSG)
                    cout << offlinejs["time"] << "[" << offlinejs["id"] << "]" << offlinejs["name"] << "说:" << offlinejs["msg"] << endl;
                else
                {
                    cout << "群组" << "[" << offlinejs["groupid"] << "]:" << "time:" << offlinejs["time"] << "[" << offlinejs["userid"] << "]" << offlinejs["name"] << "说:" << offlinejs["msg"] << endl;
                }
            }
        }
    }
}

// 接收线程
void readTaskHandler(int fd)
{
    while (1)
    {
        char buff[1024] = {0};
        int len = recv(fd, buff, sizeof(buff), 0);
        if (len == -1)
        {
            cerr << "recv error!" << endl;
            close(fd);
            exit(-1);
        }
        else
        {
            json response = json::parse(buff);
            int msgtype = response["msgid"].get<int>();
            if (msgtype == ONE_CHAT_MSG)
                cout << "time:" << response["time"] << "[" << response["id"] << "]" << response["name"] << "说:" << response["msg"] << endl;
            else if (msgtype == GROUP_CHAT_MSG)
            {
                cout << "群组" << "[" << response["groupid"] << "]:" << "time:" << response["time"] << "[" << response["userid"] << "]" << response["name"] << "说:" << response["msg"] << endl;
            }
            else if (msgtype == LOGIN_MSG_ACK)
            {
                doLogin(response);
                sem_post(&sem);
            }
            else if (msgtype == REG_MSG_ACK)
            {
                doRegister(response);
                sem_post(&sem);
            }
        }
    }
}
// 系统支持的客户端命令
unordered_map<string, string> CommandMap = {
    {"help", "显示所有支持的命令,格式help"},
    {"chat", "一对一聊天,格式chat:friendid:message"},
    {"addfriend", "添加好友,格式addfriend:friendid"},
    {"creategroup", "创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组,格式addgroup:groupid"},
    {"groupchat", "群聊,格式groupchat:groupid:message"},
    {"loginout", "注销,格式loginout"}};

unordered_map<string, function<void(int, string)>> CommandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

// 聊天主菜单页面
void mainMenu(int fd)
{
    help();
    char buffer[1024];
    while (isRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuff(buffer);
        string command;

        // 判断该指令是哪一种类型，将指令名对应的字符串取出来
        int index = commandbuff.find(":");
        if (index != string::npos)
        {
            command = commandbuff.substr(0, index);
        }
        else
        {
            command = commandbuff;
        }
        // 从指令集当中去查找该指令
        auto it = CommandHandlerMap.find(command);
        if (it != CommandHandlerMap.end())
        {
            // 找到后执行对应的指令函数
            it->second(fd, commandbuff.substr(index + 1));
        }
        else
        {
            cerr << "输入指令有误，请重新输入" << endl;
            continue;
        }
    }
}

void help(int fd, string str)
{
    for (auto &it : CommandMap)
    {
        cout << it.first << ":" << it.second << endl;
    }
    cout << endl;
}

// 一对一聊天
void chat(int fd, string str)
{
    int idx = str.find(":");
    string str1;
    string mmsg;
    int friend_id;
    if (idx != string::npos)
    {
        str1 = str.substr(0, idx);
        friend_id = stoi(str1);
        mmsg = str.substr(idx + 1);
        json chatjs;
        chatjs["msgid"] = ONE_CHAT_MSG;
        chatjs["time"] = getCurrentTime();
        chatjs["name"] = g_currentuser.getName();
        chatjs["id"] = g_currentuser.getId();
        chatjs["to"] = stoi(str);
        chatjs["msg"] = mmsg;
        int len = send(fd, chatjs.dump().c_str(), strlen(chatjs.dump().c_str()) + 1, 0);
        if (len == -1)
        {
            cerr << "chat send error" << endl;
        }
    }
    else
    {
        cerr << "输入指令有误，请重新输入" << endl;
        return;
    }
}

// 添加好友
void addfriend(int fd, string str)
{
    json friendjs;
    friendjs["msgid"] = ADD_FRIEND_MSG;
    friendjs["friendid"] = stoi(str);
    friendjs["userid"] = g_currentuser.getId();
    int len = send(fd, friendjs.dump().c_str(), strlen(friendjs.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "addfriend send error" << endl;
    }
}

// 创建群组
void creategroup(int fd, string str)
{
    int idx = str.find(":");
    if (idx != string::npos)
    {
        string gname = str.substr(0, idx);
        string gdesc = str.substr(idx + 1);

        json js;
        int userid = g_currentuser.getId();
        js["msgid"] = CREATE_GROUP_MSG;
        js["userid"] = userid;
        js["groupname"] = gname;
        js["groupdesc"] = gdesc;

        int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
        if (len == -1)
        {
            cerr << "creategroup send error" << endl;
        }
    }
    else
    {
        cerr << "输入指令有误，请重新输入" << endl;
        return;
    }
}

// 加入群组
void addgroup(int fd, string str)
{
    int userid = g_currentuser.getId();
    int gid = stoi(str);
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["userid"] = userid;
    js["groupid"] = gid;

    int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "addgroup send error" << endl;
    }
}

// 群聊
void groupchat(int fd, string str)
{
    int idx = str.find(":");
    if (idx != string::npos)
    {
        string gid = str.substr(0, idx);
        string msg = str.substr(idx + 1);

        int userid = g_currentuser.getId();
        json js;
        js["msgid"] = GROUP_CHAT_MSG;
        js["userid"] = g_currentuser.getId();
        js["groupid"] = stoi(gid);
        js["msg"] = msg;
        js["time"] = getCurrentTime();
        js["name"] = g_currentuser.getName();

        int len = send(fd, js.dump().c_str(), strlen(js.dump().c_str()) + 1, 0);
        if (len == -1)
        {
            cerr << "groupchat send error" << endl;
        }
    }
    else
    {
        cerr << "输入指令有误，请重新输入" << endl;
        return;
    }
}

// 注销
void loginout(int fd, string str)
{
    json jsquit;
    jsquit["userid"] = g_currentuser.getId();
    jsquit["msgid"] = LOGINOUT_MSG;

    int len = send(fd, jsquit.dump().c_str(), strlen(jsquit.dump().c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "loginout send error" << endl;
    }
    else
    {
        isRunning = false;
    }
}