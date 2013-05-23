#ifndef COMMON_PROFILE_H
#define COMMON_PROFILE_H

#include <windows.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "utils.h"

using namespace std;

class Profile : public Singleton<Profile>
{
public:
    Profile() : end(true), totalLogCount(0)
    {
        QueryPerformanceFrequency(&freq);
    }

    struct Node
    {
        string name;
        LARGE_INTEGER start;
        LARGE_INTEGER end;
        size_t id;
        size_t logCount;
    };
    struct SumNode
    {
        string name;
        float totalTime;
        float maxTime;
        float lastTime;
        size_t logCount;
    };
    void EndLast();
    void Log(const char *name);
    void flushFrame()
    {
        totalLogCount++;
        nodes.clear();
    }
    void getResult(vector<pair<string, float> > &result) const;
    const vector<SumNode> &getSummary() const
    {
        return summary;
    }
protected:
    vector<Node> nodes;
    LARGE_INTEGER freq;
    bool end;
    size_t totalLogCount;

    map<string, size_t> maps;
    vector<SumNode> summary;
};
#ifndef USE_PROFILER
#define USE_PROFILER 0
#endif

#if USE_PROFILER
#define PROFILE_CHECKPOINT(a) Profile::getSingleton().Log(#a)
#define PROFILE_ENDPOINT() Profile::getSingleton().EndLast()
#else
#define PROFILE_CHECKPOINT(a)
#define PROFILE_ENDPOINT()
#endif

#endif//COMMON_PROFILE_H