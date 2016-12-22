//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: concrt_extras.h
//
//  Implementation of a cooperative semaphore
//
//--------------------------------------------------------------------------

#pragma once

#include <concrt.h>

namespace Concurrency
{
namespace samples
{

    // Implementation of a cooperative semaphore
    class semaphore
    {
    public:
        semaphore(int initialCount, int maximumCount)
            : m_semVal(initialCount),
              m_maximumCount(maximumCount)
        {
            _ASSERTE(m_maximumCount >= 0);
            _ASSERTE(m_semVal <= m_maximumCount);
        }

        // P(). Attempt to decrement the semaphore value. If the value
        // is 0 the routine cooperatively blocks. If the wait times out
        // the return value is "false".
        bool wait(unsigned int timeout = COOPERATIVE_TIMEOUT_INFINITE)
        {
            bool succ = true;
            WaitNode node;

            // Try to decrement
            if (!Decrement(&node))
            {
                // Block
                if (!node.Wait(timeout))
                {
                    // Timeout
                    if (!RemoveWaiter(&node))
                    {
                        // already removed by release.
                        node.Wait();
                    }
                    else
                    {
                        // wait timed out
                        succ = false;
                    }
                }
            }

            return succ;
        }


        // V(). Increment the semaphore value by the specified count
        // and signal the appropriate number of waiters. The routine
        // returns the semahaphore value prior to this increment
        int release(int count = 1)
        {
            // Increment            
            int oldValue = 0;
            WaitNode *pNode = Increment(count, oldValue);

            // Signal waiters
            while (pNode != NULL)
            {
                WaitNode * pNext = pNode->m_pNext;
                pNode->Signal();
                pNode = pNext;
            }

            return oldValue;
        }

    private:

        // Helper class to hold an event and an intrusive next pointer 
        // for the wait queue
        class WaitNode
        {
        public:
            WaitNode()
                : m_pNext(NULL)
            {
            }

            bool Wait(unsigned int timeout = COOPERATIVE_TIMEOUT_INFINITE)
            {
                return (m_event.wait(timeout) != COOPERATIVE_WAIT_TIMEOUT);
            }

            void Signal()
            {
                m_event.set();
            }

        public:

            Concurrency::event m_event;
            WaitNode * m_pNext;
        };

        // Helper queue that holds the list of waiters
        class WaitQueue
        {
        public:
            WaitQueue() 
                : m_pHead(NULL), m_ppTail(&m_pHead) 
            { 
            };
            
            // Insert the given item
            void Enqueue(WaitNode* pNode)
            {
                _ASSERTE(pNode != NULL);

                pNode->m_pNext = NULL;
                *m_ppTail = pNode;
                m_ppTail = &pNode->m_pNext;
            }

            // Remove a single item
            WaitNode * Dequeue()
            {
                int numDequeued = 0;
                return Dequeue(1, numDequeued);
            }

            // Remove the number of specified items
            WaitNode * Dequeue(int numToDequeue, int& numDequeued)
            {
                _ASSERTE(numToDequeue > 0);
                numDequeued = 0;

                WaitNode *pHead = m_pHead;
                while (m_pHead != NULL)
                {
                    WaitNode *pCurr = m_pHead;
                    m_pHead = m_pHead->m_pNext;

                    if (++numDequeued == numToDequeue)
                    {
                        pCurr->m_pNext = NULL;
                        break;
                    }
                }               
                
                if (m_pHead == NULL)
                    m_ppTail = &m_pHead;

                return pHead;
            }

            // Find and remove the given item
            bool Remove(WaitNode * pNode)
            {
                if (m_pHead == pNode)
                {
                    Dequeue();
                    return true;
                }

                WaitNode * prev = NULL;
                WaitNode * curr = m_pHead;

                // Search the list for the given node
                while (curr != NULL)
                {
                    prev = curr;
                    curr = curr->m_pNext;

                    if (curr == pNode)
                    {
                        if (curr->m_pNext == NULL)
                        {
                            _ASSERTE(&curr->m_pNext == m_ppTail);
                            m_ppTail = &prev->m_pNext;
                        }

                        prev->m_pNext = curr->m_pNext;
                        curr->m_pNext = NULL;
                        return true;
                    }
                }

                return false;
            }

            bool Empty() const 
            { 
                return m_pHead == NULL; 
            }

        private:
            WaitNode *m_pHead;
            WaitNode **m_ppTail;
        };

        // Increment the semaphore and signal waiters (if any) atomically
        WaitNode * Increment(int count, int & oldValue)
        {
            Concurrency::critical_section::scoped_lock lock(m_cs);
            
            _ASSERTE(count > 0);
            int numWaiters = 0;           
            WaitNode * pHead = m_waitQueue.Dequeue(count, numWaiters);

            oldValue = m_semVal;
            m_semVal += (count - numWaiters);
            _ASSERTE(m_semVal <= m_maximumCount);

            return pHead;
        }

        // Try to decrement the semaphore. If not successfull, enqueue a wait node atomically
        bool Decrement(WaitNode * pNode)
        {
            Concurrency::critical_section::scoped_lock lock(m_cs);

            if (m_semVal == 0)
            {
                m_waitQueue.Enqueue(pNode);
                return false;
            }
            else
            {
                --m_semVal;
                return true;
            }
        }

        // Timeout
        bool RemoveWaiter(WaitNode * pNode)
        {
            Concurrency::critical_section::scoped_lock lock(m_cs);
            return m_waitQueue.Remove(pNode);
        }

        int m_maximumCount;
        volatile long m_semVal;

        Concurrency::critical_section m_cs;
        WaitQueue m_waitQueue;
    };


} // namespace samples
} // namespace Concurrency