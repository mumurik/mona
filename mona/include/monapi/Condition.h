#ifndef _MONAPI_CONDITION_
#define _MONAPI_CONDITION_

#include <sys/types.h>
#include <sys/error.h>
#include <monapi/syscall.h>
#include <monapi/Assert.h>

namespace MonAPI {

/*
    class: Condition

*/
class Condition
{
public:
    /*
       function: Condition

       Creates and returns a new conditional variable.

       Returns:

         A condition variable.
    */
    Condition()
    {
        intptr_t ret = syscall_condition_create(&condition_);
        ASSERT(M_OK == ret);
    }


    /*
       function: ~Condition

       Destroy the condition.
    */
    ~Condition()
    {
        intptr_t ret = syscall_condition_destroy(&condition_);
        ASSERT(M_OK == ret);
    }


    /*
       function: waitWithTimeout

       Waits on the condition until ublocked by a notify or timed out.

       Parameters:

         mutex - Mutex
         timeoutMsec - timeout in msec.

       Returns:

         Returns <M_OK> if the waiting thread is successfully unblocked. <M_TIMED_OUT>, when timeout. <M_BAD_CONDITION_ID> if condition is invalid or <M_BAD_MUTEX_ID> if mutex is invalid.

    */
    intptr_t waitWithTimeout(Mutex* mutex, intptr_t timeoutMsec)
    {
        mutex_t m = mutex->getMutex_t();
        return syscall_condition_wait_timeout(&condition_, &m, timeoutMsec);
    }


    /*
       function: wait

       Waits on the condition.

       Parameters:

         mutex - Mutex

       Returns:
         Returns <M_OK> if the waiting thread is successfully unblocked, <M_BAD_CONDITION_ID> if condition is invalid or <M_BAD_MUTEX_ID> if mutex is invalid.

    */
    intptr_t wait(Mutex* mutex)
    {
        mutex_t m = mutex->getMutex_t();
        return syscall_condition_wait(&condition_, &m);
    }


    /*
       function: notifyAll

       Unblocks all the threads waiting on the condition.


       Returns:
         Returns <M_OK> if all the theads are successfully unblocked, or <M_BAD_CONDITION_ID> if condition is invalid.

    */
    intptr_t notifyAll()
    {
        return syscall_condition_notify_all(&condition_);
    }


    /*
       function: getCond_t

       Returns:
         cond_t, which can be used with syscall_condition functions.
    */
    cond_t* getCond_t()
    {
        return &condition_;
    }

private:
    cond_t condition_;
};

};

#endif
