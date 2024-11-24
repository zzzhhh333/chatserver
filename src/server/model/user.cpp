#include "user.hpp"

User::User(int id,string name,string password,string state)
    :id(id),name(name),password(password),state(state)
{

}


void User::setId(int id)
{
    this->id = id;
}
void User::setName(string name)
{
    this->name = name;
}
void User::setPassword(string pwd)
{
    this->password = pwd;
}
void User::setState(string state)
{
    this->state = state;
}

int User::getId()
{
    return this->id;
}
string User::getName()
{
    return this->name;
}
string User::getPassword()
{
    return this->password;
}
string User::getState()
{
    return this->state;
}