//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: bounded_queue.h
//
//  Bounded blocking queue with timeout
//
//--------------------------------------------------------------------------

#pragma once

#include <concurrent_queue.h>
#include "..\concrtextras\semaphore.h"
#include <queue>

// Bounded abortable blocking queue
template<class T, class QueueType = Concurrency::concurrent_queue<T>>
class bounded_queue
{
public:

    // maxItems indicate the maximum capacity of the queue
    bounded_queue(int maxItems)
        : m_fullSemaphore(maxItems, maxItems), m_emptySemaphore((int)0, maxItems)
    {
    }

    // Enqueue the given item. Block, if the queue is full
    void enqueue(const T& item)
    {
        m_fullSemaphore.wait();
        m_queue.push(item);
        m_emptySemaphore.release();
    }

    // Dequeue an item. Block, if the queue is empty
    T dequeue(unsigned int timeout = COOPERATIVE_TIMEOUT_INFINITE)
    {
        if (!m_emptySemaphore.wait(timeout))
        {
            // Wait timed out
            throw operation_timed_out();
        }
        else
        {
            T item;
            m_queue.try_pop(item);
            m_fullSemaphore.release();

            return item;
        }
    }

    // Try to dequeue an item. Returns false if there are no items to dequeue
    bool try_dequeue(T& item)
    {
        if (!m_emptySemaphore.wait(0))
        {
            // Wait timed out
            return false;
        }
        else
        {
            m_queue.try_pop(item);
            m_fullSemaphore.release();

            return true;
        }        
    }


private:

    QueueType m_queue;
    Concurrency::samples::semaphore m_fullSemaphore;
    Concurrency::samples::semaphore m_emptySemaphore;
};

// Wrapper on std::queue to allow concurrent access
template<class T>
class LockedQueue
{
public:
    LockedQueue()
    {
    }

    void push(const T& item)
    {
        Concurrency::critical_section::scoped_lock lock(m_cs);
        m_queue.push(item);
    }

    bool try_pop(T& item)
    {
        Concurrency::critical_section::scoped_lock lock(m_cs);
        if (!m_queue.empty())
        {
            item = m_queue.front();
            m_queue.pop();
            return true;
        }

        return false;
    }

private:

    std::queue<T> m_queue;
    Concurrency::critical_section m_cs;
};

