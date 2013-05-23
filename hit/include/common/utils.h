#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <cassert>

// not allowed to copy
class NoCopy
{
protected:
    NoCopy()
    {
    }
private:
    NoCopy(const NoCopy &o)
    {
        o;
    }
    NoCopy &operator = (const NoCopy &o)
    {
        return o, *this;
    }
};

// singleton
template<typename T>
class Singleton : public NoCopy
{
    static T *ms_Singleton;
public:
    Singleton()
    {
        assert(ms_Singleton == 0);
        ms_Singleton = static_cast<T *>(this);
    }
    virtual ~Singleton()
    {
        assert(ms_Singleton);
        ms_Singleton = 0;
    }
    static T& getSingleton()
    {
        assert(ms_Singleton);
        return *ms_Singleton;
    }
    static T* getSingletonPtr()
    {
        return ms_Singleton;
    }
    virtual void releaseSingleton()
    {
    }
};

#define DEF_SINGLETON(c) \
    template<> c *Singleton<c>::ms_Singleton = 0

inline float sign(float x)
{
    return (x < 0.0f)? -1.0f : 1.0f;
}

inline float frand(float x = 1.0f)
{
    return (rand() / (float) RAND_MAX) * x;
}

inline float Pi()
{
    static const float pi = atan(1.0f) * 4.0f;

    return pi;
}

#endif//UTILS_H
