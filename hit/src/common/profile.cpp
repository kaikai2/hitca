#include "common/profile.h"

DEF_SINGLETON(Profile);

void Profile::EndLast()
{
    if (end || nodes.empty())
        return;
    Node &n = nodes.back();
    QueryPerformanceCounter(&n.end);
    end = true;
    assert(n.id < summary.size());
    SumNode &sn = summary[n.id];
    sn.logCount++;
    float time = double(n.end.QuadPart - n.start.QuadPart) / double(freq.QuadPart);
    sn.lastTime = time;
    sn.totalTime += time;
    if (time > sn.maxTime)
        sn.maxTime = time;
}

void Profile::Log(const char *name)
{
    EndLast();
    Node n;
    n.name = name;
    QueryPerformanceCounter(&n.start);
    map<string, size_t>::iterator im = maps.find(n.name);
    if (im == maps.end())
    {
        SumNode sn;
        sn.name = n.name;
        sn.totalTime = 0;
        sn.maxTime = 0;
        sn.logCount = 0;
        sn.lastTime = 0;
        maps[sn.name] = n.id = summary.size();
        summary.push_back(sn);
    }
    else
    {
        n.id = maps[name];
    }
    nodes.push_back(n);
    end = false;
}

void Profile::getResult(vector<pair<string, float> > &result) const
{
    result.clear();
    result.reserve(nodes.size());
    for (vector<Node>::const_iterator in = nodes.begin(); in != nodes.end(); ++in)
    {
        float time = double(in->end.QuadPart - in->start.QuadPart) / double(freq.QuadPart);
        result.push_back(pair<string, float>(in->name, time));
    }
}