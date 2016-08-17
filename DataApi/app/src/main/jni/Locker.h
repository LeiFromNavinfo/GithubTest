#ifndef EDITOT_LOCK_H
#define EDITOT_LOCK_H

#include "Editor.h"

class Locker
{
public:
    static boost::mutex m_lock;

public:

    static void Lock();

    static void UnLock();
};

#endif
