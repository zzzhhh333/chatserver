#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>
using namespace std;

class offlineMessageModel
{
public:
    void insert(int userid,string msg);
    vector<string> query(int id);
    void remove(int userid);
private:
};



#endif