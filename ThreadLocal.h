#ifndef THREADLOCAL_H
#define THREADLOCAL_H

#include "Mutex.h"

#include <boost/noncopyable.hpp>
#include <pthread.h>

template<typename T>
class ThreadLocal : boost::noncopyable
{
public:
    ThreadLocal()
    {
        MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
    }

    ~ThreadLocal()
    {
        MCHECK(pthread_key_delete(pkey_t));
    }

    T& value()
    {
        T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));
        if (!perThreadValue)
        {
            T* newObj = new T();
            MECHECK(pthread_setspecific(pkey_, newObj));
            perThreadValue = newObj;
        }
        return *perThreadValue;
    }

private:

    static void destructor(void *x)
    {
        T* obj = static_cast<T*>(x);
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void) dummy;
        delete obj;
    }

    pthread_key_t pkey_;
};

#endif
