//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: concrt_extras.h
//
//  Implementation of ConcRT helpers
//
//--------------------------------------------------------------------------

#pragma once

#include <concrtrm.h>
#include <concrt.h>

namespace Concurrency
{
    namespace samples
    {
        template<typename T>
        class concrt_suballocator {
        public:
            typedef T* pointer;
            typedef const T* const_pointer;
            typedef T& reference;
            typedef const T& const_reference;
            typedef T value_type;
            typedef size_t size_type;
            typedef ptrdiff_t difference_type;
            template<typename U> struct rebind {
                typedef concrt_suballocator<U> other;
            };

            concrt_suballocator() throw() {}
            concrt_suballocator( const concrt_suballocator& ) throw() {}
            template<typename U> concrt_suballocator(const concrt_suballocator<U>&) throw() {}

            pointer address(reference x) const {return &x;}
            const_pointer address(const_reference x) const {return &x;}

            pointer allocate( size_type n, const void* hint=0 ) {
                // The "hint" argument is ignored 
                return pointer(Concurrency::Alloc(n * sizeof(T)));
            }

            void deallocate( pointer p, size_type ) {
                Concurrency::Free(p);
            }

            size_type max_size() const throw() {
                size_type count = (size_type)(-1) / sizeof (T);
                return (0 < count ? count : 1);
            }

            void construct( pointer p, const T& value ) {new(static_cast<void*>(p)) T(value);}

            void destroy( pointer p ) {p->~T();}
        };
        namespace details
        {
            //a templated static method for scheduling a functor
            //assumes that the functor is heap allocated and should be deleted
            //after execution
            template<class Func>
            static void _Task_proc(void* data)
            {
                Func* pFunc = (Func*) data;
                (*pFunc)();
                delete pFunc;
            }
            template <class SchedulerObject, class Func>
            void _Schedule_task(SchedulerObject* pScheduler,const Func& fn)
            {
                Func* pFn = new Func(fn);
                pScheduler->ScheduleTask(details::_Task_proc<Func>,(void*)pFn);
            }
            template <class SchedulerObject>
            Concurrency::ScheduleGroup* _Create_Schedule_Group(SchedulerObject* sched)
            {
                return sched->CreateScheduleGroup();
            }
        }
        typedef Concurrency::SchedulerPolicy scheduler_policy;
        typedef Concurrency::TaskProc task_proc;
        /// <summary>
        /// A wrapper around Concurrency::Scheduler that offers 
        /// support for scheduling a task with a functor and release semantics
        /// </summary>
        typedef struct task_scheduler
        {
        private:
            //disable copy constructor
            task_scheduler(const task_scheduler&);
            //a pointer to a scheduler instance
            Concurrency::Scheduler* pScheduler;
            //disable assignment
            task_scheduler const & operator=(task_scheduler const&);
        public:
            typedef Concurrency::Scheduler* native_handle_type;
            const native_handle_type native_handle;
            /// <summary>
            /// Schedules a lightweight task on this scheduler.
            /// </summary>
            /// <param name="fn">A functor which is the task to run.</param>
            template <class Func>
            void schedule_task(const Func& fn)
            {
                details::_Schedule_task(pScheduler,fn);
            }
            /// <summary>
            /// Schedules a lightweight task on this scheduler.
            /// </summary>
            /// <param name="proc">A pointer to the task method.</param>
            /// <param name="data">Data that is passed into the task.</param>
            void schedule_task(task_proc proc,void* data)
            {
                pScheduler->ScheduleTask(proc,data);
            }
            /// <summary>
            /// Constructs a new task_scheduler.
            /// </summary>
            /// <param name="policy">Indicates the scheduler policy.</param>
            task_scheduler(scheduler_policy policy = scheduler_policy(0)):pScheduler(Concurrency::Scheduler::Create(policy)),native_handle(pScheduler)
            {
            }
            operator native_handle_type()
            {
                return pScheduler;
            }
            ~task_scheduler()
            {
                //release the scheduler
                pScheduler->Release();
            }
        } task_scheduler;
        /// <summary>
        /// A wrapper around Concurrency::ScheduleGroup that offers 
        /// support for scheduling a task with a functor
        /// </summary>
        typedef struct schedule_group
        {
        private:
            Concurrency::ScheduleGroup* pScheduleGroup;
            //disable copy constructor
            schedule_group(const schedule_group&);
            //disable assignment
            schedule_group const & operator=(schedule_group const&);
        public:
            typedef Concurrency::ScheduleGroup* native_handle_type;
            const native_handle_type native_handle;

            schedule_group():pScheduleGroup(details::_Create_Schedule_Group(Concurrency::CurrentScheduler::Get())),native_handle(pScheduleGroup){}
            schedule_group(const task_scheduler& scheduler):pScheduleGroup(details::_Create_Schedule_Group(scheduler.native_handle)),native_handle(pScheduleGroup)
            {
            };
            /// <summary>
            /// Schedules a lightweight task on this schedule group.
            /// </summary>
            /// <param name="fn">A functor which is the task to run.</param>
            template <class Func>
            void schedule_task(Func&& fn)
            {
                details::_Schedule_task(pScheduleGroup,fn);
            }
            /// <summary>
            /// Schedules a lightweight task on this schedule group.
            /// </summary>
            /// <param name="proc">A pointer to the task method.</param>
            /// <param name="data">Data that is passed into the task.</param>
            void schedule_task(task_proc proc,void* data)
            {
                pScheduleGroup->ScheduleTask(proc,data);
            }
        } schedule_group;
        template <class Func>
        void schedule_task(Func&& fn)
        {
            Concurrency::Scheduler* pScheduler = Concurrency::CurrentScheduler::Get();
            details::_Schedule_task(pScheduler,fn);
        }
        template <class SchedulingType, class Func>
        void schedule_task(SchedulingType* pScheduler, Func&& fn)
        {
            details::_Schedule_task(pScheduler,fn);
        }
    }
    /// <summary>
    /// An RAII style wrapper around Concurrency::Context::Oversubscribe,
    /// useful for annotating known blocking calls
    /// </summary>
    class scoped_oversubcription_token
    {
    public:
        scoped_oversubcription_token()
        {
            Concurrency::Context::CurrentContext()->Oversubscribe(true);
        }
        ~scoped_oversubcription_token()
        {
            Concurrency::Context::CurrentContext()->Oversubscribe(false);
        }
    };
}