/***
* ==++==
*
* Copyright (c) Microsoft Corporation.  All rights reserved.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* ppl_tasks.h
*
* Parallel Patterns Library - First Class Tasks
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#pragma once

#include <ppl.h>
#include <memory>
#include <vector>
#include <tuple>
#include <utility>

#pragma warning(disable:4505)

/// <summary>
///     The <c>Concurrency</c> namespace provides classes and functions that give you access to the Concurrency Runtime,
///     a concurrent programming framework for C++. For more information, see <see cref="Concurrency Runtime"/>.
/// </summary>
/**/
namespace Concurrency
{
namespace samples
{
// The task_status enum simply mirrors the task_group_status enum
typedef task_group_status task_status;

namespace details
{
    class _Unit_type {}; // special helper-type for handling void type in tasks

    struct _Task_impl_base;

    void __cdecl _ScheduleLightWeightTask(TaskProc _Proc, void * _Data)
    {
        CurrentScheduler::ScheduleTask(_Proc,_Data);
    }

    /// <summary>
    ///     The _PPLTaskHandle is the individual work item that the PPL Task is expected to execute. The user function
    ///     executed by the PPL task must be wrapped in one of these handles in order to execute properly on the underlying
    ///     TaskCollection which can self-delete.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the function object that will be invoked to execute the body of the PPL task handle.
    /// </typeparam>
    /**/
    template<typename _Function>
    class _PPLTaskHandle : public ::Concurrency::samples::details::_PPLTaskChore
    {
    public:
        _PPLTaskHandle(const _Function& _Func, _Task_impl_base * _Task_impl) : _M_function(_Func), _Task(_Task_impl)
        {
            m_pFunction = reinterpret_cast <TaskProc> (&::Concurrency::details::_UnrealizedChore::_InvokeBridge<_PPLTaskHandle>);
        }

        ~_PPLTaskHandle()
        {
        }

        void operator()() const
        {
            try 
            {
                _M_function();
            }
            catch (const _Cancel_current_ppl_task &)
            {
                _Task->_Cancel();

                // Rethrow the exception to finish cleanup of the task collection
                // It will be caught and handled by the runtime.
                throw;
            }
        }

    private:

        // The function object invoked to perform the body of the task.
        _Function _M_function;

        // The task that is executing this function
        _Task_impl_base * _Task;

        _PPLTaskHandle const & operator=(_PPLTaskHandle const&);    // no assignment operator
    };

    /// <summary>
    ///     The base implementation of a first-class task. This class contains all the non-type specific
    ///     implementation details of the task.
    /// </summary>
    /**/
    struct _Task_impl_base
    {
        _Task_impl_base() : _M_fCompleted(false), _M_fCancellationRequested(0l)
        {
        }

        virtual ~_Task_impl_base() 
        {
        }

        task_status _Wait()
        {
            _M_Completed.wait();
            if (_M_fCancellationRequested)
            {
                return canceled;
            }
            return completed;
        }

        bool _Cancel()
        {   
            //
            // TODO: any way to get rid of this lock?
            // Need to grab it to ensure that _M_pTaskCollection hasn't self-destructed yet
            //
            critical_section::scoped_lock _LockHolder(_M_ContinuationsCritSec);

            // Completed is a non-cancellable state
            if (_M_fCompleted)
            {
                return false;
            }

            _M_fCancellationRequested = true;

            _M_Completed.set();

            // Mark this task as scheduled (in case anyone is waiting on it);
            _M_Scheduled.set();

            // Cancellation completes the task, so all dependent tasks must be run to cancel them
            // They are canceled when they begin running (see _RunContinuation) and see that their 
            // ancestor has been canceled.
            _RunTaskContinuations(_M_Continuations);

            // Since we have canceled, execute any continue_on_cancel calls that may have been made
            _RunTaskContinuations(_M_CancellationContinuations);

            return true;
        }

        bool _IsCanceled()
        {
            return _M_fCancellationRequested;
        }

        static void _RunTaskContinuations(std::vector<std::pair<TaskProc,void*>>& _Continuations)
        {
            // If there is only one continuation, we could try to execute it synchronously. 
            // However, that could lead to stack overflows for very long task chains
            for (auto it = _Continuations.begin(); it != _Continuations.end(); ++it)
            {
                // When this continuation function runs, it will check to see if its ancestor
                // has canceled or not before scheduling itself for execution
                _ScheduleLightWeightTask(it->first,it->second);
            }
            _Continuations.clear();
        }

        event _M_Completed;
        event _M_Scheduled;
        bool _M_fCompleted;  // this should be used everywhere instead of the event, which is only used in wait method
        bool _M_fCancellationRequested;

        critical_section _M_ContinuationsCritSec;
        std::vector<std::pair<TaskProc,void*>> _M_Continuations;
        std::vector<std::pair<TaskProc,void*>> _M_CancellationContinuations;
    };

    /// <summary>
    ///     The implementation of a first-class task. This structure contains the task group used to execute
    ///     the task function and handles the scheduling.  The _Task_impl is created as a shared_ptr
    ///     member of the the public task class, so its destruction is handled automatically.
    /// </summary>
    /// <typeparam name="_ReturnType">
    ///     The result type of this task.
    /// </typeparam>
    /**/
    template<typename _ReturnType>
    struct _Task_impl : public _Task_impl_base
    {
        _Task_impl()
        {
        }

        virtual ~_Task_impl() 
        {
        }

        void _FinalizeAndRunContinuations(_ReturnType _Result)
        {
            _M_Result = _Result;

            // Hold this lock to ensure continuations being concurrently either get added
            // to the _M_Continuations vector or wait for the result
            critical_section::scoped_lock _LockHolder(_M_ContinuationsCritSec);
            _M_Completed.set();

            // Set the flag to mark this task as done
            _M_fCompleted = true;

            _RunTaskContinuations(_M_Continuations);

            // Clear out any cancellation continuations that may have been added
            _M_CancellationContinuations.clear();
        }

        _ReturnType _M_Result;        // this means that the result type must have a public default ctor.
    };

    template<typename _ReturnType>
    struct _Task_ptr
    {
        typedef std::shared_ptr<_Task_impl<_ReturnType>> _Type;
        static _Type make() { return std::make_shared<_Task_impl<_ReturnType>>(); }
    };

    template<typename _ReturnType>
    struct _TaskExecutionParameter
    {
        typename _Task_ptr<_ReturnType>::_Type _Task;
        std::tr1::function<_ReturnType(void)> _Func;
    };

    template<typename _ReturnType, typename _NewReturnType>
    struct _TaskContinuationParameter
    {
        typename _Task_ptr<_ReturnType>::_Type _Ancestor;
        typename _Task_ptr<_NewReturnType>::_Type _Continuation;
        typedef std::tr1::function<_NewReturnType(_ReturnType)> FuncType;
        FuncType _Func;
    };

    template<typename _ResultType>
    struct _Task_completion_event_impl
    {
        // We need to protect the loop over the array, so concurrent_vector would not have helped
        std::vector<typename _Task_ptr<_ResultType>::_Type> _Tasks;
        critical_section _TaskListCritSec;
        _ResultType _Value;
        bool _HasValue;
        bool _IsCanceled;
    };
} // details

/// <summary>
///     The task completion event class, allows users to delay the execution of a task until a condition is
///     satisfied, or start a task in response to an external event.
/// </summary>
/// <typeparam name="_ResultType">
///     The result type of this task_completion_event.
/// </typeparam>
/**/
template<typename _ResultType>
class task_completion_event 
{
    std::shared_ptr<::Concurrency::samples::details::_Task_completion_event_impl<_ResultType>> _M_Impl;
public:
    template <typename T> friend class task; // task can register itself with the event by calling the private _RegisterTask
    template <typename T> friend class task_completion_event;

    /// <summary>
    ///     The task completion event constructor.
    /// </summary>
    /**/
    task_completion_event() : _M_Impl(std::make_shared<::Concurrency::samples::details::_Task_completion_event_impl<_ResultType>>()) 
    {
        _M_Impl->_HasValue = false;
        _M_Impl->_IsCanceled = false;
    }

    /// <summary>
    ///     The set the task completion event. 
    /// </summary>
    /// <param>
    ///     The result to set this event with.
    /// </param>
    /// <returns>
    ///     If this call to set actually does set the event, <c>true</c> is returned. If the event has already been set, 
    ///     <c>false</c> is returned
    /// </returns>
    /**/
    bool set(_ResultType _Result) const // 'const' (even though it's not deep) allows to safely pass events by value into lambdas
    {
        // subsequent sets are ignored. This makes races to set benign: the first setter wins and all others are ignored.
        if (_M_Impl->_HasValue == true && _M_Impl->_IsCanceled == false)
        {
            return false;
        }

        critical_section::scoped_lock _LockHolder(_M_Impl->_TaskListCritSec);

        if (_M_Impl->_HasValue == false && _M_Impl->_IsCanceled == false)
        {
            _M_Impl->_Value = _Result;
            _M_Impl->_HasValue = true;
            for( auto _Task = _M_Impl->_Tasks.begin(); _Task != _M_Impl->_Tasks.end(); ++_Task )
            {
                (*_Task)->_FinalizeAndRunContinuations(_M_Impl->_Value);
            }
            _M_Impl->_Tasks.clear();
        }
        else
        {
            return false;
        }

        return true;
    }

    /// <summary>
    ///     Cancel the task_completion_event. Any task created using this event will be marked as canceled if it has
    ///     not already been set.
    /// </summary>
    /**/
    void _Cancel() const
    {
        if (_M_Impl->_HasValue == true && _M_Impl->_IsCanceled == false)
        {
            return;
        }

        critical_section::scoped_lock _LockHolder(_M_Impl->_TaskListCritSec);

        if (_M_Impl->_HasValue == false && _M_Impl->_IsCanceled == false)
        {
            _M_Impl->_IsCanceled = true;
            for( auto _Task = _M_Impl->_Tasks.begin(); _Task != _M_Impl->_Tasks.end(); ++_Task )
            {
                (*_Task)->_Cancel();
            }
            _M_Impl->_Tasks.clear();
        }
    }

private:

    /// <summary>
    ///     Register a task with this event. This function is called when a task is constructed using
    ///     a task_completion_event.
    /// </summary>
    /**/
    void _RegisterTask(typename ::Concurrency::samples::details::_Task_ptr<_ResultType>::_Type _TaskParam)
    {
        _TaskParam->_M_Scheduled.set();

        critical_section::scoped_lock _LockHolder(_M_Impl->_TaskListCritSec);
        if (_M_Impl->_HasValue)
        {
            _TaskParam->_FinalizeAndRunContinuations(_M_Impl->_Value);
        }
        else
        {
            _M_Impl->_Tasks.push_back(_TaskParam);
        }
    }
};

/// <summary>
///     The task completion event class, allows users to delay the execution of a task until a condition is
///     satisfied, or start a task in response to an external event.
///     Template specialization for void.
/// </summary>
/**/
template<>
class task_completion_event<void>
{
    task_completion_event<::Concurrency::samples::details::_Unit_type> _UnitEvent;

public:
    template <typename T> friend class task; // task can register itself with the event by calling the private _RegisterTask

    /// <summary>
    ///     The set the task completion event. 
    /// </summary>
    /// <returns>
    ///     If this call to set actually does set the event, <c>true</c> is returned. If the event has already been set, 
    ///     <c>false</c> is returned
    /// </returns>
    /**/
    bool set() const // 'const' (even though it's not deep) allows to safely pass events by value into lambdas
    {
        return _UnitEvent.set(::Concurrency::samples::details::_Unit_type());
    }

    /// <summary>
    ///     Cancel the task_completion_event. Any task created using this event will be marked as canceled if it has
    ///     not already been set.
    /// </summary>
    /**/
    void _Cancel() const // 'const' (even though it's not deep) allows to safely pass events by value into lambdas
    {
        _UnitEvent._Cancel();
    }

private:

    /// <summary>
    ///     Register a task with this event. This function is called when a task is constructed using
    ///     a task_completion_event.
    /// </summary>
    /**/
    void _RegisterTask(::Concurrency::samples::details::_Task_ptr<::Concurrency::samples::details::_Unit_type>::_Type _TaskParam)
    {
        _UnitEvent._RegisterTask(_TaskParam);
    }
};

/// <summary>
///     The PPL task class.
/// </summary>
/// <typeparam name="_ReturnType">
///     The result type of this task.
/// </typeparam>
/**/
template<typename _ReturnType>
class task
{
public:
    typedef _ReturnType _TaskType;

    /// <summary>
    ///     Constructor for a PPL task.
    /// </summary>
    /// <param name="_FCreateImpl">
    ///     If <c>true</c> the task should create an underlying implementation.
    /// </param>
    /// <param name="_Func">
    ///     The function for this PPL task to execute.
    /// </param>
    /// <param name="_Event">
    ///     A task_completion_event with which to create this task. The task will be set as completed
    ///     when the task_completion_event is set.
    /// </param>
    /// <remarks>
    ///     The default constructor for a task is only present in order to allow tasks to be used within containers.
    ///     Users should never create a default constructed task because it is not usable: you cannot run anything,
    ///     continue any work, or wait on it.
    /// </remarks>
    /**/
    task(bool _FCreateImpl = false) : _M_Impl(NULL) 
    {
        // The default constructor should create a task with a NULL impl. This is a signal that the
        // task is not usable and should throw if any wait, get or continue_with APIs are used.
        // In one instance (continue_with) we need to default construct a task with an Impl, since
        // that is the continuation task being returned.
        if (_FCreateImpl)
        {
            _M_Impl = ::Concurrency::samples::details::_Task_ptr<_ReturnType>::make();
        }
    }

    /// <summary>
    ///     Return the underlying implementation for this task.
    /// </summary>
    /// <returns>
    ///     The underlying implementation for the task.
    /// </returns>
    /**/
    typename ::Concurrency::samples::details::_Task_ptr<_ReturnType>::_Type _GetImpl() 
    { 
        return _M_Impl; 
    }

    template <typename T> friend class task;

private:

    // The underlying implementation for this task
    typename ::Concurrency::samples::details::_Task_ptr<_ReturnType>::_Type _M_Impl;

    /// <summary>
    ///     Static function that executes a continuation function. This function is recorded by a parent task implementation
    ///     when a continuation is created in order to execute later.
    /// </summary>
    /// <typeparam name="_NewReturnType">
    ///     The return type of the continuation.
    /// </typeparam>
    /// <param name="_PData">
    ///     A void* pointing to a _TaskContinationParameter, which contains pointers to this (parent) task implementation
    ///     as well as the the continuation task implementation.
    /// </param>
    /**/
    template <typename _NewReturnType>
    static void __cdecl _RunContinuation(void *_PData)
    {
        auto _PParam = (::Concurrency::samples::details::_TaskContinuationParameter<_ReturnType,_NewReturnType>*)_PData;
        if (_PParam->_Ancestor->_IsCanceled())
        {
            // If the ancestor was canceled, then your own execution should be canceled.
            // This traverses down the tree to cancel it.
            _PParam->_Continuation->_Cancel();
            // Delete the parameter to free up the reference for this task and the ancestor
            delete _PParam;
        }
        else
        {
            // This can only run when the ancestor has completed
            _ASSERTE(_PParam->_Ancestor->_M_fCompleted == true);
            _PParam->_Continuation->_FinalizeAndRunContinuations(_PParam->_Func(_PParam->_Ancestor->_M_Result));
        }
    }

    /// <summary>
    ///     Static function that executes a continuation function upon cancellation. This function is recorded by a 
    ///     parent task implementation when a continuation is created in order to execute later.
    /// </summary>
    /// <param name="_PData">
    ///     A void* pointing to a _TaskExecutionParameter, which contains a pointer to this task implementation.
    /// </param>
    /**/
    template <typename _ReturnType>
    static void __cdecl _RunCancellationContinuation(void *_PData)
    {
        auto _PParam = (::Concurrency::samples::details::_TaskExecutionParameter<_ReturnType>*)_PData;
        _PParam->_Task->_FinalizeAndRunContinuations(_PParam->_Func());
        delete _PParam;
    }

    /// <summary>
    ///     Static function that executes a function. Run by a light-weight task.
    /// </summary>
    /// <param name="_PData">
    ///     A void* pointing to a _TaskExecutionParameter, which contains a pointer to this task implementation.
    /// </param>
    /**/
    static void __cdecl _RunTask(void *_PData)
    {
        auto _PParam = (::Concurrency::samples::details::_TaskExecutionParameter<_ReturnType>*)_PData;
        _PParam->_Task->_FinalizeAndRunContinuations(_PParam->_Func());
        delete _PParam;
    }

public:

    /// <summary>
    ///     Constructor for a PPL task.
    /// </summary>
    /// <param name="_FCreateImpl">
    ///     If <c>true</c> the task should create an underlying implementation.
    /// </param>
    /// <param name="_Func">
    ///     The function for this PPL task to execute.
    /// </param>
    /// <param name="_Event">
    ///     A task_completion_event with which to create this task. The task will be set as completed
    ///     when the task_completion_event is set.
    /// </param>
    /// <remarks>
    ///     The default constructor for a task is only present in order to allow tasks to be used within containers.
    ///     Users should never create a default constructed task because it is not usable: you cannot run anything,
    ///     continue any work, or wait on it.
    /// </remarks>
    /**/
    task(std::tr1::function<_ReturnType()> _Func) : _M_Impl(::Concurrency::samples::details::_Task_ptr<_ReturnType>::make()) 
    {
        // Since this task is not a continuation (i.e. does not have an ancestor), simply schedule it for execution
        auto _PParam = new ::Concurrency::samples::details::_TaskExecutionParameter<_ReturnType>();
        _PParam->_Func = _Func;
        _PParam->_Task = _M_Impl;
        _ScheduleLightWeightTask(_RunTask,_PParam);
    }

    /// <summary>
    ///     Constructor for a PPL task.
    /// </summary>
    /// <param name="_FCreateImpl">
    ///     If <c>true</c> the task should create an underlying implementation.
    /// </param>
    /// <param name="_Func">
    ///     The function for this PPL task to execute.
    /// </param>
    /// <param name="_Event">
    ///     A task_completion_event with which to create this task. The task will be set as completed
    ///     when the task_completion_event is set.
    /// </param>
    /// <remarks>
    ///     The default constructor for a task is only present in order to allow tasks to be used within containers.
    ///     Users should never create a default constructed task because it is not usable: you cannot run anything,
    ///     continue any work, or wait on it.
    /// </remarks>
    /**/
    task(task_completion_event<_ReturnType> _Event) : _M_Impl(::Concurrency::samples::details::_Task_ptr<_ReturnType>::make())
    {
        _Event._RegisterTask(_M_Impl);
    }

    /// <summary>
    ///     Destructor for a PPL task.
    /// </summary>
    /**/
    ~task()
    {
    }

    /// <summary>
    ///     A utility function for executing task functions which helps to manage different types.
    /// </summary>
    /// <typeparam name="_InpType">
    ///     The input type of the function.
    /// </typeparam>
    /// <typeparam name="_OutType">
    ///     The output type of the function.
    /// </typeparam>
    /**/
    template<typename _InpType, typename _OutType>
    class _Functor_transformer
    {
    public:
        static typename ::Concurrency::samples::details::_TaskContinuationParameter<_InpType,_OutType>::FuncType _Perform(std::tr1::function<_OutType(_InpType)> _Func)
        {
            return _Func;
        }
    };

    /// <summary>
    ///     A utility function for executing task functions which helps to manage different types.
    /// </summary>
    /// <typeparam name="_InpType">
    ///     The input type of the function.
    /// </typeparam>
    /**/
    template<typename _InpType>
    class _Functor_transformer<_InpType, void>
    {
    public:
        typedef void _OutType;
        static typename ::Concurrency::samples::details::_TaskContinuationParameter<_InpType,_OutType>::FuncType _Perform(std::tr1::function<_OutType(_InpType)> _Func)
        {
            return ::Concurrency::samples::details::_MakeTToUnitFunc(_Func);
        }
    };

    /// <summary>
    ///     Schedule the actual continuation. This will either schedule the function on the continuation task's implementation
    ///     if the task has completed or append it to a list of functions to execute when the task actually does complete.
    /// </summary>
    /// <typeparam name="_FuncInputType">
    ///     The input type of the task.
    /// </typeparam>
    /// <typeparam name="_FuncOutputType">
    ///     The output type of the task.
    /// </typeparam>
    /**/
    template<typename _FuncInputType, typename _FuncOutputType>
    void _ScheduleContinuation(::Concurrency::samples::details::_TaskContinuationParameter<_FuncInputType,_FuncOutputType> *_PParam)
    {
        // If the task has canceled, cancel the continuation. If the task has completed, execute the continuation right away. 
        // Otherwise, add it to the list of pending continuations
        critical_section::scoped_lock _LockHolder(_M_Impl->_M_ContinuationsCritSec);
        if (_M_Impl->_M_fCompleted == true)
        {
            if (_M_Impl->_M_fCancellationRequested == false) 
            {
                _ScheduleLightWeightTask(_RunContinuation<_FuncOutputType>,_PParam);
            }
            else
            {
                // If the ancestor was canceled, then your own execution should be canceled.
                // This traverses down the tree to cancel it.
                _PParam->_Continuation->_Cancel();
                // Delete the parameter to free up the reference for this task and the ancestor
                delete _PParam;
            }
        }
        else
        {
            _M_Impl->_M_Continuations.push_back(std::pair<TaskProc,void*>(_RunContinuation<_FuncOutputType>,_PParam));
        }
    }

    /// <summary>
    ///     Add a continuation task to this task. The continuation will execute when this task completes, and its function
    ///     will be presented the output of this task's function.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the function object that will be invoked by this task.
    /// </typeparam>
    /// <param name="_Func">
    ///     The continuation function to execute when this task completes. This continuation function must take as input the 
    ///     output of this parent task that it is continuing from.
    /// </param>
    /// <returns>
    ///     A new task which will be scheduled for execution when this current task completes. The new task's type will
    ///     be the output of the function <c>_Func</c>
    /// </returns>
    /**/
    template<typename _Function>
    auto continue_with(const _Function& _Func) -> task<decltype (_Func(_ReturnType()))>
    {
        if (_M_Impl == NULL)
        {
            throw invalid_operation("continue_with() cannot be called on a default constructed task.");
        }

        typedef decltype (_Func(_ReturnType())) _FuncOutputType;
        typedef _ReturnType _FuncInputType;

        // Create the continuation task
        task<_FuncOutputType> _ContinuationTask(true);
        auto _PParam = new ::Concurrency::samples::details::_TaskContinuationParameter<_FuncInputType,_FuncOutputType>();
        _PParam->_Func = _Functor_transformer<_FuncInputType,_FuncOutputType>::_Perform(_Func);
        _PParam->_Ancestor = _M_Impl;
        _PParam->_Continuation = _ContinuationTask._GetImpl();

        // Schedule the continuation task
        _ScheduleContinuation(_PParam);
        return _ContinuationTask;
    }

    /// <summary>
    ///     Schedule a cancellation continuation. If the task is cancelled, this task will execute.
    /// </summary>
    /// <typeparam name="_FuncOutputType">
    ///     The output type of the task.
    /// </typeparam>
    /**/
    template<typename _FuncOutputType>
    void _ScheduleCancellationContinuation(::Concurrency::samples::details::_TaskExecutionParameter<_FuncOutputType> *_PParam)
    {
        // If the task has canceled, execute the continuation right away. Otherwise, add it to the list of pending cancellation continuations
        critical_section::scoped_lock _LockHolder(_M_Impl->_M_ContinuationsCritSec);
        if (_M_Impl->_M_fCancellationRequested == true)
        {
                _ScheduleLightWeightTask(_RunCancellationContinuation<_FuncOutputType>,_PParam);
        }
        else
        {
            if (_M_Impl->_M_fCompleted == false)
            {
                _M_Impl->_M_CancellationContinuations.push_back(std::pair<TaskProc,void*>(_RunCancellationContinuation<_FuncOutputType>,_PParam));
            }
            else
            {
                delete _PParam;
            }
        }
    }

    /// <summary>
    ///     Add a cancellation continuation task to this task. The continuation will execute when this task cancels.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the function object that will be invoked by this task upon cancellation.
    /// </typeparam>
    /// <param name="_Func">
    ///     The continuation function to execute when this task cancels.
    /// </param>
    /// <returns>
    ///     A new task which will be scheduled for execution when this current task cancels.
    /// </returns>
    /**/
    template<typename _Function>
    auto _Continue_on_cancel(const _Function& _Func) -> task<decltype (_Func())>
    {
        // Create the continuation task
        task<void> _ContinuationTask(true);

        auto _PParam = new ::Concurrency::samples::details::_TaskExecutionParameter<::Concurrency::samples::details::_Unit_type>();
        _PParam->_Func = ::Concurrency::samples::details::_MakeVoidToUnitFunc(_Func);
        _PParam->_Task = _ContinuationTask._GetImpl();

        // Schedule the continuation task
        _ScheduleCancellationContinuation(_PParam);
        return _ContinuationTask;
    }

    /// <summary>
    ///     Wait on the task to complete. If the task is still in the process of executing, a
    ///     call to wait can potentially inline work.
    /// </summary>
    /// <returns>
    ///     A <c>task_status</c> which can be either <c>completed</c> or <c>canceled</c>.
    /// </returns>
    /**/
    task_status wait()
    {
        if (_M_Impl == NULL)
        {
            throw invalid_operation("wait()/get() cannot be called on a default constructed task.");
        }

        return _M_Impl->_Wait();
    }

    /// <summary>
    ///     Return the result of this task. If the task has not yet completed, this will wait on the 
    ///     task to complete. If the task is still in the process of executing, a call to wait can
    ///     potentially inline work.
    /// </summary>
    /// <returns>
    ///     The output of the task.
    /// </returns>
    /// <remarks>
    ///     If the task is canceled, a call to get will throw an <c>invalid_operation</c> exception.
    /// </remarks>
    /**/
    _ReturnType get()
    {
        if (this->wait() == canceled)
        {
            throw invalid_operation("get() was called on a canceled task.");
        }

        return _M_Impl->_M_Result;
    }

    /// <summary>
    ///     Cancel the work being done by this task.
    /// </summary>
    /// <returns>
    ///     If the cancel was successful <c>true</c> is returned. If the task has already completed
    ///     or canceled, <c>false</c> is returned.
    /// </returns>
    /**/
    bool cancel()
    {
        if (_M_Impl == NULL)
        {
            throw invalid_operation("cancel() cannot be called on a default constructed task.");
        }

        return _M_Impl->_Cancel();
    }

    /// <summary>
    ///     Checks whether the task has been canceled
    /// </summary>
    /// <returns>
    ///     If the task is canceled, <c>true</c> is returned, otherwise, <c>false</c>.
    /// </returns>
    /**/
    bool is_canceled()
    {
        if (_M_Impl == NULL)
        {
            throw invalid_operation("is_canceled() cannot be called on a default constructed task.");
        }

        return _M_Impl->_IsCanceled();
    }

    /// <summary>
    ///     Compare two tasks.
    /// </summary>
    /// <returns>
    ///     If the tasks point to the same underlying implementation, <c>true</c> is returned, otherwise, <c>false</c>.
    /// </returns>
    /**/
    bool operator==(const task<_ReturnType>& _Rhs) const
    {
        return (_M_Impl == _Rhs._M_Impl);
    }

    /// <summary>
    ///     Compare two tasks
    /// </summary>
    /// <returns>
    ///     If the tasks point to the same underlying implementation, <c>false</c> is returned, otherwise, <c>true</c>.
    /// </returns>
    /**/
    bool operator!=(const task<_ReturnType>& _Rhs)
    {
        return !operator==(_Rhs);
    }

};

namespace details
{
    template<typename _NewReturnType>
    struct _TaskContinuationParameter<void,_NewReturnType>
    {
        typename ::Concurrency::samples::details::_Task_ptr<_Unit_type>::_Type _Ancestor;
        typename ::Concurrency::samples::details::_Task_ptr<_NewReturnType>::_Type _Continuation;
        typedef std::tr1::function<_NewReturnType(_Unit_type)> FuncType;
        FuncType _Func;
    };

    template<typename _ReturnType>
    struct _TaskContinuationParameter<_ReturnType,void>
    {
        typename ::Concurrency::samples::details::_Task_ptr<_ReturnType>::_Type _Ancestor;
        typename ::Concurrency::samples::details::_Task_ptr<_Unit_type>::_Type _Continuation;
        typedef std::tr1::function<_Unit_type(_ReturnType)> FuncType;
        FuncType _Func;
    };

    template<>
    struct _TaskContinuationParameter<void,void>
    {
        ::Concurrency::samples::details::_Task_ptr<_Unit_type>::_Type _Ancestor;
        ::Concurrency::samples::details::_Task_ptr<_Unit_type>::_Type _Continuation;
        typedef std::tr1::function<_Unit_type(_Unit_type)> FuncType;
        FuncType _Func;
    };

    // Utility method for dealing with void functions
    static std::tr1::function<_Unit_type(void)> _MakeVoidToUnitFunc(const std::tr1::function<void(void)>& _Func)
    {
        return [=]() -> _Unit_type { _Func(); return _Unit_type(); };
    }

    template <typename _Type>
    std::tr1::function<_Type(_Unit_type)> _MakeUnitToTFunc(const std::tr1::function<_Type(void)>& _Func)
    {
        return [=](_Unit_type) -> _Type { return _Func(); };
    }

    template <typename _Type>
    std::tr1::function<_Unit_type(_Type)> _MakeTToUnitFunc(const std::tr1::function<void(_Type)>& _Func)
    {
        return [=](_Type t) -> _Unit_type { _Func(t); return _Unit_type(); };
    }

    static std::tr1::function<_Unit_type(_Unit_type)> _MakeUnitToUnitFunc(const std::tr1::function<void(void)>& _Func)
    {
        return [=](_Unit_type) -> _Unit_type { _Func(); return _Unit_type(); };
    }
}
/// <summary>
///     The PPL task class. Explicit specialization for void.
/// </summary>
/**/
template<>
class task<void>
{
    // This is the task that will execute the void function
    task<::Concurrency::samples::details::_Unit_type> _UnitTask;

    /// <summary>
    ///     Return the underlying implementation for this task.
    /// </summary>
    /// <returns>
    ///     The underlying implementation for the task.
    /// </returns>
    /**/
    ::Concurrency::samples::details::_Task_ptr<::Concurrency::samples::details::_Unit_type>::_Type _GetImpl() 
    { 
        return _UnitTask._M_Impl; 
    }

public:
    typedef void _TaskType;

    friend class task<void>;
    template <typename T> friend class task;
    template <typename T> friend class task_completion_event;

    /// <summary>
    ///     Constructor for a PPL task.
    /// </summary>
    /// <param name="_FCreateImpl">
    ///     If <c>true</c> the task should create an underlying implementation.
    /// </param>
    /// <param name="_Func">
    ///     The function for this PPL task to execute.
    /// </param>
    /// <param name="_Event">
    ///     A task_completion_event with which to create this task. The task will be set as completed
    ///     when the task_completion_event is set.
    /// </param>
    /// <remarks>
    ///     The default constructor for a task is only present in order to allow tasks to be used within containers.
    ///     Users should never create a default constructed task because it is not usable: you cannot run anything,
    ///     continue any work, or wait on it.
    /// </remarks>
    /**/
    task(bool _FCreateImpl = false) : _UnitTask(_FCreateImpl) {}

    /// <summary>
    ///     Constructor for a PPL task.
    /// </summary>
    /// <param name="_FCreateImpl">
    ///     If <c>true</c> the task should create an underlying implementation.
    /// </param>
    /// <param name="_Func">
    ///     The function for this PPL task to execute.
    /// </param>
    /// <param name="_Event">
    ///     A task_completion_event with which to create this task. The task will be set as completed
    ///     when the task_completion_event is set.
    /// </param>
    /// <remarks>
    ///     The default constructor for a task is only present in order to allow tasks to be used within containers.
    ///     Users should never create a default constructed task because it is not usable: you cannot run anything,
    ///     continue any work, or wait on it.
    /// </remarks>
    /**/
    task(std::tr1::function<void(void)> _Func) : _UnitTask(::Concurrency::samples::details::_MakeVoidToUnitFunc(_Func)) {}

    /// <summary>
    ///     Constructor for a PPL task.
    /// </summary>
    /// <param name="_FCreateImpl">
    ///     If <c>true</c> the task should create an underlying implementation.
    /// </param>
    /// <param name="_Func">
    ///     The function for this PPL task to execute.
    /// </param>
    /// <param name="_Event">
    ///     A task_completion_event with which to create this task. The task will be set as completed
    ///     when the task_completion_event is set.
    /// </param>
    /// <remarks>
    ///     The default constructor for a task is only present in order to allow tasks to be used within containers.
    ///     Users should never create a default constructed task because it is not usable: you cannot run anything,
    ///     continue any work, or wait on it.
    /// </remarks>
    /**/
    task(task_completion_event<void> _Event) : _UnitTask(_Event._UnitEvent) { }


    /// <summary>
    ///     A utility function for executing task functions which helps to manage different types.
    /// </summary>
    /// <typeparam name="_OutType">
    ///     The output type of the function.
    /// </typeparam>
    /**/
    template<typename _OutType>
    class _Functor_transformer
    {
    public:
        static typename ::Concurrency::samples::details::_TaskContinuationParameter<void,_OutType>::FuncType _Perform(std::tr1::function<_OutType(void)> _Func)
        {
            return ::Concurrency::samples::details::_MakeUnitToTFunc<_OutType>(_Func);
        }
    };

    /// <summary>
    ///     A utility function for executing task functions which helps to manage different types.
    /// </summary>
    /**/
    template<>
    class _Functor_transformer<void>
    {
    public:
        typedef void _OutType;
        static ::Concurrency::samples::details::_TaskContinuationParameter<void,void>::FuncType _Perform(std::tr1::function<void(void)> _Func)
        {
            return ::Concurrency::samples::details::_MakeUnitToUnitFunc(_Func);
        }
    };

    /// <summary>
    ///     Add a continuation task to this task. The continuation will execute when this task completes, and its function
    ///     will be presented the output of this task's function.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the function object that will be invoked by this task.
    /// </typeparam>
    /// <param name="_Func">
    ///     The continuation function to execute when this task completes. This continuation function must take as input the 
    ///     output of this parent task that it is continuing from.
    /// </param>
    /// <returns>
    ///     A new task which will be scheduled for execution when this current task completes. The new task's type will
    ///     be the output of the function <c>_Func</c>
    /// </returns>
    /**/
    template<typename _Function>
    auto continue_with(const _Function& _Func) -> task<decltype(_Func())>
    {
        if (_UnitTask._M_Impl == NULL)
        {
            throw invalid_operation("continue_with() cannot be called on a default constructed task.");
        }

        typedef decltype (_Func()) _FuncReturnType;

        // Create the continuation task
        task<_FuncReturnType> _ContinuationTask(true);
        auto pParam = new ::Concurrency::samples::details::_TaskContinuationParameter<void,_FuncReturnType>();
        pParam->_Func = _Functor_transformer<_FuncReturnType>::_Perform(_Func);
        pParam->_Ancestor = _UnitTask._M_Impl;
        pParam->_Continuation = _ContinuationTask._GetImpl();

        // Schedule the continuation task
        _UnitTask._ScheduleContinuation(pParam);
        return _ContinuationTask;
    }

    /// <summary>
    ///     Add a cancellation continuation task to this task. The continuation will execute when this task cancels.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the function object that will be invoked by this task upon cancellation.
    /// </typeparam>
    /// <param name="_Func">
    ///     The continuation function to execute when this task cancels.
    /// </param>
    /// <returns>
    ///     A new task which will be scheduled for execution when this current task cancels.
    /// </returns>
    /**/
    template<typename _Function>
    auto _Continue_on_cancel(const _Function& _Func) -> task<decltype (_Func())>
    {
        // Create the continuation task
        task<void> _ContinuationTask(true);

        auto _PParam = new ::Concurrency::samples::details::_TaskExecutionParameter<::Concurrency::samples::details::_Unit_type>();
        _PParam->_Func = ::Concurrency::samples::details::_MakeVoidToUnitFunc(_Func);
        _PParam->_Task = _ContinuationTask._UnitTask._GetImpl();

        // Schedule the continuation task
        _UnitTask._ScheduleCancellationContinuation(_PParam);
        return _ContinuationTask;
    }

    /// <summary>
    ///     Wait on the task to complete. If the task is still in the process of executing, a
    ///     call to wait can potentially inline work.
    /// </summary>
    /// <returns>
    ///     A <c>task_status</c> which can be either <c>completed</c> or <c>canceled</c>.
    /// </returns>
    /**/
    task_status wait() 
    {
        return _UnitTask.wait();
    }

    /// <summary>
    ///     Cancel the work being done by this task.
    /// </summary>
    /// <returns>
    ///     If the cancel was successful <c>true</c> is returned. If the task has already completed
    ///     or canceled, <c>false</c> is returned.
    /// </returns>
    /**/
    bool cancel()
    {
        return _UnitTask.cancel();
    }

    /// <summary>
    ///     Checks whether the task has been canceled
    /// </summary>
    /// <returns>
    ///     If the task is canceled, <c>true</c> is returned, otherwise, <c>false</c>.
    /// </returns>
    /**/
    bool is_canceled()
    {
        return _UnitTask.is_canceled();
    }

    /// <summary>
    ///     Return the result of this task. If the task has not yet completed, this will wait on the 
    ///     task to complete. If the task is still in the process of executing, a call to wait can
    ///     potentially inline work.
    /// </summary>
    /// <remarks>
    ///     If the task is canceled, a call to get will throw an <c>invalid_operation</c> exception.
    /// </remarks>
    /**/
    void get()
    {
        _UnitTask.get();
    }

    /// <summary>
    ///     Compare two tasks.
    /// </summary>
    /// <returns>
    ///     If the tasks point to the same underlying implementation, <c>true</c> is returned, otherwise, <c>false</c>.
    /// </returns>
    /**/
    bool operator==(const task<void>& _Rhs) const
    {
        return (_UnitTask == _Rhs._UnitTask);
    }

    /// <summary>
    ///     Compare two tasks.
    /// </summary>
    /// <returns>
    ///     If the tasks point to the same underlying implementation, <c>false</c> is returned, otherwise, <c>true</c>.
    /// </returns>
    /**/
    bool operator!=(const task<void>& _Rhs)
    {
        return !operator==(_Rhs);
    }
};

/// <summary>
///     Returns an indication of whether the task which is currently executing inline on the current context
///     is in the midst of an active cancellation (or will be shortly).  
/// </summary>
/// <returns>
///     <c>true</c> if the task  which is currently executing is canceling, <c>false</c> otherwise.
/// </returns>
/**/
static bool __cdecl is_current_task_canceling()
{
    return Context::IsCurrentTaskCollectionCanceling();
}

/// <summary>
///     Cancel the currently running task. This function can be called from within a task's body of
///     execution to begin a cancellation.
/// </summary>
/**/
static void __cdecl cancel_current_task()
{
    throw "not_supported in sample pack version of Tasks";
}

namespace details 
{
    // Helper struct for when_all, when_any operators to know when tasks have completed
    template<typename _Type>
    struct _RunAllParam
    {
        std::vector<_Type> _M_vector;
        _Type _M_mergeVal;
        long volatile _M_lCompleteCount;
        long volatile _M_lCancelCount;
        _RunAllParam() : _M_lCompleteCount(0), _M_lCancelCount(0) {}
    };

    // Helper struct specialization for void
    template<>
    struct _RunAllParam<void>
    {
        long volatile _M_lCompleteCount;
        long volatile _M_lCancelCount;
        _RunAllParam() : _M_lCompleteCount(0), _M_lCancelCount(0) {}
    };

    template<typename _ElementType, typename _Iterator>
    struct _WhenAllImpl
    {
        static task<std::vector<_ElementType>> _Perform(_Iterator _Begin, _Iterator _End) {
            auto _PParam = new ::Concurrency::samples::details::_RunAllParam<_ElementType>();
            task_completion_event<::Concurrency::samples::details::_Unit_type> _Completed;
            task<::Concurrency::samples::details::_Unit_type> _All_tasks_completed(_Completed);

            if( _Begin == _End )
            {
                _Completed.set(::Concurrency::samples::details::_Unit_type());
            }
            else
            {
                // Copy the tasks to an internal vector for processing. This allows const iterator types
                // to be processed.
                std::vector<task<_ElementType>> _Tasks(_Begin, _End);
                size_t _Len = _Tasks.size();
                _PParam->_M_vector.resize(_Len);
                size_t index = 0;
                for (auto _PTask = _Tasks.begin(); _PTask != _Tasks.end(); ++_PTask)
                {
                    _PTask->continue_with([=](_ElementType _Result) {
                        _PParam->_M_vector[index] = _Result;
                        if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) _Len))
                        {
                            if (!_Completed.set(::Concurrency::samples::details::_Unit_type()))
                            {
                                delete _PParam;
                            }
                        }
                    });

                    _PTask->_Continue_on_cancel([=]() {
                        _Completed._Cancel();
                        if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) _Len)) 
                        {
                            delete _PParam;
                        }
                    });

                    index++;
                }
            }

            return _All_tasks_completed.continue_with([=](::Concurrency::samples::details::_Unit_type) -> std::vector<_ElementType> {
                auto _Result = _PParam->_M_vector; // copy by value
                delete _PParam;
                return _Result;
            });
        }
    };

    template<typename _ElementType, typename _Iterator>
    struct _WhenAllImpl<std::vector<_ElementType>, _Iterator>
    {
        static task<std::vector<_ElementType>> _Perform(_Iterator _Begin, _Iterator _End) {
            auto _PParam = new ::Concurrency::samples::details::_RunAllParam<std::vector<_ElementType>>();
            task_completion_event<::Concurrency::samples::details::_Unit_type> _Completed;
            task<::Concurrency::samples::details::_Unit_type> _All_tasks_completed(_Completed);

            // Copy the tasks to an internal vector for processing. This allows const iterator types
            // to be processed.
            std::vector<task<std::vector<_ElementType>>> _Tasks(_Begin, _End);
            size_t _Len = _Tasks.size();

            // TODO: handle empty vector?

            _PParam->_M_vector.resize(_Len);
            size_t index = 0;
            for (auto _PTask = _Tasks.begin(); _PTask != _Tasks.end(); ++_PTask)
            {
                _PTask->continue_with([=](std::vector<_ElementType> _Result) {
                    _PParam->_M_vector[index] = _Result;
                    if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) _Len))
                    {
                        if (!_Completed.set(::Concurrency::samples::details::_Unit_type()))
                        {
                            delete _PParam;
                        }
                    }
                });

                _PTask->_Continue_on_cancel([=, &_All_tasks_completed]() {
                    _Completed._Cancel();
                    if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) _Len)) 
                    {
                        delete _PParam;
                    }
                });

                index++;
            }

            return _All_tasks_completed.continue_with([=](::Concurrency::samples::details::_Unit_type) -> std::vector<_ElementType> {
                _ASSERTE(_PParam->_M_lCompleteCount == ((long) _Len));
                std::vector<_ElementType> _Result = _PParam->_M_vector[0];
                _Result.insert(_Result.end(), _PParam->_M_vector[1].begin(), _PParam->_M_vector[1].end());
                delete _PParam;
                return _Result;
            });
        }
    };

    template<typename _Iterator>
    struct _WhenAllImpl<void, _Iterator>
    {
        static task<void> _Perform(_Iterator _Begin, _Iterator _End) {
            auto _PParam = new ::Concurrency::samples::details::_RunAllParam<::Concurrency::samples::details::_Unit_type>();
            task_completion_event<::Concurrency::samples::details::_Unit_type> _Completed;
            task<::Concurrency::samples::details::_Unit_type> _All_tasks_completed(_Completed);

            if( _Begin == _End )
            {
                _Completed.set(::Concurrency::samples::details::_Unit_type());
            }
            else
            {
                // Copy the tasks to an internal vector for processing. This allows const iterator types
                // to be processed.
                std::vector<task<void>> _Tasks(_Begin, _End);
                size_t _Len = _Tasks.size();

                for (auto _PTask = _Tasks.begin(); _PTask != _Tasks.end(); ++_PTask)
                {
                    _PTask->continue_with([=]() {
                        if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) _Len))
                        {
                            if (!_Completed.set(::Concurrency::samples::details::_Unit_type()))
                            {
                                delete _PParam;
                            }
                        }
                    });

                    _PTask->_Continue_on_cancel([=, &_All_tasks_completed]() {
                        _Completed._Cancel();
                        if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) _Len)) 
                        {
                            delete _PParam;
                        }
                    });
                }
            }

            return _All_tasks_completed.continue_with([=](::Concurrency::samples::details::_Unit_type){
                delete _PParam;
            });
        }
    };

    template<typename _ReturnType>
    task<std::vector<_ReturnType>> _WhenAllVectorAndValue(task<std::vector<_ReturnType>>& _VectorTask, task<_ReturnType>& _ValueTask,
                                                          bool _OutputVectorFirst)
    {
        auto _PParam = new ::Concurrency::samples::details::_RunAllParam<_ReturnType>();
        task_completion_event<::Concurrency::samples::details::_Unit_type> _Completed;
        task<::Concurrency::samples::details::_Unit_type> _All_tasks_completed(_Completed);

        _VectorTask.continue_with([=](std::vector<_ReturnType> _Result) {
                _PParam->_M_vector = _Result;
                if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) 2))
                {
                    if (!_Completed.set(::Concurrency::samples::details::_Unit_type()))
                    {
                        delete _PParam;
                    }
                }
            });

        _VectorTask._Continue_on_cancel([=, &_All_tasks_completed]() {
            _Completed._Cancel();
            if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) 2)) 
            {
                delete _PParam;
            }
        });

        _ValueTask.continue_with([=](_ReturnType _Result) {
                _PParam->_M_mergeVal = _Result;
                if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) 2))
                {
                    if (!_Completed.set(::Concurrency::samples::details::_Unit_type()))
                    {
                        delete _PParam;
                    }
                }
            });

        _ValueTask._Continue_on_cancel([=, &_All_tasks_completed]() {
            _Completed._Cancel();
            if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) 2)) 
            {
                delete _PParam;
            }
        });

        if (_OutputVectorFirst == true)
        {
            return _All_tasks_completed.continue_with([=](::Concurrency::samples::details::_Unit_type) -> std::vector<_ReturnType> {
                _ASSERTE(_PParam->_M_lCompleteCount == ((long) 2));
                auto _Result = _PParam->_M_vector; // copy by value
                _Result.push_back(_PParam->_M_mergeVal);
                delete _PParam;
                return _Result;
            });
        }
        else
        {
            return _All_tasks_completed.continue_with([=](::Concurrency::samples::details::_Unit_type) -> std::vector<_ReturnType> {
                _ASSERTE(_PParam->_M_lCompleteCount == ((long) 2));
                auto _Result = _PParam->_M_vector; // copy by value
                _Result.push_back(_PParam->_M_mergeVal);
                delete _PParam;
                return _Result;
            });
        }
    }
} // namespace details

/// <summary>
///     First-class tasks when_all API using begin and end iterators
/// </summary>
/// <typeparam name="_Iterator">
///     The type of the input iterator.
/// </typeparam>
/// <param name="_Begin">
///     Position of the first element in the range of elements to be combined and waited on.
/// </param>
/// <param name="_End">
///     Position of the first element beyond the range of elements to be combined and waited on.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<std::vector<T>></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template <typename _Iterator>
auto when_all(_Iterator _Begin, _Iterator _End) 
    -> decltype (::Concurrency::samples::details::_WhenAllImpl<std::iterator_traits<_Iterator>::value_type::_TaskType, _Iterator>::_Perform(_Begin, _End))
{
    typedef std::iterator_traits<_Iterator>::value_type::_TaskType _ElementType;
    return ::Concurrency::samples::details::_WhenAllImpl<_ElementType, _Iterator>::_Perform(_Begin, _End);
}

/// <summary>
///     When_all operator (task1 && task2) for two tasks
/// </summary>
/// <typeparam name="_ReturnType">
///     The type of the tasks.
/// </typeparam>
/// <param name="_Lhs">
///     The first task to combine into this when_all task.
/// </param>
/// <param name="_Rhs">
///     The second task to combine into this when_all task.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<std::vector<T>></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template<typename _ReturnType>
task<std::vector<_ReturnType>> operator&&(task<_ReturnType> _Lhs, task<_ReturnType> _Rhs)
{
    task<_ReturnType> _PTasks[2] = {_Lhs, _Rhs};
    return when_all(_PTasks, _PTasks+2);
}

/// <summary>
///     When_all operator (task1 && task2) for two tasks
/// </summary>
/// <typeparam name="_ReturnType">
///     The type of the tasks.
/// </typeparam>
/// <param name="_Lhs">
///     The first task to combine into this when_all task.
/// </param>
/// <param name="_Rhs">
///     The second task to combine into this when_all task.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<std::vector<T>></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template<typename _ReturnType>
task<std::vector<_ReturnType>> operator&&(task<std::vector<_ReturnType>> _Lhs, task<_ReturnType> _Rhs)
{
    return ::Concurrency::samples::details::_WhenAllVectorAndValue(_Lhs, _Rhs, true);
}

/// <summary>
///     When_all operator (task1 && task2) for two tasks
/// </summary>
/// <typeparam name="_ReturnType">
///     The type of the tasks.
/// </typeparam>
/// <param name="_Lhs">
///     The first task to combine into this when_all task.
/// </param>
/// <param name="_Rhs">
///     The second task to combine into this when_all task.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<std::vector<T>></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template<typename _ReturnType>
task<std::vector<_ReturnType>> operator&&(task<_ReturnType> _Lhs, task<std::vector<_ReturnType>> _Rhs)
{
    return ::Concurrency::samples::details::_WhenAllVectorAndValue(_Rhs, _Lhs, false);
}

/// <summary>
///     When_all operator (task1 && task2) for two tasks
/// </summary>
/// <typeparam name="_ReturnType">
///     The type of the tasks.
/// </typeparam>
/// <param name="_Lhs">
///     The first task to combine into this when_all task.
/// </param>
/// <param name="_Rhs">
///     The second task to combine into this when_all task.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<std::vector<T>></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template<typename _ReturnType>
task<std::vector<_ReturnType>> operator&&(task<std::vector<_ReturnType>> _Lhs, task<std::vector<_ReturnType>> _Rhs)
{
    task<std::vector<_ReturnType>> _PTasks[2] = {_Lhs, _Rhs};
    return when_all(_PTasks, _PTasks+2);
}

namespace details 
{
    template<typename _ElementType, typename _Iterator>
    struct _WhenAnyImpl
    {
        static task<std::pair<_ElementType, size_t>> _Perform(_Iterator _Begin, _Iterator _End) {
            auto _PParam = new ::Concurrency::samples::details::_RunAllParam<_ElementType>();
            task_completion_event<std::pair<_ElementType, size_t>> _Completed;
            task<std::pair<_ElementType, size_t>> _Any_tasks_completed(_Completed);
            size_t _Len = _End - _Begin;

            // Copy the tasks to an internal vector for processing. This allows const iterator types
            // to be processed.
            std::vector<task<_ElementType>> _Tasks(_Begin, _End);

            size_t index = 0;

            for (auto _PTask = _Tasks.begin(); _PTask != _Tasks.end(); ++_PTask)
            {
                _PTask->continue_with([=](_ElementType _Result) {
                    _Completed.set(std::make_pair(_Result, index));
                });

                _PTask->_Continue_on_cancel([=]() {
                    if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) _Len))
                        _Completed._Cancel();
                });

                index++;
            }

            // Clean up the memory if this task was canceled
            _Any_tasks_completed._Continue_on_cancel([=]() { delete _PParam; });

            return _Any_tasks_completed.continue_with([=](std::pair<_ElementType, size_t> _Result) -> std::pair<_ElementType, size_t> {
                delete _PParam;
                return _Result;
            });
        }
    };

    template<typename _Iterator>
    struct _WhenAnyImpl<void, _Iterator>
    {
        static task<size_t> _Perform(_Iterator _Begin, _Iterator _End) {
            auto _PParam = new ::Concurrency::samples::details::_RunAllParam<void>();
            task_completion_event<size_t> _Completed;
            task<size_t> _Any_tasks_completed(_Completed);
            size_t _Len = _End - _Begin;

            // Copy the tasks to an internal vector for processing. This allows const iterator types
            // to be processed.
            std::vector<task<void>> _Tasks(_Begin, _End);

            size_t index = 0;
            for (auto _PTask = _Tasks.begin(); _PTask != _Tasks.end(); ++_PTask)
            {
                _PTask->continue_with([=]() {
                    _Completed.set(index);
                });

                _PTask->_Continue_on_cancel([=]() {
                    if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) _Len))
                        _Completed._Cancel();
                });

                index++;
            }

            // Clean up the memory if this task was canceled
            _Any_tasks_completed._Continue_on_cancel([=]() { delete _PParam; });

            return _Any_tasks_completed.continue_with([=](size_t _Result) -> size_t {
                delete _PParam;
                return _Result;
            });
        }
    };
} // namespace details

/// <summary>
///     First-class tasks when_any API using begin and end iterators
/// </summary>
/// <typeparam name="_Iterator">
///     The type of the input iterator.
/// </typeparam>
/// <param name="_Begin">
///     Position of the first element in the range of elements to be combined and waited on.
/// </param>
/// <param name="_End">
///     Position of the first element beyond the range of elements to be combined and waited on.
/// </param>
/// <returns>
///     A task that completes when any one of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<std::pair<T, size_t>></c>. Where the first element of the pair is the result of the
///     completing task, and the second element is the index of the task that finished.  If the input tasks are of type <c>void</c> 
///     the output is a <c>task<size_t></c>, where the result is the index of the completing task.
/// </returns>
/**/
template<typename _Iterator>
auto when_any(_Iterator _Begin, _Iterator _End)
    -> decltype (::Concurrency::samples::details::_WhenAnyImpl<std::iterator_traits<_Iterator>::value_type::_TaskType, _Iterator>::_Perform(_Begin, _End))
{
    typedef std::iterator_traits<_Iterator>::value_type::_TaskType _ElementType;
    return ::Concurrency::samples::details::_WhenAnyImpl<_ElementType, _Iterator>::_Perform(_Begin, _End);
}

/// <summary>
///     When_any operator (task1 || task2) for two task<T>
/// </summary>
/// <typeparam name="_ReturnType">
///     The type of the tasks.
/// </typeparam>
/// <param name="_Lhs">
///     The first task to combine into this when_all task.
/// </param>
/// <param name="_Rhs">
///     The second task to combine into this when_all task.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<T></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template<typename _ReturnType>
task<_ReturnType> operator||(task<_ReturnType> _Lhs, task<_ReturnType> _Rhs)
{
    auto _PParam = new ::Concurrency::samples::details::_RunAllParam<_ReturnType>();
    task_completion_event<_ReturnType> _Completed;
    task<_ReturnType> _Any_tasks_completed(_Completed);

    _Lhs.continue_with([=](_ReturnType _Result) {
            _Completed.set(_Result);
        });

    _Lhs._Continue_on_cancel([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) 2))
            {
                _Completed._Cancel();
                delete _PParam; 
            }
        });

    _Rhs.continue_with([=](_ReturnType _Result) {
            _Completed.set(_Result);
        });

    _Rhs._Continue_on_cancel([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) 2))
            {
                _Completed._Cancel();
                delete _PParam; 
            }
        });

    return _Any_tasks_completed.continue_with([=](_ReturnType _Ret) -> _ReturnType {
        delete _PParam;
        return _Ret;
    });
}

/// <summary>
///     When_any operator (task1 || task2) for one task<std::vector<T>> and a task<T>
/// </summary>
/// <typeparam name="_ReturnType">
///     The type of the tasks.
/// </typeparam>
/// <param name="_Lhs">
///     The first task to combine into this when_all task.
/// </param>
/// <param name="_Rhs">
///     The second task to combine into this when_all task.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<T></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template<typename _ReturnType>
task<std::vector<_ReturnType>> operator||(task<std::vector<_ReturnType>> _Lhs, task<_ReturnType> _Rhs)
{
    auto _PParam = new ::Concurrency::samples::details::_RunAllParam<_ReturnType>();
    task_completion_event<std::vector<_ReturnType>> _Completed;
    task<std::vector<_ReturnType>> _Any_tasks_completed(_Completed);

    _Lhs.continue_with([=](std::vector<_ReturnType> _Result) {
            _Completed.set(_Result);
        });

    _Lhs._Continue_on_cancel([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) 2))
            {
                _Completed._Cancel();
                delete _PParam; 
            }
        });

    _Rhs.continue_with([=](_ReturnType _Result) {
            std::vector<_ReturnType> _Vec;
            _Vec.push_back(_Result);
            _Completed.set(_Vec);
        });

    _Rhs._Continue_on_cancel([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) 2))
            {
                _Completed._Cancel();
                delete _PParam; 
            }
        });

    return _Any_tasks_completed.continue_with([=](std::vector<_ReturnType> _Ret) -> std::vector<_ReturnType> {
        delete _PParam;
        return _Ret;
    });
}

/// <summary>
///     When_any operator (task1 || task2) for one task<T> and a task<std::vector<T>>
/// </summary>
/// <typeparam name="_ReturnType">
///     The type of the tasks.
/// </typeparam>
/// <param name="_Lhs">
///     The first task to combine into this when_all task.
/// </param>
/// <param name="_Rhs">
///     The second task to combine into this when_all task.
/// </param>
/// <returns>
///     A task that completes when all of the input tasks are complete. If the input tasks are of type <c>T</c>, the output
///     of this function will be a <c>task<T></c>. If the input tasks are of type <c>void</c> the output will 
///     also be a <c>task<void></c>.
/// </returns>
/**/
template<typename _ReturnType>
task<std::vector<_ReturnType>> operator||(task<_ReturnType> _Lhs, task<std::vector<_ReturnType>> _Rhs)
{
    return _Rhs || _Lhs;
}

// When_all operator (task1 && task2) for two task<void>
static task<void> operator&&(task<void> _Lhs, task<void> _Rhs)
{
    auto _PParam = new ::Concurrency::samples::details::_RunAllParam<void>();
    task_completion_event<::Concurrency::samples::details::_Unit_type> _Completed;
    task<::Concurrency::samples::details::_Unit_type> _All_tasks_completed(_Completed);

    _Lhs.continue_with([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) 2))
            {
                _Completed.set(::Concurrency::samples::details::_Unit_type());
            }
        });

    _Lhs._Continue_on_cancel([=]() {
            _Completed._Cancel();
        });

    _Rhs.continue_with([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCompleteCount) == ((long) 2))
            {
                _Completed.set(::Concurrency::samples::details::_Unit_type());
            }
        });

    _Rhs._Continue_on_cancel([=]() {
            _Completed._Cancel();
        });

    // Clean up the memory if this task was canceled
    _All_tasks_completed._Continue_on_cancel([=]() { delete _PParam; });

    return _All_tasks_completed.continue_with([=](::Concurrency::samples::details::_Unit_type) -> void {
        _ASSERTE(_PParam->_M_lCompleteCount == ((long) 2));
        delete _PParam;
    });
}

// When_any operator (task1 || task2) for two task<void>
static task<void> operator||(task<void> _Lhs, task<void> _Rhs)
{
    auto _PParam = new ::Concurrency::samples::details::_RunAllParam<void>();
    task_completion_event<::Concurrency::samples::details::_Unit_type> _Completed;
    task<::Concurrency::samples::details::_Unit_type> _Any_task_completed(_Completed);

    _Lhs.continue_with([=]() {
            _Completed.set(::Concurrency::samples::details::_Unit_type());
        });

    _Lhs._Continue_on_cancel([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) 2))
            {
                _Completed._Cancel();
            }
        });

    _Rhs.continue_with([=]() {
            _Completed.set(::Concurrency::samples::details::_Unit_type());
        });

    _Rhs._Continue_on_cancel([=]() {
            if (_InterlockedIncrement(&_PParam->_M_lCancelCount) == ((long) 2))
            {
                _Completed._Cancel();
            }
        });

    // Clean up the memory if this task was canceled
    _Any_task_completed._Continue_on_cancel([=]() { delete _PParam; });

    return _Any_task_completed.continue_with([=](::Concurrency::samples::details::_Unit_type){  delete _PParam; });
}

} // namespace samples

} // namespace Concurrency

