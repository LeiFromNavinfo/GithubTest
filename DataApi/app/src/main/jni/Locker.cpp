#include "Locker.h"

boost::mutex Locker::m_lock;

void Locker::Lock()
{
    m_lock.lock();
}

void Locker::UnLock()
{
    m_lock.unlock();
}