#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User
{
public:
    User(int id=-1,string name="",string password="",string state="offline");

    void setId(int id);
    void setName(string name);
    void setPassword(string pwd);
    void setState(string state);

    int getId();
    string getName();
    string getPassword();
    string getState();
protected:
    int id;
    string name;
    string password;
    string state;

};



#endif