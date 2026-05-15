#ifndef __MUTEX_HPP__
#define __MUTEX_HPP__

#include <iostream>
#include <pthread.h>

namespace LockModule
{
    class Mutex
    {
    public:
        Mutex()
        {
            pthread_mutex_init(&_mutex, nullptr);
        }
        void Lock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&_mutex);
        }
        pthread_mutex_t *GetMutex()
        {
            return &_mutex;
        }

    private:
        pthread_mutex_t _mutex;
    };
    
class LockGuard
    {
    public:
        LockGuard(Mutex &m) :_m(m)
        {
            _m.Lock();
        }
        ~LockGuard()
        {
            _m.unlock();
        }
    private:
        Mutex &_m;
    };
}

#endif