#pragma once
#include <boost/lockfree/queue.hpp>
#include "Session.h"

template<class T>
class LockFreeQueueFixedSize : public boost::lockfree::queue<T,boost::lockfree::fixed_sized<true>>
{
public:
    LockFreeQueueFixedSize(size_t queueSize = defaultQueueSize) :
        boost::lockfree::queue<T, boost::lockfree::fixed_sized<true>> (defaultQueueSize),
        m_size(0)
    {}

private:
    static const size_t defaultQueueSize = 50000;
    std::atomic<size_t> m_size;
};
