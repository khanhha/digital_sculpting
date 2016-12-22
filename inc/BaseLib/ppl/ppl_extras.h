//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: ppl_extras.h
//
//  Implementation of various PPL algorithms.
//
//--------------------------------------------------------------------------

#pragma once

#include <ppl.h>
#include <numeric>
#include <vector>
#include "concrt_extras.h"
namespace Concurrency
{
namespace samples
{
namespace details
{
};
template<class in_it,class pr>
inline bool parallel_all_of(in_it first, in_it last, const pr& pred)
{

    typedef typename std::iterator_traits<in_it>::value_type item_type;

    structured_task_group tasks;

    auto for_each_predicate = [&pred,&tasks](const item_type& cur){
        if (!pred(cur))
            tasks.cancel();
    };

    auto task = make_task([&first,&last,&pred,&tasks,&for_each_predicate](){
        parallel_for_each(first,last,for_each_predicate);
    });

    return tasks.run_and_wait(task) != canceled;
}

template<class in_it,class pr>
inline bool parallel_any_of(in_it first, in_it last, const pr& pred)
{

    typedef typename std::iterator_traits<in_it>::value_type item_type;

    structured_task_group tasks;

    auto for_each_predicate = [&pred,&tasks](const item_type& cur){
        if (pred(cur))
            tasks.cancel();
    };

    auto task = make_task([&first,&last,&pred,&tasks,&for_each_predicate](){
        parallel_for_each(first,last,for_each_predicate);
    });

    return tasks.run_and_wait(task) == canceled;
}

template<class in_it,class pr>
inline bool parallel_none_of(in_it first, in_it last, const pr& pred)
{

    typedef typename std::iterator_traits<in_it>::value_type item_type;

    structured_task_group tasks;

    auto for_each_predicate = [&pred,&tasks](const item_type& cur){
        if (pred(cur))
            tasks.cancel();
    };

    auto task = make_task([&first,&last,&pred,&tasks,&for_each_predicate](){
        parallel_for_each(first,last,for_each_predicate);
    });

    return tasks.run_and_wait(task) != canceled;
}

template<class in_it, class pr>
inline typename std::iterator_traits<in_it>::difference_type
    parallel_count_if(in_it first, in_it last, const pr& pred)
{
    typedef typename std::iterator_traits<in_it>::value_type item_type;

    combinable<iterator_traits<in_it>::difference_type> sums;

    parallel_for_each(first,last,[&](const item_type& cur){
        if (pred(cur))
            ++sums.local();
    });

    return sums.combine(std::plus<iterator_traits<in_it>::difference_type>());
}
namespace details
{
    template <typename random_iterator, typename index_type, typename function, bool is_iterator>
    class fixed_chunk_class
    {
    public:
        fixed_chunk_class(const random_iterator& first, index_type first_iteration, index_type last_iteration, const index_type& step, const function& func) :
            m_first(first), m_first_iteration(first_iteration), m_last_iteration(last_iteration), m_step(step), m_function(func)
            {
                // Empty constructor since members are already assigned
            }

            void operator()() const
            {
                // Keep the secondary, scaled, loop index for quick indexing into the data structure
                index_type scaled_index = m_first_iteration * m_step;

                for (index_type i = m_first_iteration; i < m_last_iteration; (i++, scaled_index += m_step))
                {
                    // Execute one iteration: the element is at scaled index away from the first element.
                    ::Concurrency::_Parallel_chunk_helper_invoke<random_iterator, index_type, function, is_iterator>::_Invoke(m_first, scaled_index, m_function);
                }
            }

    private:
        const random_iterator& m_first;
        const index_type&      m_step;
        const function&        m_function;
        const index_type       m_first_iteration;
        const index_type       m_last_iteration;

        fixed_chunk_class const & operator=(fixed_chunk_class const&);    // no assignment operator
    };

    template <typename random_iterator, typename index_type, typename function>
    void parallel_for_impl(random_iterator first, random_iterator last, index_type step, const function& func)
    {
        const bool is_iterator = !(std::tr1::is_same<random_iterator, index_type>::value);
        typedef details::fixed_chunk_class<random_iterator, index_type, function, is_iterator> worker_class;

        // The step argument must be 1 or greater; otherwise it is an invalid argument
        if (step < 1)
        {
            throw std::invalid_argument("step");
        }

        // If there are no elements in this range we just return
        if (first >= last)
        {
            return;
        }

        // This is safe to do only if one can assume no signed overflow problems
        index_type range = last - first;

        if (range <= step)
        {
            index_type iteration = 0;
            ::Concurrency::_Parallel_chunk_helper_invoke<random_iterator, index_type, function, is_iterator>::_Invoke(first, iteration, func);
        }
        else
        {
            // Get the number of chunks to divide the work
            index_type num_chunks = _Get_num_chunks<index_type>();
            index_type iterations;

            if (step != 1)
            {
                iterations = ((range - 1) / step) + 1;
            }
            else
            {
                iterations = last - first;
            }

            // Allocate memory on the stack for task_handles to ensure everything is properly structured.
            task_handle<worker_class> * chunk_helpers = (task_handle<worker_class> *) _alloca(sizeof(task_handle<worker_class>) * num_chunks);

            structured_task_group task_group;
            ::Concurrency::_Parallel_chunk_impl(first, iterations, step, func, task_group, chunk_helpers, num_chunks, true);
        }
    }
    template <typename random_iterator, typename function>
    void parallel_for_each_impl(const random_iterator& first, const random_iterator& last, const function& func, std::random_access_iterator_tag)
    {
        std::iterator_traits<random_iterator>::difference_type step = 1;
        details::parallel_for_impl(first, last, step, func);
    }

    template <typename forward_iterator, typename function>
    class parallel_for_each_helper
    {
    public:
        typedef typename std::iterator_traits<forward_iterator>::value_type value_type;
        static const unsigned int size = 1024;

        parallel_for_each_helper(forward_iterator& first, const forward_iterator& last, const function& func) :
        m_function(func), m_len(0)
        {
            // Add a batch of work items to this functor's array
            for (unsigned int index=0; (index < size) && (first != last); index++)
            {
                m_element[m_len++] = &(*first++);
            }
        }

        void operator()() const
        {
            // Invoke parallel_for on the batched up array of elements
            parallel_for_impl(0U, m_len, 1U,
                [this] (unsigned int index)
            {
                m_function(*(m_element[index]));
            }
            );
        }

    private:

        const function& m_function;
        value_type *    m_element[size];
        unsigned int    m_len;

        parallel_for_each_helper const & operator=(parallel_for_each_helper const&);    // no assignment operator
    };
    template <typename forward_iterator, typename function>
    void parallel_for_each_forward_impl(forward_iterator& first, const forward_iterator& last, const function& func, task_group& tg)
    {
        // This functor will be copied on the heap and will execute the chunk in parallel
        {
            Concurrency::samples::details::parallel_for_each_helper<forward_iterator, function> functor(first, last, func);
            tg.run(functor);
        }

        // If there is a tail, push the tail
        if (first != last)
        {
            tg.run(
                [&first, &last, &func, &tg]
            {
                Concurrency::samples::details::parallel_for_each_forward_impl(first, last, func, tg);
            }
            );
        }
    }

    template <typename forward_iterator, typename function>
    void parallel_for_each_impl(forward_iterator first, const forward_iterator& last, const function& func, std::forward_iterator_tag)
    {
        // Since this is a forward iterator, it is difficult to validate that first comes before _Last, so
        // it is up to the user to provide valid range.
        if (first != last)
        {
            task_group tg;
            parallel_for_each_forward_impl(first, last, func, tg);
            tg.wait();
        }
    }
};

// Public API entries for parallel_for_fixed

/// <summary>
///     Performs parallel iteration over a range of indices from first
///     to last, not including last.
/// </summary>
/// <param name="first">
///     First index to be included in parallel iteration.
/// </param>
/// <param name="last">
///     First index after first not to be included in parallel iteration.
/// </param>
/// <param name="step">
///     Step to be used in computing index for the given iteration. Only positive step is supported;
///     exception is thrown if step is smaller than or equal to 0.
/// </param>
/// <param name="func">
///     Function object to be executed on each iteration.
/// </param>
/// <remarks>
///     For more information, see <see cref="Parallel Algorithms"/>.
/// </remarks>
template <typename index_type, typename function>
void parallel_for_fixed(index_type first, index_type last, index_type step, const function& func)
{
    _Trace_ppl_function(PPLParallelForEventGuid, _TRACE_LEVEL_INFORMATION, CONCRT_EVENT_START);
    details::parallel_for_impl(first, last, step, func);
    _Trace_ppl_function(PPLParallelForEventGuid, _TRACE_LEVEL_INFORMATION, CONCRT_EVENT_END);
}

/// <summary>
///     Performs parallel iteration over a range of indices from first
///     to last, not including last.
/// </summary>
/// <param name="first">
///     First index to be included in parallel iteration.
/// </param>
/// <param name="last">
///     First index after first not to be included in parallel iteration.
/// </param>
/// <param name="func">
///     Function object to be executed on each iteration.
/// </param>
/// <remarks>
///     For more information, see <see cref="Parallel Algorithms"/>.
/// </remarks>
template <typename index_type, typename function>
void parallel_for_fixed(index_type first, index_type last, const function& func)
{
    parallel_for_fixed(first, last, index_type(1), func);
}

/// <summary>
///     This template function is semantically equivalent to std::for_each, except that
///     the iteration is done in parallel and ordering is unspecified. The function argument
///     func must support operator()(T) where T is the item type of the container being
///     iterated over.
/// </summary>
/// <param name="first">
///     First element to be included in parallel iteration.
/// </param>
/// <param name="last">
///     First element after first not to be included in parallel iteration.
/// </param>
/// <param name="func">
///     Function object to be executed on each iteration.
/// </param>
/// <remarks>
///     For more information, see <see cref="Parallel Algorithms"/>.
/// </remarks>
template <typename iterator, typename function>
void parallel_for_each_fixed(iterator first, iterator last, const function& func)
{
    _Trace_ppl_function(PPLParallelForeachEventGuid, _TRACE_LEVEL_INFORMATION, CONCRT_EVENT_START);
    details::parallel_for_each_impl(first, last, func, std::_Iter_cat(first));
    _Trace_ppl_function(PPLParallelForeachEventGuid, _TRACE_LEVEL_INFORMATION, CONCRT_EVENT_END);
}

namespace details
{
    //
    // _MallocaArrayHolder is used when the allocation size is known up front, and the memory must be allocated in a contiguous space
    //
    template<typename _ElemType>
    class _MallocaArrayHolder
    {
    public:

        _MallocaArrayHolder() : _M_ElemArray(NULL), _M_ElemsConstructed(0) {}

        // _Initialize takes the pointer to the memory allocated by the user via _malloca
        void _Initialize(_ElemType * _Elem)
        {
            _M_ElemArray = _Elem;
            _M_ElemsConstructed = 0;
        }

        // Register the next slot for destruction. Because we only keep the index of the last slot to be destructed,
        // this method must be called sequentially from 0 to N where N < _ElemCount.
        void _IncrementConstructedElemsCount()
        {
            _M_ElemsConstructed++;
        }

        virtual ~_MallocaArrayHolder()
        {
            for( size_t _I=0; _I < _M_ElemsConstructed; ++_I )
            {
                _M_ElemArray[_I]._ElemType::~_ElemType();
            }
            // Works even when object was not initialized, i.e. _M_ElemArray == NULL
            _freea(_M_ElemArray);
        }
    private:
        _ElemType * _M_ElemArray;
        size_t     _M_ElemsConstructed;

        // not supposed to be copy-constructed or assigned
        _MallocaArrayHolder(const _MallocaArrayHolder & );
        _MallocaArrayHolder&  operator = (const _MallocaArrayHolder & );
    };
};

// Disable C4180: qualifier applied to function type has no meaning; ignored
// Warning fires for passing Foo function pointer to parallel_for instead of &Foo.
#pragma warning(push)
#pragma warning(disable: 4180)

/// <summary>
///     This template function is semantically similar with <c>std::accumulate</c>, except that it requires associativity (not commutativity) 
///     for the reduce functor and an identity value instead of the initial value in std::accumulate. The execution will be parallelized 
///     to fully utilize all CPUs, the execution order of each chunk is not determined.
/// </summary>
/// <typeparam name="_Forward_iterator">
///     The iterator type of input range, it must at least to be a <c>forward_iterator</c>.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for reduce.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for reduce.
/// </param>
/// <param name="_Identity">
///     The identity value has the <c>_Reduce_type</c> type; it will be directly passed to <c>_Range_fun</c> function.
/// </param>
/// <returns>
///     The result of the reduction.
/// </returns>
/// <remarks>
///     For the first function overload, the iterator's value_type <c>T</c> must be the same as the identity value type as well as the reduction 
///     result type. 
///     <para>The <c>T T::operator + (T)</c> is required to reduce elements in each chunk and sub results reduced from chunks. Particularly, for 
///     the first phase reduction, operator, <c>T = T + T</c> will be applied in each chunk with the identity value as the initial value. In the
///     second phase, the operator <c>T = T + T</c> will be applied to reduce the sub results of first phase reduce. </para>
///     <para>For the second function overload, the iterator's value_type <c>T</c> must be the same as the initial value type as well as the 
///     reduction result type. A symmetric binary functor <c>_Sym_fun</c>: <c>T (T, T)</c> is required to reduce elements in each chunk and sub 
///     results reduced from chunks. Particularly, for the first phase reduce, functor <c>_Sym_fun</c>: <c>T (T, T)</c> will be applied on each 
///     chunk with the identity value as the initial value. In the second phase,<c>_Sym_fun</c>: <c>T (T, T)</c> will be used to reduce sub results 
///     from the first phase.</para>
///     <para>For the third function overload, the iterator's value_type <c>T</c> can be different from the identity value type or the reduce result 
///     type <c>_Reduce_type</c>. The range reduce functor <c>_Range_fun: _Reduce_type (_Forward_iterator, _Forward_iterator, _Reduce_type)</c> is 
///     required in first phase reduce, and symmetric binary functor <c>_Sym_fun: _Reduce_type (_Reduce_type, _Reduce_type) </c> is required in the 
///     second phase reduce. Particularly, for the first phase reduce, functor <c>_Range_fun: 
///     _Reduce_type (_Forward_iterator, _Forward_iterator, _Reduce_type)</c> will be applied with the identity value as the initial value. In the 
///     second phase, <c>_Sym_fun: _Reduce_type (_Reduce_type, _Reduce_type) </c> will be applied to reduce the sub results from first phase, and the 
///     result will be returned as final result.</para>
///     <para>The user should not have any assumptions on chunk division.</para>
/// </remarks>
/**/
template<typename _Forward_iterator>
inline typename std::iterator_traits<_Forward_iterator>::value_type parallel_reduce(
    _Forward_iterator _Begin, _Forward_iterator _End, const typename std::iterator_traits<_Forward_iterator>::value_type &_Identity)
{
    return parallel_reduce(_Begin, _End, _Identity, std::plus<std::iterator_traits<_Forward_iterator>::value_type>());
}

/// <summary>
///     This template function is semantically similar with <c>std::accumulate</c>, except that it requires associativity (not commutativity) 
///     for the reduce functor and an identity value instead of the initial value in std::accumulate. The execution will be parallelized 
///     to fully utilize all CPUs, the execution order of each chunk is not determined.
/// </summary>
/// <typeparam name="_Forward_iterator">
///     The iterator type of input range, it must at least to be a <c>forward_iterator</c>.
/// </typeparam>
/// <typeparam name="_Sym_reduce_fun">
///     The symmetric reduce function type <c>_Reduce_type (_Reduce_type, _Reduce_type)</c>. 
///     This type (return type and arguments type) should be consistent with the output type of <c>_Range_reduce_fun</c> and
///     return type of this <c>parallel_reduce</c> function.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for reduce.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for reduce.
/// </param>
/// <param name="_Identity">
///     The identity value has the <c>_Reduce_type</c> type; it will be directly passed to <c>_Range_fun</c> function.
/// </param>
/// <param name="_Sym_fun">
///     The reduce function that will be used in the second phase reduce, for more information please refer to remarks.
/// </param>
/// <returns>
///     The result of the reduction.
/// </returns>
/// <remarks>
///     For the first function overload, the iterator's value_type <c>T</c> must be the same as the identity value type as well as the reduction 
///     result type. 
///     <para>The <c>T T::operator + (T)</c> is required to reduce elements in each chunk and sub results reduced from chunks. Particularly, for 
///     the first phase reduction, operator, <c>T = T + T</c> will be applied in each chunk with the identity value as the initial value. In the
///     second phase, the operator <c>T = T + T</c> will be applied to reduce the sub results of first phase reduce. </para>
///     <para>For the second function overload, the iterator's value_type <c>T</c> must be the same as the initial value type as well as the 
///     reduction result type. A symmetric binary functor <c>_Sym_fun</c>: <c>T (T, T)</c> is required to reduce elements in each chunk and sub 
///     results reduced from chunks. Particularly, for the first phase reduce, functor <c>_Sym_fun</c>: <c>T (T, T)</c> will be applied on each 
///     chunk with the identity value as the initial value. In the second phase,<c>_Sym_fun</c>: <c>T (T, T)</c> will be used to reduce sub results 
///     from the first phase.</para>
///     <para>For the third function overload, the iterator's value_type <c>T</c> can be different from the identity value type or the reduce result 
///     type <c>_Reduce_type</c>. The range reduce functor <c>_Range_fun: _Reduce_type (_Forward_iterator, _Forward_iterator, _Reduce_type)</c> is 
///     required in first phase reduce, and symmetric binary functor <c>_Sym_fun: _Reduce_type (_Reduce_type, _Reduce_type) </c> is required in the 
///     second phase reduce. Particularly, for the first phase reduce, functor <c>_Range_fun: 
///     _Reduce_type (_Forward_iterator, _Forward_iterator, _Reduce_type)</c> will be applied with the identity value as the initial value. In the 
///     second phase, <c>_Sym_fun: _Reduce_type (_Reduce_type, _Reduce_type) </c> will be applied to reduce the sub results from first phase, and the 
///     result will be returned as final result.</para>
///     <para>The user should not have any assumptions on chunk division.</para>
/// </remarks>
/**/
template<typename _Forward_iterator, typename _Sym_reduce_fun>
inline typename std::iterator_traits<_Forward_iterator>::value_type parallel_reduce(_Forward_iterator _Begin, _Forward_iterator _End, 
    const typename std::iterator_traits<_Forward_iterator>::value_type &_Identity, _Sym_reduce_fun _Sym_fun)
{
    typedef typename std::remove_cv<std::iterator_traits<_Forward_iterator>::value_type>::type _Reduce_type;

    return parallel_reduce(_Begin, _End, _Identity, 
        [_Sym_fun](_Forward_iterator _Begin, _Forward_iterator _End, _Reduce_type _Init)->_Reduce_type 
    {
        while (_Begin != _End)
        {
            _Init = _Sym_fun(_Init, *_Begin++); 
        }

        return _Init;
    },
        _Sym_fun);
}

/// <summary>
///     This template function is semantically similar with <c>std::accumulate</c>, except that it requires associativity (not commutativity) 
///     for the reduce functor and an identity value instead of the initial value in std::accumulate. The execution will be parallelized 
///     to fully utilize all CPUs, the execution order of each chunk is not determined.
/// </summary>
/// <typeparam name="_Reduce_type">
///     The type that the input will reduce to, which can be different from the input element type. 
///     The return value and identity value will has this type.
/// </typeparam>
/// <typeparam name="_Forward_iterator">
///     The iterator type of input range, it must at least to be a <c>forward_iterator</c>.
/// </typeparam>
/// <typeparam name="_Range_reduce_fun">
///     The type of reduce function with type <c>_Reduce_type (_Forward_iterator, _Forward_iterator, _Reduce_type)</c>. 
///     The type of this function should be consistent with the input iterators and identity value.
/// </typeparam>
/// <typeparam name="_Sym_reduce_fun">
///     The symmetric reduce function type <c>_Reduce_type (_Reduce_type, _Reduce_type)</c>. 
///     This type (return type and arguments type) should be consistent with the output type of <c>_Range_reduce_fun</c> and
///     return type of this <c>parallel_reduce</c> function.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for reduce.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for reduce.
/// </param>
/// <param name="_Identity">
///     The identity value has the <c>_Reduce_type</c> type; it will be directly passed to <c>_Range_fun</c> function.
/// </param>
/// <param name="_Range_fun">
///     The reduce function that will be used in the first phase reduce, for more information please refer to remarks.
/// </param>
/// <param name="_Sym_fun">
///     The reduce function that will be used in the second phase reduce, for more information please refer to remarks.
/// </param>
/// <returns>
///     The result of the reduction.
/// </returns>
/// <remarks>
///     For the first function overload, the iterator's value_type <c>T</c> must be the same as the identity value type as well as the reduction 
///     result type. 
///     <para>The <c>T T::operator + (T)</c> is required to reduce elements in each chunk and sub results reduced from chunks. Particularly, for 
///     the first phase reduction, operator, <c>T = T + T</c> will be applied in each chunk with the identity value as the initial value. In the
///     second phase, the operator <c>T = T + T</c> will be applied to reduce the sub results of first phase reduce. </para>
///     <para>For the second function overload, the iterator's value_type <c>T</c> must be the same as the initial value type as well as the 
///     reduction result type. A symmetric binary functor <c>_Sym_fun</c>: <c>T (T, T)</c> is required to reduce elements in each chunk and sub 
///     results reduced from chunks. Particularly, for the first phase reduce, functor <c>_Sym_fun</c>: <c>T (T, T)</c> will be applied on each 
///     chunk with the identity value as the initial value. In the second phase,<c>_Sym_fun</c>: <c>T (T, T)</c> will be used to reduce sub results 
///     from the first phase.</para>
///     <para>For the third function overload, the iterator's value_type <c>T</c> can be different from the identity value type or the reduce result 
///     type <c>_Reduce_type</c>. The range reduce functor <c>_Range_fun: _Reduce_type (_Forward_iterator, _Forward_iterator, _Reduce_type)</c> is 
///     required in first phase reduce, and symmetric binary functor <c>_Sym_fun: _Reduce_type (_Reduce_type, _Reduce_type) </c> is required in the 
///     second phase reduce. Particularly, for the first phase reduce, functor <c>_Range_fun: 
///     _Reduce_type (_Forward_iterator, _Forward_iterator, _Reduce_type)</c> will be applied with the identity value as the initial value. In the 
///     second phase, <c>_Sym_fun: _Reduce_type (_Reduce_type, _Reduce_type) </c> will be applied to reduce the sub results from first phase, and the 
///     result will be returned as final result.</para>
///     <para>The user should not have any assumptions on chunk division.</para>
/// </remarks>
/**/
template<typename _Reduce_type, typename _Forward_iterator, typename _Range_reduce_fun, typename _Sym_reduce_fun>
inline _Reduce_type parallel_reduce(_Forward_iterator _Begin, _Forward_iterator _End, const _Reduce_type& _Identity, 
    const _Range_reduce_fun &_Range_fun, const _Sym_reduce_fun &_Sym_fun)
{
    typedef typename std::iterator_traits<_Forward_iterator>::value_type _Value_type;

    static_assert(!std::tr1::is_same<typename std::iterator_traits<_Forward_iterator>::iterator_category, std::input_iterator_tag>::value
        && !std::tr1::is_same<typename std::iterator_traits<_Forward_iterator>::iterator_category, std::output_iterator_tag>::value, 
        "iterator can not be input_iterator or output_iterator.");

    return _Parallel_reduce_impl(_Begin, _End,
        _Reduce_functor_helper<_Reduce_type, _Range_reduce_fun, 
        _Order_combinable<_Reduce_type, _Sym_reduce_fun>>(_Identity, _Range_fun, _Order_combinable<_Reduce_type, _Sym_reduce_fun>(_Sym_fun)),
        std::iterator_traits<_Forward_iterator>::iterator_category());
}

// Ordered serial combinable object
template<typename _Ty, typename _Sym_fun>
class _Order_combinable
{
public:
    // Only write once, limited contention will be caused
    struct _Bucket
    {
        // Allocate enough space in the Bucket to hold a value
        char _Value[(sizeof(_Ty) / sizeof(char))];
        _Bucket * _Next;

        _Bucket(_Bucket *_N)
        {
            _Next = _N;
        }

        void _Insert(_Bucket *_Item)
        {
            // No need to lock, only one thread will insert
            _Item->_Next = _Next;
            _Next = _Item;
        }

        // Construct value in bucket
        void _Put(const _Ty &_Cur)
        {
            new(reinterpret_cast<_Ty *>(&_Value)) _Ty(_Cur);
        }
    };

private:
    const _Sym_fun &_M_fun;
    size_t _M_number;
    _Bucket *_M_root;
    _Order_combinable &operator =(const _Order_combinable &other);

public:
    _Bucket *_Construct(_Bucket *_Par = 0)
    {
        _Bucket * _Ret = static_cast<_Bucket *>(Concurrency::Alloc(sizeof(_Bucket)));
        return new(_Ret)_Bucket(_Par);
    }

    _Order_combinable(const _Sym_fun &_Fun): _M_fun(_Fun)
    {
        _M_root = 0;
        _M_number = 0;
    }

    ~_Order_combinable()
    {
        while (_M_root)
        {
            _Bucket *_Cur = _M_root;
            _M_root = _M_root->_Next;
            reinterpret_cast<_Ty &>(_Cur->_Value).~_Ty();
            Concurrency::Free(_Cur);
        }
    }

    // Serially combine and release the list, return result
    _Ty _Serial_combine_release()
    {
        _Ty _Ret(reinterpret_cast<_Ty &>(_M_root->_Value));
        _Bucket *_Cur = _M_root;
        _M_root = _M_root->_Next;

        while (_M_root)
        {
            reinterpret_cast<_Ty &>(_Cur->_Value).~_Ty();
            Concurrency::Free(_Cur);
            _Cur = _M_root;
            _Ret = _M_fun(reinterpret_cast <_Ty &> (_Cur->_Value), _Ret);
            _M_root = _M_root->_Next;
        }

        reinterpret_cast<_Ty &>(_Cur->_Value).~_Ty();
        Concurrency::Free(_Cur);

        return _Ret;
    }

    // allocate a bucket and push back to the list
    _Bucket *_Unsafe_push_back()
    {
        return _M_root = _Construct(_M_root);
    }
};

// Implementation for the parallel reduce
template <typename _Forward_iterator, typename _Function>
typename _Function::_Reduce_type _Parallel_reduce_impl(_Forward_iterator _First, const _Forward_iterator& _Last, const _Function& _Func, 
    std::forward_iterator_tag)
{
    // Since this is a forward iterator, it is difficult to validate that _First comes before _Last, so
    // it is up to the user to provide valid range.
    if (_First != _Last)
    {
        task_group _Task_group;
        _Parallel_reduce_forward_executor(_First, _Last, _Func, _Task_group);
        _Task_group.wait();
        return _Func._Combinable._Serial_combine_release();
    }
    else
    {
        return _Func._Identity_value;
    }
}

template <typename _Random_iterator, typename _Function>
typename _Function::_Reduce_type _Parallel_reduce_impl(_Random_iterator _First, _Random_iterator _Last, const _Function& _Func, 
    std::random_access_iterator_tag)
{
    typedef _Parallel_reduce_fixed_worker<_Random_iterator, _Function> _Worker_class;

    // Special case for 0, 1 element
    if (_First >= _Last)
    {
        return _Func._Identity_value;
    }
    // Directly compute if size is too small
    else if (_Last - _First <= 1)
    {
        _Worker_class(_First, _Last, _Func)();
        return _Func._Combinable._Serial_combine_release();
    }
    else
    {
        // Use fixed ordered chunk partition to schedule works
        _Parallel_reduce_random_executor<_Worker_class>(_First, _Last, _Func);
        return _Func._Combinable._Serial_combine_release();
    }
}

// Helper function assemble all functors
template <typename _Reduce_type, typename _Sub_function, typename _Combinable_type>
struct _Reduce_functor_helper
{
    const _Sub_function &_Sub_fun;
    const _Reduce_type &_Identity_value;

    mutable _Combinable_type &_Combinable;

    typedef _Reduce_type _Reduce_type;
    typedef typename _Combinable_type::_Bucket Bucket_type;

    _Reduce_functor_helper(const _Reduce_type &_Identity, const _Sub_function &_Sub_fun, _Combinable_type &&comb):
    _Sub_fun(_Sub_fun), _Combinable(comb), _Identity_value(_Identity)
    {
    }

private:
    _Reduce_functor_helper &operator =(const _Reduce_functor_helper &other);
};

// All the code below is the worker without range stealing
template<typename _Forward_iterator, typename _Functor>
class _Parallel_reduce_fixed_worker
{
public:
    // The bucket allocation order will depend on the worker construction order
    _Parallel_reduce_fixed_worker(_Forward_iterator _Begin, _Forward_iterator _End, const _Functor &_Fun):
        _M_begin(_Begin), _M_end(_End), _M_fun(_Fun), _M_bucket(_M_fun._Combinable._Unsafe_push_back())
        {
        }

        void operator ()() const
        {
            _M_bucket->_Put(_M_fun._Sub_fun(_M_begin, _M_end, _M_fun._Identity_value));
        }

private:
    const _Functor &_M_fun;
    const _Forward_iterator _M_begin, _M_end;
    typename _Functor::Bucket_type * const _M_bucket;
    _Parallel_reduce_fixed_worker &operator =(const _Parallel_reduce_fixed_worker &other);
};

// the parallel worker executor for fixed iterator
// it will divide fixed number of chunks
// almost same as fixed parallel for, except keep the chunk dividing order
template <typename _Worker, typename _Random_iterator, typename _Function>
void _Parallel_reduce_random_executor(_Random_iterator _Begin, _Random_iterator _End, const _Function& _Fun)
{
    size_t _Cpu_num = static_cast<size_t>(CurrentScheduler::Get()->GetNumberOfVirtualProcessors()), _Size = _End - _Begin;

    structured_task_group _Tg;
    Concurrency::samples::details::_MallocaArrayHolder<task_handle<_Worker>> _Holder;
    task_handle<_Worker> *_Tasks = static_cast<task_handle<_Worker> *>(_malloca(sizeof(task_handle<_Worker>) * (_Cpu_num - 1)));
    _Holder._Initialize(_Tasks);

    size_t _Begin_index = 0;
    size_t _Step = _Size / _Cpu_num;
    size_t _NumRemaining = _Size - _Step * _Cpu_num;

    for(size_t _I = 0; _I < _Cpu_num - 1; _I++)
    {
        size_t _Next = _Begin_index + _Step;

        // Add remaining to each chunk
        if (_NumRemaining)
        {
            --_NumRemaining;
            ++_Next;
        }

        // New up a task_handle "in-place", in the array preallocated on the stack
        new (_Tasks + _I) task_handle<_Worker>(_Worker(_Begin + _Begin_index, _Begin + _Next, _Fun));
        _Holder._IncrementConstructedElemsCount();

        // Run each of the chunk _Tasks in parallel
        _Tg.run(_Tasks[_I]);
        _Begin_index = _Next;
    }

    task_handle<_Worker> _Tail(_Worker(_Begin + _Begin_index, _End, _Fun));
    _Tg.run_and_wait(_Tail);
}

// The parallel worker executor for forward iterators
// Divide chunks on the fly
template <typename _Forward_iterator, typename _Function, int _Default_worker_size, int _Default_chunk_size>
struct _Parallel_reduce_forward_executor_helper
{
    typedef _Parallel_reduce_fixed_worker<_Forward_iterator, _Function> _Worker_class;
    mutable std::auto_ptr<task_handle<_Worker_class>> _Workers;
    int _Worker_size;

    _Parallel_reduce_forward_executor_helper(_Forward_iterator &_First, _Forward_iterator _Last, const _Function& _Func):
    _Workers(static_cast<task_handle<_Worker_class> *>(Concurrency::Alloc(sizeof(task_handle<_Worker_class>) * _Default_worker_size)))
    {
        _Worker_size = 0;
        while (_Worker_size < _Default_worker_size && _First != _Last)
        {
            // Copy the range _Head
            _Forward_iterator _Head = _First;

            // Read from forward iterator
            for (size_t _I = 0; _I < _Default_chunk_size && _First != _Last; ++_I, ++_First) 
            {
                // Body is empty
            }

            // _First will be the end of current chunk
            new (_Workers.get() + _Worker_size++) task_handle<_Worker_class>(_Worker_class(_Head, _First, _Func));
        }
    }

    _Parallel_reduce_forward_executor_helper(const _Parallel_reduce_forward_executor_helper &_Other): 
    _Workers(_Other._Workers), _Worker_size(_Other._Worker_size)
    {
    }

    void operator ()() const
    {
        structured_task_group _Tg;
        for(int _I = 0; _I < _Worker_size; _I++)
        {
            _Tg.run(_Workers.get()[_I]);
        }
        _Tg.wait();
    }

    ~_Parallel_reduce_forward_executor_helper()
    {
        if (_Workers.get())
        {
            for (int _I = 0; _I < _Worker_size; _I++)
            {
                _Workers.get()[_I].~task_handle<_Worker_class>();
            }
            Concurrency::Free(_Workers.release());
        }
    }
};

template <typename _Forward_iterator, typename _Function>
void _Parallel_reduce_forward_executor(_Forward_iterator _First, _Forward_iterator _Last, const _Function& _Func, task_group& _Task_group)
{
    const static int _Internal_worker_number = 1024, _Default_chunk_size = 512;
    typedef _Parallel_reduce_fixed_worker<_Forward_iterator, _Function> _Worker_class;

    structured_task_group _Worker_group;
    Concurrency::samples::details::_MallocaArrayHolder<task_handle<_Worker_class>> _Holder;
    task_handle<_Worker_class>* _Workers = static_cast<task_handle<_Worker_class>*>(_malloca(_Internal_worker_number * sizeof(task_handle<_Worker_class>)));
    _Holder._Initialize(_Workers);

    // Start execution first
    int _Index = 0;
    while (_Index < _Internal_worker_number && _First != _Last)
    {
        // Copy the range _Head
        _Forward_iterator _Head = _First;

        // Read from forward iterator
        for (size_t _I = 0; _I < _Default_chunk_size && _First != _Last; ++_I, ++_First)
        {
            // Body is empty
        };

        // Create a new task, _First is range _End
        new (_Workers + _Index) task_handle<_Worker_class>(_Worker_class(_Head, _First, _Func));
        _Holder._IncrementConstructedElemsCount();
        _Worker_group.run(_Workers[_Index]);
        ++_Index;
    }

    // Divide and append the left
    while (_First != _Last)
    {
        _Task_group.run(_Parallel_reduce_forward_executor_helper<_Forward_iterator, _Function, _Internal_worker_number, _Default_chunk_size>(_First, _Last, _Func));
    }

    _Worker_group.wait();
}

#pragma warning(pop)


// Disable C4180: qualifier applied to function type has no meaning; ignored
// Warning fires for passing Foo function pointer to parallel_for instead of &Foo.
#pragma warning(push)
#pragma warning(disable: 4180)

//
// Dispatch the execution and handle the condition that all of the iterators are random access
//
template<typename _Any_input_traits, typename _Any_output_traits>
struct _Unary_transform_impl_helper
{
    template<typename _Input_iterator, typename _Output_iterator, typename _Unary_operator>
    static void _Parallel_transform_unary_impl(_Input_iterator _Begin, _Input_iterator _End, _Output_iterator& _Result, const _Unary_operator& _Unary_op)
    {
        task_group _Tg;
        _Parallel_transform_unary_impl2(_Begin, _End, _Result, _Unary_op, _Tg);
        _Tg.wait();
    }
};

template<>
struct _Unary_transform_impl_helper<std::random_access_iterator_tag, std::random_access_iterator_tag>
{
    template<typename _Random_input_iterator, typename _Random_output_iterator, typename _Unary_operator>
    static void _Parallel_transform_unary_impl(_Random_input_iterator _Begin, _Random_input_iterator _End, 
        _Random_output_iterator& _Result, const _Unary_operator& _Unary_op)
    {
        if (_Begin < _End)
        {
            Concurrency::_Parallel_for_impl(static_cast<size_t>(0), static_cast<size_t>(_End - _Begin), static_cast<size_t>(1), 
                [_Begin, &_Result, &_Unary_op](size_t _Index)
            {
                _Result[_Index] = _Unary_op(_Begin[_Index]);
            });
            _Result += _End - _Begin;
        }
    }
};

template<typename _Any_input_traits1, typename _Any_input_traits2, typename _Any_output_traits>
struct _Binary_transform_impl_helper
{

    template<typename _Input_iterator1, typename _Input_iterator2, typename _Output_iterator, typename _Binary_operator>
    static void _Parallel_transform_binary_impl(_Input_iterator1 _Begin1, _Input_iterator1 _End1, _Input_iterator2 _Begin2, 
        _Output_iterator& _Result, const _Binary_operator& _Binary_op)
    {
        task_group _Tg;
        _Parallel_transform_binary_impl2(_Begin1, _End1, _Begin2, _Result, _Binary_op, _Tg);
        _Tg.wait();
    }
};

template<>
struct _Binary_transform_impl_helper<std::random_access_iterator_tag, std::random_access_iterator_tag, std::random_access_iterator_tag>
{
    template<typename _Random_input_iterator1, typename _Random_input_iterator2, typename _Random_output_iterator, typename _Binary_operator>
    static void _Parallel_transform_binary_impl(_Random_input_iterator1 _Begin1, _Random_input_iterator1 _End1, 
        _Random_input_iterator2 _Begin2, _Random_output_iterator& _Result, const _Binary_operator& _Binary_op)
    {
        if (_Begin1 < _End1)
        {
            Concurrency::_Parallel_for_impl(static_cast<size_t>(0), static_cast<size_t>(_End1 - _Begin1), static_cast<size_t>(1), 
                [_Begin1, _Begin2, &_Result, &_Binary_op](size_t _Index)
            {
                _Result[_Index] = _Binary_op(_Begin1[_Index], _Begin2[_Index]);
            });
            _Result += _End1 - _Begin1;
        }
    }
};

//
// The implementation for at least one of the iterator is forward iterator
//
template <typename _Forward_iterator, typename _Iterator_kind>
class _Iterator_helper
{
public:
    static const size_t _Size = 1024;
    typedef typename std::iterator_traits<_Forward_iterator>::value_type value_type;

    _Iterator_helper()
    {
        static_assert(!std::is_same<_Iterator_kind, std::input_iterator_tag>::value 
            && !std::is_same<_Iterator_kind, std::output_iterator_tag>::value,
            "iterator can not be input_iterator or output_iterator.");
    }

    size_t _Populate(_Forward_iterator& _First, _Forward_iterator _Last)
    {
        size_t _Length = 0;
        static_assert(std::is_lvalue_reference<decltype(*_First)>::value, "lvalue required for forward iterator operator *");

        for (size_t _Index=0; (_Index < _Size) && (_First != _Last); _Index++)
        {
            // We only support l-value here, so it's safe
            _M_element_array[_Length++] = &(*_First++);
        }

        return _Length;
    }

    void _Populate(_Forward_iterator& _First, size_t _Length)
    {
        for (size_t _Index=0; _Index < _Length; _Index++)
        {
            _M_element_array[_Index] = &(*_First++);
        }
    }

    void _Store(const value_type& _Elem, size_t _Index) const
    {
        *(_M_element_array[_Index]) = _Elem;
    }

    typename std::iterator_traits<_Forward_iterator>::reference _Load(size_t _Index) const
    {
        return *(_M_element_array[_Index]);
    }

private:
    typename std::iterator_traits<_Forward_iterator>::pointer _M_element_array[_Size];
};

template <typename _Random_iterator>
class _Iterator_helper<_Random_iterator, std::random_access_iterator_tag>
{
public:
    static const size_t _Size = 1024;
    typedef typename std::iterator_traits<_Random_iterator>::value_type value_type;

    _Iterator_helper()
    {
    }

    size_t _Populate(_Random_iterator& _First, _Random_iterator _Last)
    {
        std::iterator_traits<_Random_iterator>::difference_type _Range = _Last - _First;
        std::iterator_traits<_Random_iterator>::difference_type _Sized = _Size;
        _M_first = _First;

        if (_Range > _Sized)
        {
            _First += _Size;
            return _Size;
        }
        else
        {
            _First += _Range;
            return static_cast<size_t>(_Range);
        }
    }

    void _Populate(_Random_iterator& _First, size_t _Length)
    {
        _M_first = _First;
        _First += _Length;
    }

    void _Store(const value_type& _Elem, size_t _Index) const
    {
        _M_first[_Index] = _Elem;
    }

    typename std::iterator_traits<_Random_iterator>::reference _Load(size_t _Index) const
    {
        // We only support l-value here
        return _M_first[_Index];
    }

private:
    _Random_iterator _M_first;
};

template <typename _Input_iterator1, typename _Input_iterator2, typename _Output_iterator, typename _Binary_operator>
class _Parallel_transform_binary_helper
{
public:
    _Parallel_transform_binary_helper(_Input_iterator1& _First1, _Input_iterator1 _Last1, _Input_iterator2& _First2, 
        _Output_iterator& _Result, const _Binary_operator& _Binary_op) :
    _M_binary_op(_Binary_op), _M_len(0)
    {
        _M_len = _M_input_helper1._Populate(_First1, _Last1);
        _M_input_helper2._Populate(_First2, _M_len);
        _M_output_helper._Populate(_Result, _M_len);
    }

    void operator()() const
    {
        // Invoke parallel_for on the batched up array of elements
        Concurrency::_Parallel_for_impl(static_cast<size_t>(0), _M_len, static_cast<size_t>(1),
            [this] (size_t _Index)
        {
            _M_output_helper._Store(_M_binary_op(_M_input_helper1._Load(_Index), _M_input_helper2._Load(_Index)), _Index);
        });
    }

private:

    _Iterator_helper<_Input_iterator1, typename std::iterator_traits<_Input_iterator1>::iterator_category>   _M_input_helper1;
    _Iterator_helper<_Input_iterator2, typename std::iterator_traits<_Input_iterator2>::iterator_category>   _M_input_helper2;
    _Iterator_helper<_Output_iterator, typename std::iterator_traits<_Output_iterator>::iterator_category>   _M_output_helper;
    const _Binary_operator&                                                                                  _M_binary_op;
    size_t                                                                                                   _M_len;

    _Parallel_transform_binary_helper const & operator=(_Parallel_transform_binary_helper const&);    // no assignment operator
};

template <typename _Input_iterator1, typename _Input_iterator2, typename _Output_iterator, typename _Binary_operator>
void _Parallel_transform_binary_impl2(_Input_iterator1 _First1, _Input_iterator1 _Last1, _Input_iterator2 _First2, _Output_iterator &_Result,
    const _Binary_operator& _Binary_op, task_group& _Tg)
{
    // This functor will be copied on the heap and will execute the chunk in parallel
    {
        _Parallel_transform_binary_helper<_Input_iterator1, _Input_iterator2, _Output_iterator, _Binary_operator> functor(_First1, _Last1, _First2, _Result, _Binary_op);
        _Tg.run(functor);
    }

    // If there is a tail, push the tail
    if (_First1 != _Last1)
    {
        _Tg.run(
            [=, &_Result, &_Binary_op, &_Tg]
        {
            _Parallel_transform_binary_impl2(_First1, _Last1, _First2, _Result, _Binary_op, _Tg);
        });
    }
}

template <typename _Input_iterator, typename _Output_iterator, typename _Unary_operator>
class _Parallel_transform_unary_helper
{
public:
    _Parallel_transform_unary_helper(_Input_iterator& _First, _Input_iterator _Last, _Output_iterator &_Result, const _Unary_operator& _Unary_op) :
        _M_unary_op(_Unary_op), _M_len(0)
        {
            _M_len = _M_input_helper._Populate(_First, _Last);
            _M_output_helper._Populate(_Result, _M_len);
        }

        void operator()() const
        {
            // Invoke parallel_for on the batched up array of elements
            Concurrency::_Parallel_for_impl(static_cast<size_t>(0), _M_len, static_cast<size_t>(1),
                [this] (size_t _Index)
            {
                _M_output_helper._Store(_M_unary_op(_M_input_helper._Load(_Index)), _Index);
            });
        }

private:

    _Iterator_helper<_Input_iterator, typename std::iterator_traits<_Input_iterator>::iterator_category>   _M_input_helper;
    _Iterator_helper<_Output_iterator, typename std::iterator_traits<_Output_iterator>::iterator_category> _M_output_helper;
    const _Unary_operator&                                                                                     _M_unary_op;
    size_t                                                                                              _M_len;

    _Parallel_transform_unary_helper const & operator=(_Parallel_transform_unary_helper const&);    // no assignment operator
};

template <typename _Input_iterator, typename _Output_iterator, typename _Unary_operator>
void _Parallel_transform_unary_impl2(_Input_iterator _First, _Input_iterator _Last, _Output_iterator &_Result, 
    const _Unary_operator& _Unary_op, task_group& _Tg)
{
    // This functor will be copied on the heap and will execute the chunk in parallel
    {
        _Parallel_transform_unary_helper<_Input_iterator, _Output_iterator, _Unary_operator> functor(_First, _Last, _Result, _Unary_op);
        _Tg.run(functor);
    }

    // If there is a tail, push the tail
    if (_First != _Last)
    {
        _Tg.run(
            [=, &_Result, &_Unary_op, &_Tg]
        {
            _Parallel_transform_unary_impl2(_First, _Last, _Result, _Unary_op, _Tg);
        });
    }
}

/// <summary>
///     This template function is semantically equivalent to <c>std::transform</c>, except that
///     the iteration is done in parallel and ordering is unspecified. 
/// </summary>
/// <typeparam name="_Input_iterator">
///     The type of input iterator.
/// </typeparam>
/// <typeparam name="_Output_iterator">
///     The type of output iterator.
/// </typeparam>
/// <typeparam name="_Unary_operator">
///     The unary functor type to be executed on each iteration.
/// </typeparam>
/// <param name="_First">
///     The position of the first element to be included in input iterator.
/// </param>
/// <param name="_Last">
///     The position of the first element not to be included in input iterator.
/// </param>
/// <param name="_Result">
///     The position of first element in output iterator.
/// </param>
/// <param name="_Unary_op">
///     Function object to be executed on each iteration, a unary operator.
/// </param>
/// <returns>
///     The position after last result element in output iterator.
/// </returns>
/// <remarks>
///     <para>For the first overloading function, function argument <c> _Unary_op </c> must support 
///     <c>operator()(T)</c> where <c>T</c> is the item type of the range being iterated over.</para>
///     <para>For the second overloading function, function argument <c> _Binary_op </c> must support 
///     <c>operator()(T, U)</c> where <c>T</c>, <c>U</c> are value_types of the two input iterators.</para>
///     <para>For more information, see <see cref="Parallel Algorithms"/>.</para>
/// </remarks>
/**/
template <typename _Input_iterator, typename _Output_iterator, typename _Unary_operator>
_Output_iterator parallel_transform(_Input_iterator _First, _Input_iterator _Last, _Output_iterator _Result, const _Unary_operator& _Unary_op)
{
    typedef std::iterator_traits<_Input_iterator>::iterator_category _Input_iterator_type;
    typedef std::iterator_traits<_Output_iterator>::iterator_category _Output_iterator_type;

    if (_First != _Last)
    {
        _Unary_transform_impl_helper<_Input_iterator_type, _Output_iterator_type>
            ::_Parallel_transform_unary_impl(_First, _Last, _Result, _Unary_op);
    }

    return _Result;
}

/// <summary>
///     This template function is semantically equivalent to <c>std::transform</c>, except that
///     the iteration is done in parallel and ordering is unspecified.
/// </summary>
/// <typeparam name="_Input_iterator1">
///     The type of first input iterator.
/// </typeparam>
/// <typeparam name="_Input_iterator2">
///     The type of second input iterator.
/// </typeparam>
/// <typeparam name="_Output_iterator">
///     The type of output iterator.
/// </typeparam>
/// <typeparam name="_Binary_operator">
///     The binary functor type to be executed on each iteration.
/// </typeparam>
/// <param name="_First1">
///     The position of the first element to be included in first input iterator.
/// </param>
/// <param name="_Last1">
///     The position of the first element not to be included in first input iterator.
/// </param>
/// <param name="_First2">
///     The position of the first element to be included in second input iterator.
/// </param>
/// <param name="_Result">
///     The position of first element in output iterator.
/// </param>
/// <param name="_Binary_op">
///     Function object to be executed on each iteration, a binary operator.
/// </param>
/// <returns>
///     The position after the last result element in output iterator.
/// </returns>
/// <remarks>
///     <para>For the first overloading function, function argument <c> _Unary_op </c> must support 
///     <c>operator()(T)</c> where <c>T</c> is the item type of the range being iterated over.</para>
///     <para>For the second overloading function, function argument <c> _Binary_op </c> must support 
///     <c>operator()(T, U)</c> where <c>T</c>, <c>U</c> are value_types of the two input iterators.</para>
///     <para>For more information, see <see cref="Parallel Algorithms"/>.</para>
/// </remarks>
/**/
template <typename _Input_iterator1, typename _Input_iterator2, typename _Output_iterator, typename _Binary_operator>
_Output_iterator parallel_transform(_Input_iterator1 _First1, _Input_iterator1 _Last1, _Input_iterator2 _First2, 
    _Output_iterator _Result, const _Binary_operator& _Binary_op)
{
    typedef std::iterator_traits<_Input_iterator1>::iterator_category _Input_iterator_type1;
    typedef std::iterator_traits<_Input_iterator2>::iterator_category _Input_iterator_type2;
    typedef std::iterator_traits<_Output_iterator>::iterator_category _Output_iterator_type;

    if (_First1 != _Last1)
    {
        _Binary_transform_impl_helper<_Input_iterator_type1, _Input_iterator_type2, _Output_iterator_type>
            ::_Parallel_transform_binary_impl(_First1, _Last1, _First2, _Result, _Binary_op);
    }

    return _Result;
}

#pragma warning(pop)


namespace details
{
    /// <summary>
    ///     An implementation of parallel partial sum that splits the work into smallest possible chunks
    ///     using recursion. This is efficient when the binary operation is dependent on the values as work
    ///     stealing would provide automatic load balancing
    /// </summary>
    /// <param name="out_randomIterator">
    ///     Type of the iterator to the container that will hold the output values
    /// </param>
    /// <param name="size_type">
    ///     Type of the size of the container
    /// </param>
    /// <param name="BinaryOperator">
    ///     The binary operator that computes the sum of two values
    /// </param>
    template <typename out_randomIterator, typename size_type, typename BinaryOperator>
    void parallel_partial_sum_impl_recursive(out_randomIterator begin, size_type size, size_type skip, BinaryOperator sumFunction)
    {
        if (size <= 1)
        {
            return;
        }

        size_type halfLength = size / 2;

        // pairwise combine
        parallel_for<size_t>(0, halfLength, [begin, skip, sumFunction](size_type index)
        {
            out_randomIterator loc = begin + (index * 2 * skip);
            *(loc + skip) = sumFunction(*loc, *(loc + skip));
        });

        // Recursively prefix scan the pairwise computations
        parallel_partial_sum_impl_recursive(begin + skip, halfLength, skip*2, sumFunction);

        // Pairwise combine
        parallel_for<size_t>(0, (size % 2 == 0) ? halfLength - 1 : halfLength, [begin, skip, sumFunction](size_type index)
        {
            out_randomIterator loc = begin + (index * 2 * skip) + skip;
            *(loc + skip) = sumFunction(*loc, *(loc + skip));
        });
    }

    /// <summary>
    ///     An implementation of parallel partial sum that splits the work into fixed chunks.
    ///     This is efficient when the binary operation is relatively fast and independent of the values.
    /// </summary>
    /// <param name="input_iterator">
    ///     Type of the iterator to the container that holds the input values
    /// </param>
    /// <param name="out_randomIterator">
    ///     Type of the iterator to the container that will hold the output values
    /// </param>
    /// <param name="BinaryOperator">
    ///     The binary operator that computes the sum of two values
    /// </param>
    template <typename in_randomIterator, typename out_randomIterator, typename BinaryOperator>
    void parallel_partial_sum_impl(in_randomIterator begin, in_randomIterator end, out_randomIterator result, BinaryOperator sumFunction)
    {
        typedef typename std::iterator_traits<out_randomIterator>::value_type value_type;
        typedef std::iterator_traits<in_randomIterator>::difference_type size_type;

        // We will use the number of virtual processors to compute the number of chunks
        const int oversubscription = 2;
        size_type numChunks = static_cast<size_type>(CurrentScheduler::Get()->GetNumberOfVirtualProcessors() * oversubscription);
        size_type size = end - begin;

        if (size < numChunks)
        {
            // Not enough iterations to warrant parallel computation
            std::partial_sum(begin, end, result, sumFunction);            
            return;
        }

        std::vector<value_type> tempSum(numChunks);

        // Determine the range for each chunk
        size_type chunkSize = size / numChunks;
        _ASSERTE(chunkSize >= 1);

        parallel_for(size_type(0), numChunks, [begin, end, result, sumFunction, chunkSize, numChunks, &tempSum](size_type index)
        {
            // Compute partial sum for this chunk.

            in_randomIterator start = begin + (index * chunkSize);
            in_randomIterator last = (index == (numChunks - 1)) ? end : start + chunkSize;

            out_randomIterator resultStart = result + (index * chunkSize);
            out_randomIterator resultEnd = resultStart + (last - start);

            std::partial_sum(start, last, resultStart, sumFunction);

            // Store the upper bounds into intermediate buffer.
            tempSum[index] = *(resultEnd-1);
        });

        // Compute partial sum on the intermediate buffer
        std::partial_sum(tempSum.begin(), tempSum.end(), tempSum.begin(), sumFunction);

        // The first chunk is already correct. For the other chunks propagate the partial
        // sum from the intermediate buffer
        parallel_for(size_type(1), numChunks, [begin, end, result, sumFunction, chunkSize, numChunks, tempSum](size_type index)
        {
            // Add the partial sum from the previous chunk to all the
            // elements in this chunk
            in_randomIterator start = begin + (index * chunkSize);
            in_randomIterator last = (index == (numChunks - 1)) ? end : start + chunkSize;

            out_randomIterator resultStart = result + (index * chunkSize);
            out_randomIterator resultEnd = resultStart + (last - start);

            for (out_randomIterator iter = resultStart; iter < resultEnd; ++iter)
            {
                *iter = sumFunction(*iter, tempSum[index-1]);
            }
        });
    }
}
/// <summary>
///     Compute inclusive partial sum
/// </summary>
/// <param name="in_randomIterator">
///     Type of the iterator to the container that holds the input values
/// </param>
/// <param name="out_randomIterator">
///     Type of the iterator to the container that will hold the output values
/// </param>
/// <param name="BinaryOperator">
///     The binary operator that computes the sum of two values
/// </param>
template <typename in_randomIterator, typename out_randomIterator, typename BinaryOperator>
void parallel_partial_sum_fixed(in_randomIterator begin, in_randomIterator end, out_randomIterator result, BinaryOperator sumFunction)
{
    return details::parallel_partial_sum_impl(begin, end, result, sumFunction);
}

/// <summary>
///     Compute inclusive partial sum
/// </summary>
/// <param name="in_randomIterator">
///     Type of the iterator to the container that holds the input values
/// </param>
/// <param name="out_randomIterator">
///     Type of the iterator to the container that will hold the output values
/// </param>
/// <param name="BinaryOperator">
///     The binary operator that computes the sum of two values
/// </param>
template <typename in_randomIterator, typename out_randomIterator, typename BinaryOperator>
void parallel_partial_sum(in_randomIterator begin, in_randomIterator end, out_randomIterator result, BinaryOperator sumFunction)
{
    typedef std::iterator_traits<in_randomIterator>::difference_type size_type;

    // Copy input to output first
    out_randomIterator out = result;
    for (in_randomIterator in = begin; in < end; ++in)
    {
        *out = *in;
        ++out;                
    }

    // Perform in place partial sum
    return details::parallel_partial_sum_impl_recursive(result, (end-begin), size_type(1), sumFunction);
}

/// <summary>
///     Compute inclusive partial sum. This overload performs the operation in place.
/// </summary>
/// <param name="in_randomIterator">
///     Type of the iterator to the container that holds the input values
/// </param>
/// <param name="BinaryOperator">
///     The binary operator that computes the sum of two values
/// </param>
template <typename in_randomIterator, typename BinaryOperator>
void parallel_partial_sum(in_randomIterator begin, in_randomIterator end, BinaryOperator sumFunction)
{
    typedef std::iterator_traits<in_randomIterator>::difference_type size_type;
    return details::parallel_partial_sum_impl_recursive(begin, (end-begin), size_type(1), sumFunction);
}
/// <summary>
///  merge two sorted sequences in parallel
/// </summary>
/// <param name="ran_it">
///     Type of the iterator to the container that holds the input values
/// </param>
/// <param name="out_it">
///     Type of the iterator to the container that holds the output values
/// </param>
template<typename ran_it, typename out_it>
inline void parallel_merge(ran_it first1, ran_it last1, ran_it first2, ran_it last2, out_it out)
{
    int sequence1Length = last1 - first1;
    int sequence2Length = last2 - first2;

    if (min(sequence2Length, sequence1Length ) < 512)
    {
        std::merge( first1, last1, first2, last2, out);
        return;
    }

    if( sequence1Length < sequence2Length )
    {
        std::swap(first1, first2);
        std::swap(last1, last2);
    }

    ran_it mid1 = first1 + (last1-first1)/2;

    ran_it m2 = std::lower_bound( first2, last2, *mid1 );

    structured_task_group tasks;

    auto t = make_task([&](){parallel_merge(first1, mid1, first2, m2, out);});
    tasks.run(t);

    out_it out_right = out;
    out_right += (mid1 - first1) + (m2 - first2);

    tasks.run_and_wait([&](){parallel_merge(mid1, last1, m2, last2, out_right );});
}

#pragma push_macro("_MAX_NUM_TASKS_PER_CORE")
#pragma push_macro("_FINE_GRAIN_CHUNK_SIZE")
#pragma push_macro("_SORT_MAX_RECURSION_DEPTH")

// This number is used to control dynamic task splitting
// The ideal chunk (task) division is that the number of cores is equal to the number of tasks, but it will 
// perform very poorly when tasks are not balanced. The simple solution is to allocate more tasks than number 
// of cores.  _MAX_NUM_TASKS_PER_CORE provides a maximum number of tasks that will be allocated per core.
// If this number is too small, the load balancing problem will affect efficiency very seriously, especially
// when the compare operation is expensive.
//
// Note that this number is a maximum number -- the dynamic partition system will reduce the number of partitions
// per core based on the dynamic load. If all cores are very busy, the number of partitions will shrink to 
// reduce the scheduler overhead.
//
// Initially, the total tasks(chunks) number of partitions "_Div_num" will be: core number * _MAX_NUM_TASKS_PER_CORE.
// The _Div_num will be divided by 2 after each task splitting. There are two special numbers for _Div_num:
//     1. When _Div_num reaches the point that _Div_num < _MAX_NUM_TASKS_PER_CORE, it means we have split more tasks than cores.
//     2. When _Div_num reaches the point that _Div_num <= 1, it means stop splitting more tasks and begin sorting serially.
#define _MAX_NUM_TASKS_PER_CORE 1024

// This is a number mainly is used to control the sampling and dynamic task splitting strategies.
// If the user configurable minimal divisible chunk size (default is 2048) is smaller than FINE_GRAIN_CHUNK_SIZE,
// the random sampling algorithm for quicksort will enter fine-grained mode, and take a strategy that reduces the sampling 
// overhead. Also, the dynamic task splitting will enter fine-grained mode, which will split as many tasks as possible.
#define _FINE_GRAIN_CHUNK_SIZE 512

// This is the maximum depth that the quicksort will be called recursively.  If we allow too far, a stack overflow may occur.
#define _SORT_MAX_RECURSION_DEPTH 64

template<typename _Random_iterator, typename _Function>
inline size_t _Median_of_three(const _Random_iterator &_Begin, size_t _A, size_t _B, size_t _C, const _Function &_Func, bool &_Potentially_equal)
{
    _Potentially_equal = false;
    if (_Func(_Begin[_A], _Begin[_B]))
    {
        if (_Func(_Begin[_A], _Begin[_C]))
        {
            return _Func(_Begin[_B], _Begin[_C]) ? _B : _C;
        }
        else
        {
            return _A;
        }
    }
    else
    {
        if (_Func(_Begin[_B], _Begin[_C]))
        {
            return _Func(_Begin[_A], _Begin[_C]) ? _A : _C;
        }
        else
        {
            _Potentially_equal = true;
            return _B;
        }
    }
}

template<typename _Random_iterator, typename _Function>
inline size_t _Median_of_nine(const _Random_iterator &_Begin, size_t _Size, const _Function &_Func, bool &_Potentially_equal)
{
    size_t _Offset = _Size / 8;
    size_t _A = _Median_of_three(_Begin, 0, _Offset, _Offset * 2, _Func, _Potentially_equal),
        _B = _Median_of_three(_Begin, _Offset * 3, _Offset * 4, _Offset * 5, _Func, _Potentially_equal),
        _C = _Median_of_three(_Begin, _Offset * 6, _Offset * 7, _Size - 1, _Func, _Potentially_equal);
    _B = _Median_of_three(_Begin, _A, _B, _C, _Func, _Potentially_equal);

    if (_Potentially_equal)
    {
        _Potentially_equal = !_Func(_Begin[_C], _Begin[_A]);
    }

    return _B;
}

// _Potentially_equal means that potentially all the values in the buffer are equal to the pivot value
template<typename _Random_iterator, typename _Function>
inline size_t _Select_median_pivot(const _Random_iterator &_Begin, size_t _Size, const _Function &_Func, const size_t _Chunk_size, bool &_Potentially_equal)
{
    // Base on different chunk size, apply different sampling optimization
    if (_Chunk_size < _FINE_GRAIN_CHUNK_SIZE && _Size <= max(_Chunk_size * 4, static_cast<size_t>(15)))
    {
        bool _Never_care_equal;
        return _Median_of_three(_Begin, 0, _Size / 2, _Size - 1, _Func, _Never_care_equal);
    }
    else
    {
        return _Median_of_nine(_Begin, _Size, _Func, _Potentially_equal);
    }
}

// Find out two middle points for two sorted arrays by binary search so that the number of total elements on the left part of two middle points is equal
// to the number of total elements on the right part of two sorted arrays and all elements on the left part is smaller than right part. 
template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Function>
size_t _Search_mid_point(const _Random_iterator &_Begin1, size_t &_Len1, const _Random_buffer_iterator &_Begin2, size_t &_Len2, const _Function &_Func)
{
    size_t _Len = (_Len1 + _Len2) / 2, _Index1 = 0, _Index2 = 0;

    while (_Index1 < _Len1 && _Index2 < _Len2)
    {
        size_t _Mid1 = (_Index1 + _Len1) / 2, _Mid2 = (_Index2 + _Len2) / 2;
        if (_Func(_Begin1[_Mid1], _Begin2[_Mid2]))
        {
            if (_Mid1 + _Mid2 < _Len)
            {
                _Index1 = _Mid1 + 1;
            }
            else
            {
                _Len2 = _Mid2;
            }
        }
        else
        {
            if (_Mid1 + _Mid2 < _Len)
            {
                _Index2 = _Mid2 + 1;
            }
            else
            {
                _Len1 = _Mid1;
            }
        }
    }

    if (_Index1 == _Len1)
    {
        _Len2 = _Len - _Len1;
    }
    else
    {
        _Len1 = _Len - _Len2;
    }

    return _Len;
}

// "move" operation is applied between buffers
template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Random_output_iterator, typename _Function>
void _Merge_chunks(_Random_iterator _Begin1, const _Random_iterator &_End1, _Random_buffer_iterator _Begin2, const _Random_buffer_iterator &_End2, 
    _Random_output_iterator _Output, const _Function &_Func)
{
    while (_Begin1 != _End1 && _Begin2 != _End2)
    {
        if (_Func(*_Begin1, *_Begin2))
        {
            *_Output++ = std::move(*_Begin1++);
        }
        else
        {
            *_Output++ = std::move(*_Begin2++);
        }
    }

    if (_Begin1 != _End1)
    {
        std::_Move(_Begin1, _End1, _Output);
    }
    else if (_Begin2 != _End2)
    {
        std::_Move(_Begin2, _End2, _Output);
    }
}

// _Div_num of threads(tasks) merge two chunks in parallel, _Div_num should be power of 2, if not, the largest power of 2 that is
// smaller than _Div_num will be used
template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Random_output_iterator, typename _Function>
void _Parallel_merge(_Random_iterator _Begin1, size_t _Len1, _Random_buffer_iterator _Begin2, size_t _Len2, _Random_output_iterator _Output, 
    const _Function &_Func, size_t _Div_num)
{
    // Turn to serial merge or continue splitting chunks base on "_Div_num"
    if (_Div_num <= 1 || (_Len1 <= 1 && _Len2 <= 1))
    {
        _Merge_chunks(_Begin1, _Begin1 + _Len1, _Begin2, _Begin2 + _Len2, _Output, _Func);
    }
    else
    {
        size_t _Mid_len1 = _Len1, _Mid_len2 = _Len2;
        size_t _Mid = _Search_mid_point(_Begin1, _Mid_len1, _Begin2, _Mid_len2, _Func);

        structured_task_group _Tg;
        auto _Handle = make_task([&] 
        {
            _Parallel_merge(_Begin1, _Mid_len1, _Begin2, _Mid_len2, _Output, _Func, _Div_num / 2); 
        });
        _Tg.run(_Handle);

        _Parallel_merge(_Begin1 + _Mid_len1, _Len1 - _Mid_len1, _Begin2 + _Mid_len2, _Len2 - _Mid_len2, _Output + _Mid, _Func, _Div_num / 2);

        _Tg.wait();
    }
}

// Return current sorting byte from key 
template<typename _Ty, typename _Function>
inline size_t _Radix_key(const _Ty& _Val, size_t _Radix, _Function _Proj_func)
{
    return static_cast<size_t>(_Proj_func(_Val) >> static_cast<int>(8 * _Radix) & 255);
}

// One pass of radix sort
template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Function>
void _Integer_radix_pass(const _Random_iterator &_Begin, size_t _Size, const _Random_buffer_iterator &_Output, size_t _Radix, _Function _Proj_func)
{
    if (!_Size)
    {
        return;
    }

    size_t _Pos[256] = {0};

    for (size_t _I = 0; _I < _Size; _I++)
    {
        ++_Pos[_Radix_key(_Begin[_I], _Radix, _Proj_func)];
    }

    for (size_t _I = 1; _I < 256; _I++)
    {
        _Pos[_I] += _Pos[_I - 1];
    }

    // _Size > 0
    for (size_t _I = _Size - 1; _I != 0; _I--)
    {
        _Output[--_Pos[_Radix_key(_Begin[_I], _Radix, _Proj_func)]] = std::move(_Begin[_I]);
    }

    _Output[--_Pos[_Radix_key(_Begin[0], _Radix, _Proj_func)]] = std::move(_Begin[0]);
}

// Serial least-significant-byte radix sort, it will sort base on last "_Radix" number of bytes
template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Function>
void _Integer_radix_sort(const _Random_iterator &_Begin, size_t _Size, const _Random_buffer_iterator &_Output, 
    size_t _Radix, _Function _Proj_func, size_t _Deep = 0)
{
    size_t _Cur_radix = 0;
    if (_Size == 0)
    {
        return;
    }

    while (_Cur_radix < _Radix)
    {
        _Integer_radix_pass(_Begin, _Size, _Output, _Cur_radix++, _Proj_func);
        _Integer_radix_pass(_Output, _Size, _Begin, _Cur_radix++, _Proj_func);
    }

    if (_Cur_radix == _Radix)
    {
        _Integer_radix_pass(_Begin, _Size, _Output, _Cur_radix++, _Proj_func);
    }

    // if odd round is passed, then move result back to input buffer
    if (_Deep + _Radix + 1 & 1)
    {
        if (_Radix + 1 & 1)
        {
            std::_Move(_Output, _Output + _Size, _Begin);
        }
        else
        {
            std::_Move(_Begin, _Begin + _Size, _Output);
        }
    }
}

// Parallel most-significant-byte _Radix sort.
// In the end, it will turn to serial least-significant-byte radix sort
template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Function>
void _Parallel_integer_radix_sort(const _Random_iterator &_Begin, size_t _Size, const _Random_buffer_iterator &_Output, 
    size_t _Radix, _Function _Proj_func, const size_t _Chunk_size, size_t _Deep = 0)
{
    // If the chunk _Size is too small, then turn to serial least-significant-byte radix sort
    if (_Size <= _Chunk_size || _Radix < 1)
    {
        return _Integer_radix_sort(_Begin, _Size, _Output, _Radix, _Proj_func, _Deep);
    }

    size_t _Threads_num = Concurrency::CurrentScheduler::Get()->GetNumberOfVirtualProcessors();
    size_t _Buffer_size = sizeof(size_t) * 256 * _Threads_num;
    size_t _Step = _Size / _Threads_num;
    size_t _Remain = _Size % _Threads_num;

    Concurrency::samples::details::_MallocaArrayHolder<size_t [256]> _Holder;
    size_t (*_Chunks)[256] = static_cast<size_t (*)[256]>(_malloca(_Buffer_size));
    _Holder._Initialize(_Chunks);

    memset(_Chunks, 0, _Buffer_size);

    // Our purpose is to map unsorted data in buffer "_Begin" to buffer "_Output" so that all elements who have the same 
    // byte value in the "_Radix" position will be grouped together in the buffer "_Output"
    //
    // Serial version:
    // To understand this algorithm, first consider a serial version. In following example, we treat 1 digit as 1 byte, so we have a 
    // total of 10 elements for each digit instead of 256 elements in each byte. Let's suppose "_Radix" == 1 (right most is 0), and:
    //
    //      begin:  [ 32 | 62 | 21 | 43 | 55 | 43 | 23 | 44 ]
    // 
    // We want to divide the output buffer "_Output" into 10 chunks, and each the element in the "_Begin" buffer should be mapped into 
    // the proper destination chunk based on its current digit (byte) indicated by "_Radix"
    //
    // Because "_Radix" == 1, after a pass of this function, the chunks in the "_Output" should look like:
    //
    //      buffer: [   |   | 21 23 | 32 | 43 43 44 | 55 | 62 |   |   |   ]
    //                0   1     2      3      4        5    6   7   8   9
    // 
    // The difficulty is determining where to insert values into the "_Output" to get the above result. The way to get the
    // start position of each chunk of the buffer is:
    //      1. Count the number of elements for each chunk (in above example, chunk0 is 0, chunk1 is 0, chunk2 is 2, chunk3 is 1 ...
    //      2. Make a partial sum for these chunks( in above example,  we will get chunk0=chunk0=0, chunk1=chunk0+chunk1=0, 
    //         chunk2=chunk0+chunk1+chunk2=2, chunk3=chunk0+chunk1+chunk2+chunk3=3
    //
    // After these steps, we will get the end position of each chunk in the "_Output". The begin position of each chunk will be the end 
    // point of last chunk (begin point is close but the end point is open). After that,  we can scan the original array again and directly 
    // put elements from original buffer "_Begin" into specified chunk on buffer "_Output".
    // Finally, we invoke _parallel_integer_radix_sort in parallel for each chunk and sort them in parallel based on the next digit (byte).
    // Because this is a STABLE sort algorithm, if two numbers has same key value on this byte (digit), their original order should be kept.
    //
    // Parallel version:
    // Almost the same as the serial version, the differences are:
    //      1. The count for each chunk is executed in parallel, and each thread will count one segment of the input buffer "_Begin".
    //         The count result will be separately stored in their own chunk size counting arrays so we have a total of threads-number 
    //         of chunk count arrays.
    //         For example, we may have chunk00, chunk01, ..., chunk09 for first thread, chunk10, chunk11, ..., chunk19 for second thread, ...
    //      2. The partial sum should be executed across these chunk counting arrays that belong to different threads, instead of just 
    //         making a partial sum in one counting array. 
    //         This is because we need to put values from different segments into one final buffer, and the absolute buffer position for 
    //         each chunkXX is needed.
    //      3. Make a parallel scan for original buffer again, and move numbers in parallel into the corresponding chunk on each buffer based
    //         on these threads' chunk size counters.

    // Count in parallel and separately save their local results without reducing
    Concurrency::parallel_for(static_cast<size_t>(0), _Threads_num, [=](size_t _Index) 
    {
        size_t _Beg_index, _End_index;

        // Calculate the segment position
        if (_Index < _Remain)
        {
            _Beg_index = _Index * (_Step + 1);
            _End_index = _Beg_index + (_Step + 1);
        }
        else
        {
            _Beg_index = _Remain * (_Step + 1) + (_Index - _Remain) * _Step;
            _End_index = _Beg_index + _Step;
        }

        // Do a counting
        while (_Beg_index != _End_index)
        {
            ++_Chunks[_Index][_Radix_key(_Begin[_Beg_index++], _Radix, _Proj_func)];
        }
    });

    int _Index = -1, _Count = 0;

    // Partial sum cross different threads' chunk counters
    for (int _I = 0; _I < 256; _I++)
    {
        size_t _Last = _I ? _Chunks[_Threads_num - 1][_I - 1] : 0;
        _Chunks[0][_I] += _Last;

        for (size_t _J = 1; _J < _Threads_num; _J++)
        {
            _Chunks[_J][_I] += _Chunks[_J - 1][_I];
        }

        // "_Chunks[_Threads_num - 1][_I] - _Last" will get the global _Size for chunk _I(including all threads local _Size for chunk _I)
        // this will chunk whether the chunk _I is empty or not. If it's not empty, it will be recorded.
        if (_Chunks[_Threads_num - 1][_I] - _Last)
        {
            ++_Count;
            _Index = _I;
        }
    }

    // If there is more than 1 chunk that has content, then continue the original algorithm
    if (_Count > 1)
    {
        // Move the elements in parallel into each chunk
        Concurrency::parallel_for(static_cast<size_t>(0), _Threads_num, [=](size_t _Index) 
        {
            size_t _Beg_index, _End_index;

            // Calculate the segment position
            if (_Index < _Remain)
            {
                _Beg_index = _Index * (_Step + 1);
                _End_index = _Beg_index + (_Step + 1);
            }
            else
            {
                _Beg_index = _Remain * (_Step + 1) + (_Index - _Remain) * _Step;
                _End_index = _Beg_index + _Step;
            }

            // Do a move operation to directly put each value into its destination chunk
            // Chunk pointer is moved after each put operation.
            if (_Beg_index != _End_index--)
            {
                while (_Beg_index != _End_index)
                {
                    _Output[--_Chunks[_Index][_Radix_key(_Begin[_End_index], _Radix, _Proj_func)]] = std::move(_Begin[_End_index]);
                    --_End_index;
                }
                _Output[--_Chunks[_Index][_Radix_key(_Begin[_End_index], _Radix, _Proj_func)]] = std::move(_Begin[_End_index]);
            }
        });

        // Invoke _parallel_integer_radix_sort in parallel for each chunk 
        Concurrency::parallel_for(static_cast<size_t>(0), static_cast<size_t>(256), [=](size_t _Index) 
        {
            if (_Index < 256 - 1)
            {
                _Parallel_integer_radix_sort(_Output + _Chunks[0][_Index], _Chunks[0][_Index + 1] - _Chunks[0][_Index], 
                    _Begin + _Chunks[0][_Index], _Radix - 1, _Proj_func, _Chunk_size, _Deep + 1);
            }
            else
            {
                _Parallel_integer_radix_sort(_Output + _Chunks[0][_Index], _Size - _Chunks[0][_Index], 
                    _Begin + _Chunks[0][_Index], _Radix - 1, _Proj_func, _Chunk_size, _Deep + 1);
            }
        });
    }
    else
    {
        // Only one chunk has content
        // A special optimization is applied because one chunk means all numbers have a same value on this particular byte (digit).
        // Since we cannot sort them at all (they are all equal at this point), directly call _parallel_integer_radix_sort to
        // sort next byte (digit)
        _Parallel_integer_radix_sort(_Begin, _Size, _Output, _Radix - 1, _Proj_func, _Chunk_size, _Deep);
    }
}

template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Function>
void _Parallel_integer_sort_asc(const _Random_iterator &_Begin, size_t _Size, const _Random_buffer_iterator &_Output,
    _Function _Proj_func, const size_t _Chunk_size)
{
    typedef typename std::iterator_traits<_Random_iterator>::value_type _Value_type;
    // The key type of the radix sort, this must be an "unsigned integer-like" type, i.e., it needs support: 
    //     operator>> (int), operator>>= (int), operator& (int), operator <, operator size_t ()
    typedef std::remove_const<std::remove_reference<decltype(_Proj_func(*_Begin))>::type>::type _Integer_type;

    // Find out the max value, which will be used to determine the highest differing byte (the radix position)
    _Integer_type _Max_val = Concurrency::samples::parallel_reduce(_Begin, _Begin + _Size, _Proj_func(*_Begin), 
        [=](_Random_iterator _Begin, _Random_iterator _End, _Integer_type _Init) -> _Integer_type 
    {
        while (_Begin != _End)
        {
            _Integer_type _Ret = _Proj_func(*_Begin++);
            if (_Init < _Ret)
            {
                _Init = _Ret;
            }
        }

        return _Init;
    }, std::max<_Integer_type>);
    size_t _Radix = 0;

    // Find out highest differing byte
    while (_Max_val >>= 8)
    {
        ++_Radix;
    }

    _Parallel_integer_radix_sort(_Begin, _Size, _Output, _Radix, _Proj_func, _Chunk_size);
}

template<typename _Random_iterator, typename _Function>
void _Parallel_quicksort_impl(const _Random_iterator &_Begin, size_t _Size, const _Function &_Func, size_t _Div_num, const size_t _Chunk_size, int _Depth)
{
    if (_Depth >= _SORT_MAX_RECURSION_DEPTH || _Size <= _Chunk_size || _Size <= static_cast<size_t>(3) || _Chunk_size >= _FINE_GRAIN_CHUNK_SIZE && _Div_num <= 1)
    {
        return std::sort(_Begin, _Begin + _Size, _Func);
    }

    // Determine whether we need to do a three-way quick sort
    // We benefit from three-way merge if there are a lot of elements that are EQUAL to the median value,
    // _Select_median_pivot function will test redundant density by sampling
    bool _Is_three_way_split = false;
    size_t _Mid_index = _Select_median_pivot(_Begin, _Size, _Func, _Chunk_size, _Is_three_way_split);

    // Move the median value to the _Begin position.
    if (_Mid_index)
    {
        std::swap(*_Begin, _Begin[_Mid_index]);
    }
    size_t _I = 1, _J = _Size - 1;

    // Three-way or two-way partition
    // _Div_num < _MAX_NUM_TASKS_PER_CORE is checked to make sure it will never do three-way split before splitting enough tasks
    if (_Is_three_way_split && _Div_num < _MAX_NUM_TASKS_PER_CORE)
    {
        while (_Func(*_Begin, _Begin[_J]))
        {
            --_J;
        }

        while (_Func(_Begin[_I], *_Begin))
        {
            ++_I;
        }

        // Starting from this point, left side of _I will less than median value, right side of _J will be greater than median value, 
        // and the middle part will be equal to median. _K is used to scan between _I and _J
        size_t _K = _J;
        while (_I <= _K)
        {
            if (_Func(_Begin[_K], *_Begin))
            {
                std::swap(_Begin[_I++], _Begin[_K]);
            }
            else
            {
                --_K;
            }

            while (_Func(*_Begin, _Begin[_K]))
            {
                std::swap(_Begin[_K--], _Begin[_J--]);
            }
        }

        ++_J;
    }
    else
    {
        while (_I <= _J)
        {
            // Will stop before _Begin
            while (_Func(*_Begin, _Begin[_J]))
            {
                --_J;
            }

            // There must be another element equal or greater than *_Begin
            while (_Func(_Begin[_I], *_Begin))
            {
                ++_I;
            }

            if (_I < _J)
            {
                std::swap(_Begin[_I++], _Begin[_J--]);
            }
            else
            {
                break;
            }
        }

        _I = ++_J;
    }

    std::swap(*_Begin, _Begin[--_I]);

    structured_task_group _Tg;
    volatile size_t _Next_div = _Div_num / 2;
    auto _Handle = make_task([&] 
    {
        _Parallel_quicksort_impl(_Begin + _J, _Size - _J, _Func, _Next_div, _Chunk_size, _Depth+1);
    });
    _Tg.run(_Handle);

    _Parallel_quicksort_impl(_Begin, _I, _Func, _Next_div, _Chunk_size, _Depth+1);

    // If at this point, the work hasn't been scheduled, then slow down creating new tasks
    if (_Div_num < _MAX_NUM_TASKS_PER_CORE)
    {
        _Next_div /= 2;
    }

    _Tg.wait();
}

// This function will be called to sort the elements in the "_Begin" buffer. However, we can't tell whether the result will end up in buffer
// "_Begin", or buffer "_Output" when it returned. The return value is designed to indicate which buffer holds the sorted result.
// Return true if the merge result is in the "_Begin" buffer; return false if the result is in the "_Output" buffer.
// We can't always put the result into one assigned buffer because that may cause frequent buffer copies at return time.
template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Function>
inline bool _Parallel_buffered_sort_impl(const _Random_iterator &_Begin, size_t _Size, _Random_buffer_iterator _Output, const _Function &_Func, 
    int _Div_num, const size_t _Chunk_size)
{
    static_assert(std::is_same<std::iterator_traits<_Random_iterator>::value_type, std::iterator_traits<_Random_buffer_iterator>::value_type>::value, 
        "same value type expected");

    if (_Div_num <= 1 || _Size <= _Chunk_size)
    {
        _Parallel_quicksort_impl(_Begin, _Size, _Func, _MAX_NUM_TASKS_PER_CORE, _Chunk_size, 0);

        // In case _Size <= _Chunk_size happened BEFORE the planned stop time (when _Div_num == 1) we need to calculate how many turns of 
        // binary divisions are left. If there are an odd number of turns left, then the buffer move is necessary to make sure the final 
        // merge result will be in the original input array.
        int _Left_div_turns = 0;
        while (_Div_num >>= 1)
        {
            _Left_div_turns++;
        }

        if (_Left_div_turns & 1)
        {
            std::move(_Begin, _Begin + _Size, _Output);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        size_t _Mid = _Size / 2;
        structured_task_group _Tg;

        auto _Handle = make_task([&, _Chunk_size] 
        {
            _Parallel_buffered_sort_impl(_Begin, _Mid, _Output, _Func, _Div_num / 2, _Chunk_size); 
        });
        _Tg.run(_Handle);

        bool _Is_buffer_swap = _Parallel_buffered_sort_impl(_Begin + _Mid, _Size - _Mid, _Output + _Mid, _Func, _Div_num / 2, _Chunk_size);

        _Tg.wait();

        if (_Is_buffer_swap)
        {
            _Parallel_merge(_Output, _Mid, _Output + _Mid, _Size - _Mid, _Begin, _Func, _Div_num);
        }
        else
        {
            _Parallel_merge(_Begin, _Mid, _Begin + _Mid, _Size - _Mid, _Output, _Func, _Div_num);
        }

        return !_Is_buffer_swap;
    }
}

// Disable the warning saying constant value in condition expression.
// This is by design that lets the compiler optimize the trivial constructor.
#pragma warning (push)
#pragma warning (disable: 4127)

// Allocate and construct a buffer
template<typename _Allocator>
inline typename _Allocator::pointer _Construct_buffer(size_t _N, _Allocator &_Alloc)
{
    typename _Allocator::pointer _P = _Alloc.allocate(_N);

    // If the objects being sorted have trivial default constructors, they do not need to be 
    // constructed here. This can benefit performance.
    if (!std::has_trivial_default_constructor<typename _Allocator::value_type>::value)
    {
        for (size_t _I = 0; _I < _N; _I++)
        {
            // Objects being sorted must have a default constructor
            _Allocator::value_type _T;
            _Alloc.construct(_P + _I, std::forward<_Allocator::value_type>(_T));
        }
    }

    return _P;
}

// Destroy and deallocate a buffer
template<typename _Allocator>
inline void _Destroy_buffer(typename _Allocator::pointer _P, size_t _N, _Allocator &_Alloc)
{
    // If the objects being sorted have trivial default destructors, they do not need to be 
    // destructed here. This can benefit performance.
    if (!std::has_trivial_destructor<typename _Allocator::value_type>::value)
    {
        for (size_t _I = 0; _I < _N; _I++)
        {
            _Alloc.destroy(_P + _I);
        }
    }

    _Alloc.deallocate(_P, _N);
}

//
// Exception safe RAII wrapper for the allocated buffers
//

template<typename _Allocator>
class _AllocatedBufferHolder
{
public:
    _AllocatedBufferHolder(size_t _Size, _Allocator _Alloc)
    {
        _M_size = _Size;
        _M_alloc = _Alloc;
        _M_buffer = _Construct_buffer(_Size, _Alloc);
    }

    ~_AllocatedBufferHolder()
    {
        _Destroy_buffer(_M_buffer, _M_size, _M_alloc);
    }

    typename _Allocator::pointer _Get_buffer()
    {
        return _M_buffer;
    }

private:
    size_t _M_size;
    _Allocator _M_alloc;
    typename _Allocator::pointer _M_buffer;
};

#pragma warning (pop)

/// <summary>
///     This template function is semantically similar with <c>std::sort</c> that it is a compare-based unstable in-place sort.
/// </summary>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for sort.
/// </param>
/// <remarks>
///     There are two overloads.
///     <para>For the first function overload, it is using an in-place sorting algorithm. A default <c>std::less</c> binary 
///     compare functor will be applied, so the compare operator <c>operator </c> is required for the element type, and the 
///     sorting result will be in increasing order. </para>
///     <para>For the second function overload, it is using an in-place sorting algorithm. A binary compare predicate functor 
///     <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is the element type.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial 
///     sort as soon as  the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before 
///     that point. Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Random_iterator>
inline void parallel_sort(const _Random_iterator &_Begin, const _Random_iterator &_End)
{
    parallel_sort(_Begin, _End, std::less<std::iterator_traits<_Random_iterator>::value_type>());
}

/// <summary>
///     This template function is semantically similar with <c>std::sort</c> that it is a compare-based unstable in-place sort.
/// </summary>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <typeparam name="_Function">
///     The binary comparison predicate functor type.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for sort.
/// </param>
/// <param name="_Func">
///     The binary comparison prediction functor.
/// </param>
/// <param name="_Chunk_size">
///     The minimal divisible chunk size that can be split for parallel execution.
/// </param>
/// <remarks>
///     There are two overloads.
///     <para>For the first function overload, it is using an in-place sorting algorithm. A default <c>std::less</c> binary 
///     compare functor will be applied, so the compare operator <c>operator </c> is required for the element type, and the 
///     sorting result will be in increasing order. </para>
///     <para>For the second function overload, it is using an in-place sorting algorithm. A binary compare predicate functor 
///     <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is the element type.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial 
///     sort as soon as  the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before 
///     that point. Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Random_iterator, typename _Function>
inline void parallel_sort(const _Random_iterator &_Begin, const _Random_iterator &_End, const _Function &_Func, const size_t _Chunk_size = 2048)
{
    // We make the guarantee that if the sort is part of a tree that has been canceled before starting, it will
    // not begin at all.
    if (is_current_task_group_canceling())
    {
        return;
    }

    size_t _Size = _End - _Begin;
    size_t _Core_num = Concurrency::CurrentScheduler::Get()->GetNumberOfVirtualProcessors();

    if (_Size <= _Chunk_size || _Core_num < 2)
    {
        return std::sort(_Begin, _End, _Func);
    }

    _Parallel_quicksort_impl(_Begin, _Size, _Func, _Core_num * _MAX_NUM_TASKS_PER_CORE, _Chunk_size, 0);
}

/// <summary>
///     This template function is semantically similar to <c>std::sort</c> in that it is a compare-based unstable sort, except that 
///     it needs O(n) additional space, and requires a default constructor for the type of sorting element.
/// </summary>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for sort.
/// </param>
/// <remarks>
///     There are four overloaded functions, they all require <c>n * sizeof(T)</c> additional space, where <c>n</c> is the number of elements 
///     to be sorted, and <c>T</c> is the element type.
///     <para>For the first function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate 
///     the buffer for this function.</para>
///     <para>For the second function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. To allocate the buffer for this algorithm, users should provide an allocator 
///     template argument. For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>For the third function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer in this function.</para>
///     <para>For the fourth function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. To allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Random_iterator>
inline void parallel_buffered_sort(const _Random_iterator &_Begin, const _Random_iterator &_End)
{
    parallel_buffered_sort<std::allocator<typename std::iterator_traits<_Random_iterator>::value_type>>(_Begin, _End, 
        std::less<std::iterator_traits<_Random_iterator>::value_type>());
}

/// <summary>
///     This template function is semantically similar to <c>std::sort</c> in that it is a compare-based unstable sort, except that 
///     it needs O(n) additional space, and requires a default constructor for the type of sorting element.
/// </summary>
/// <typeparam name="_Allocator">
///     The STL compatible memory allocator type.
/// </typeparam>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for sort.
/// </param>
/// <remarks>
///     There are four overloaded functions, they all require <c>n * sizeof(T)</c> additional space, where <c>n</c> is the number of elements 
///     to be sorted, and <c>T</c> is the element type.
///     <para>For the first function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate 
///     the buffer for this function.</para>
///     <para>For the second function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. To allocate the buffer for this algorithm, users should provide an allocator 
///     template argument. For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>For the third function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer in this function.</para>
///     <para>For the fourth function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. To allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Allocator, typename _Random_iterator>
inline void parallel_buffered_sort(const _Random_iterator &_Begin, const _Random_iterator &_End)
{
    parallel_buffered_sort<_Allocator>(_Begin, _End, 
        std::less<std::iterator_traits<_Random_iterator>::value_type>());
}

/// <summary>
///     This template function is semantically similar to <c>std::sort</c> in that it is a compare-based unstable sort, except that 
///     it needs O(n) additional space, and requires a default constructor for the type of sorting element.
/// </summary>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <typeparam name="_Function">
///     The binary comparison predicate functor type.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for sort.
/// </param>
/// <param name="_Func">
///     The binary comparison predicate functor.
/// </param>
/// <param name="_Chunk_size">
///     The minimal divisible chunk size that can be split for parallel execution.
/// </param>
/// <remarks>
///     There are four overloaded functions, they all require <c>n * sizeof(T)</c> additional space, where <c>n</c> is the number of elements 
///     to be sorted, and <c>T</c> is the element type.
///     <para>For the first function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate 
///     the buffer for this function.</para>
///     <para>For the second function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. To allocate the buffer for this algorithm, users should provide an allocator 
///     template argument. For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>For the third function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer in this function.</para>
///     <para>For the fourth function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. To allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Random_iterator, typename _Function>
inline void parallel_buffered_sort(const _Random_iterator &_Begin, const _Random_iterator &_End, const _Function &_Func, const size_t _Chunk_size = 2048)
{
    parallel_buffered_sort<std::allocator<typename std::iterator_traits<_Random_iterator>::value_type>>(_Begin, _End, _Func, _Chunk_size);
}

/// <summary>
///     This template function is semantically similar to <c>std::sort</c> in that it is a compare-based unstable sort, except that 
///     it needs O(n) additional space, and requires a default constructor for the type of sorting element.
/// </summary>
/// <typeparam name="_Allocator">
///     The STL compatible memory allocator type.
/// </typeparam>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <typeparam name="_Function">
///     The binary comparison predicate functor type.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for sort.
/// </param>
/// <param name="_Func">
///     The binary comparison predicate functor.
/// </param>
/// <param name="_Chunk_size">
///     The minimal divisible chunk size that can be split for parallel execution.
/// </param>
/// <remarks>
///     There are four overloaded functions, they all require <c>n * sizeof(T)</c> additional space, where <c>n</c> is the number of elements 
///     to be sorted, and <c>T</c> is the element type.
///     <para>For the first function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate 
///     the buffer for this function.</para>
///     <para>For the second function overload, a default <c>std::less</c> binary comparison functor will be applied to sort, so the comparison 
///     operator <c>operator <()</c> is required for the element type. To allocate the buffer for this algorithm, users should provide an allocator 
///     template argument. For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>For the third function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. The STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer in this function.</para>
///     <para>For the fourth function overload, a binary compare predicate functor <c>_Func: bool (T, T) </c> is required, in which <c>T</c> is 
///     the element type. To allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Allocator, typename _Random_iterator, typename _Function>
inline void parallel_buffered_sort(const _Random_iterator &_Begin, const _Random_iterator &_End, const _Function &_Func, const size_t _Chunk_size = 2048)
{
    // We make the guarantee that if the sort is part of a tree that has been canceled before starting, it will
    // not begin at all.
    if (is_current_task_group_canceling())
    {
        return;
    }

    size_t _Size = _End - _Begin;
    size_t _Core_num = Concurrency::CurrentScheduler::Get()->GetNumberOfVirtualProcessors();

    if (_Size <= _Chunk_size || _Core_num < 2)
    {
        return std::sort(_Begin, _End, _Func);
    }
    const static size_t CORE_NUM_MASK = 0x55555555;

    _Allocator _Alloc;
    _AllocatedBufferHolder<_Allocator> _Holder(_Size, _Alloc);

    // This buffered sort algorithm will divide chunks and apply parallel quicksort on each chunk. In the end, it will 
    // apply parallel merge to these sorted chunks.
    // 
    // We need to decide the number of chunks to divide the input buffer into. If we divide it into n chunks, log(n) 
    // merges will be needed to get the final sorted result.  In this algorithm, we have two buffers for each merge 
    // operation, let's say buffer A and B. Buffer A is the original input array, buffer B is the additional allocated 
    // buffer.  Each turn's merge will put the merge result into the other buffer; for example, if we decided to split 
    // into 8 chunks in buffer A at very beginning, after one pass of merging, there will be 4 chunks in buffer B.
    // If we apply one more pass of merging, there will be 2 chunks in buffer A again.
    // 
    // The problem is we want to the final merge pass to put the result back in buffer A, so that we don't need 
    // one extra copy to put the sorted data back to buffer A.
    // To make sure the final result is in buffer A (original input array), we need an even number of merge passes,
    // which means log(n) must be an even number. Thus n must be a number power(2, even number). For example, when the
    // even number is 2, n is power(2, 2) = 4, when even number is 4, n is power(2, 4) = 16. When we divide chunks 
    // into these numbers, the final merge result will be in the original input array. Now we need to decide the chunk(split) 
    // number based on this property and the number of cores.
    // 
    // We want to get a chunk (split) number close the the core number (or a little more than the number of cores), 
    // and it also needs to satisfy above property. For a 8 core machine, the best chunk number should be 16, because it's 
    // the smallest number that satisfies the above property and is bigger than the core number (so that we can utilize all 
    // cores, a little more than core number is OK, we need to split more tasks anyway). 
    // 
    // In this algorithm, we will make this alignment by bit operations (it's easy and clear). For a binary representation, 
    // all the numbers that satisfy power(2, even number) will be 1, 100, 10000, 1000000, 100000000 ...
    // After OR-ing these numbers together, we will get a mask (... 0101 0101 0101) which is all possible combinations of 
    // power(2, even number). We use _Core_num & CORE_NUM_MASK | _Core_num << 1 & CORE_NUM_MASK, a bit-wise operation to align 
    // _Core_num's highest bit into a power(2, even number).
    // 
    // It means if _Core_num = 8, the highest bit in binary is bin(1000) which is not power(2, even number). After this 
    // bit-wise operation, it will align to bin(10000) = 16 which is power(2, even number). If the _Core_num = 16, after 
    // alignment it still returns 16. The trick is to make sure the highest bit of _Core_num will align to the "1" bit of the 
    // mask bin(... 0101 0101 0101) We don't care about the other bits on the aligned result except the highest bit, since they 
    // will be ignored in the function.
    _Parallel_buffered_sort_impl(_Begin, _Size, stdext::make_unchecked_array_iterator(_Holder._Get_buffer()), 
        _Func, _Core_num & CORE_NUM_MASK | _Core_num << 1 & CORE_NUM_MASK, _Chunk_size);
}

#pragma warning(push)
#pragma warning (disable: 4127)
//
// This is a default function used for parallel_radixsort which will return just the value.
// It also performs compile-time checks to ensure that the data type is integral.
//
template <typename _DataType>
struct _Radix_sort_default_function
{
    size_t operator()(const _DataType& val) const
    {
        // An instance of the type predicate returns the value if the type _DataType is one of the integral types, otherwise it
        // statically asserts.
        // An integral type is one of: bool, char, unsigned char, signed char, wchar_t, short, unsigned short, int, unsigned int, long, 
        // and unsigned long. 
        // In addition, with compilers that provide them, an integral type can be one of long long, unsigned long long, __int64, and 
        // unsigned __int64
        static_assert(std::is_integral<_DataType>::value, 
            "Type should be integral to use default radix function. For more information on integral types, please refer to http://msdn.microsoft.com/en-us/library/bb983099.aspx.");
        static_assert((sizeof(_DataType) <= sizeof(size_t)), "Passed Type is bigger than size_t.");

        if (std::is_unsigned<_DataType>::value)
        {
            return val;
        }
        else
        {
            // The default function needs to take the signed integer-like representation and map it to an unsigned one.  The
            // following code will take the midpoint of the unsigned representable range (SIZE_MAX/2)+1 and does an unsigned
            // add of the value.  Thus, it maps a [-signed_min,+signed_max] range into a [0, unsigned_max] range.
            return (((SIZE_MAX/2) + 1) + static_cast<size_t>(val));
        }
    }
};
#pragma warning (pop)

/// <summary>
///     This template function will sort elements with a radix sorting algorithm. This is a stable sort function which requires a 
///     projection function that can project sorting elements into unsigned integer-like keys. The algorithm will sort elements in 
///     key increasing order.
/// </summary>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for radix sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for radix sort.
/// </param>
/// <remarks>
///     There are three overloads, they all require <c>n * sizeof(T)</c> bytes of additional space, where <c>n</c> is the number of elements
///     to be sorted, and <c>T</c> is the element type. An unary projection functor <c>_Proj_func: I (T)</c> is required to return a key 
///     from a sorting element, in which <c>T</c> is the element type and <c>I</c> is an unsigned integer-like type. A default constructor 
///     is required for the elements to be sorted.
///     <para>For the first function overload, the STL memory allocator <c>std::allocator<T></c> will be used to allocate the buffer for 
///     this function. In addition, a default projection function which simply returns the value is used.  This function generates a 
///     compiler error if the type is not an integral type.</para>
///     <para>For the second function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>. In addition, a default projection function which 
///     simply returns the value is used.  This function generates a compiler error if the type is not an integral type.</para>
///     <para>For the third function overload, the STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer 
///     for this function.</para>
///     <para>For the fourth function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Random_iterator>
inline void parallel_radixsort(const _Random_iterator &_Begin, const _Random_iterator &_End)
{
    typedef std::iterator_traits<_Random_iterator>::value_type _DataType;

    _Radix_sort_default_function<_DataType> _Proj_func;

    parallel_radixsort<std::allocator<_DataType>>(_Begin, _End, _Proj_func, 256 * 256);
}

/// <summary>
///     This template function will sort elements with a radix sorting algorithm. This is a stable sort function which requires a 
///     projection function that can project sorting elements into unsigned integer-like keys. The algorithm will sort elements in 
///     key increasing order.
/// </summary>
/// <typeparam name="_Allocator">
///     The STL compatible memory allocator type.
/// </typeparam>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires the iterator category to be random_iterator.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for radix sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for radix sort.
/// </param>
/// <remarks>
///     There are three overloads, they all require <c>n * sizeof(T)</c> bytes of additional space, where <c>n</c> is the number of elements
///     to be sorted, and <c>T</c> is the element type. An unary projection functor <c>_Proj_func: I (T)</c> is required to return a key 
///     from a sorting element, in which <c>T</c> is the element type and <c>I</c> is an unsigned integer-like type. A default constructor 
///     is required for the elements to be sorted.
///     <para>For the first function overload, the STL memory allocator <c>std::allocator<T></c> will be used to allocate the buffer for 
///     this function. In addition, a default projection function which simply returns the value is used.  This function generates a 
///     compiler error if the type is not an integral type.</para>
///     <para>For the second function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>. In addition, a default projection function which 
///     simply returns the value is used.  This function generates a compiler error if the type is not an integral type.</para>
///     <para>For the third function overload, the STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer 
///     for this function.</para>
///     <para>For the fourth function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Allocator, typename _Random_iterator>
inline void parallel_radixsort(const _Random_iterator &_Begin, const _Random_iterator &_End)
{
    typedef std::iterator_traits<_Random_iterator>::value_type _DataType;

    _Radix_sort_default_function<_DataType> _Proj_func;

    parallel_radixsort<_Allocator>(_Begin, _End, _Proj_func, 256 * 256);
}

/// <summary>
///     This template function will sort elements with a radix sorting algorithm. This is a stable sort function which requires a 
///     projection function that can project sorting elements into unsigned integer-like keys. The algorithm will sort elements in 
///     key increasing order.
/// </summary>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires iterator category to be random_iterator.
/// </typeparam>
/// <typeparam name="_Function">
///     The unary projection functor type.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for radix sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for radix sort.
/// </param>
/// <param name="_Func">
///     The unary projection functor which returns an unsigned integer-like key from the element type.
/// </param>
/// <param name="_Chunk_size">
///     The minimal divisible chunk size that can be split for parallel execution.
/// </param>
/// <remarks>
///     There are three overloads, they all require <c>n * sizeof(T)</c> bytes of additional space, where <c>n</c> is the number of elements
///     to be sorted, and <c>T</c> is the element type. An unary projection functor <c>_Proj_func: I (T)</c> is required to return a key 
///     from a sorting element, in which <c>T</c> is the element type and <c>I</c> is an unsigned integer-like type. A default constructor 
///     is required for the elements to be sorted.
///     <para>For the first function overload, the STL memory allocator <c>std::allocator<T></c> will be used to allocate the buffer for 
///     this function. In addition, a default projection function which simply returns the value is used.  This function generates a 
///     compiler error if the type is not an integral type.</para>
///     <para>For the second function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>. In addition, a default projection function which 
///     simply returns the value is used.  This function generates a compiler error if the type is not an integral type.</para>
///     <para>For the third function overload, the STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer 
///     for this function.</para>
///     <para>For the fourth function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Random_iterator, typename _Function>
inline void parallel_radixsort(const _Random_iterator &_Begin, const _Random_iterator &_End, const _Function &_Proj_func, const size_t _Chunk_size = 256 * 256)
{
    parallel_radixsort<std::allocator<typename std::iterator_traits<_Random_iterator>::value_type>>(
        _Begin, _End, _Proj_func, _Chunk_size);
}

/// <summary>
///     This template function will sort elements with a radix sorting algorithm. This is a stable sort function which requires a 
///     projection function that can project sorting elements into unsigned integer-like keys. The algorithm will sort elements in 
///     key increasing order.
/// </summary>
/// <typeparam name="_Allocator">
///     The STL compatible memory allocator type.
/// </typeparam>
/// <typeparam name="_Random_iterator">
///     The iterator type of the input range, it requires iterator category to be random_iterator.
/// </typeparam>
/// <typeparam name="_Function">
///     The unary projection functor type.
/// </typeparam>
/// <param name="_Begin">
///     The position of the first element to be included for radix sort.
/// </param>
/// <param name="_End">
///     The position of the first element not to be included for radix sort.
/// </param>
/// <param name="_Func">
///     The unary projection functor which returns an unsigned integer-like key from the element type.
/// </param>
/// <param name="_Chunk_size">
///     The minimal divisible chunk size that can be split for parallel execution.
/// </param>
/// <remarks>
///     There are three overloads, they all require <c>n * sizeof(T)</c> bytes of additional space, where <c>n</c> is the number of elements
///     to be sorted, and <c>T</c> is the element type. An unary projection functor <c>_Proj_func: I (T)</c> is required to return a key 
///     from a sorting element, in which <c>T</c> is the element type and <c>I</c> is an unsigned integer-like type. A default constructor 
///     is required for the elements to be sorted.
///     <para>For the first function overload, the STL memory allocator <c>std::allocator<T></c> will be used to allocate the buffer for 
///     this function. In addition, a default projection function which simply returns the value is used.  This function generates a 
///     compiler error if the type is not an integral type.</para>
///     <para>For the second function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>. In addition, a default projection function which 
///     simply returns the value is used.  This function generates a compiler error if the type is not an integral type.</para>
///     <para>For the third function overload, the STL memory allocator <c>std::allocator<T> </c> will be used to allocate the buffer 
///     for this function.</para>
///     <para>For the fourth function overload, to allocate the buffer for this algorithm, users should provide an allocator template argument. 
///     For more information about allocators, please refer to <see cref="allocator Class"/>.</para>
///     <para>
///     For the optional argument <c>_Chunk_size</c>, the partitioner will guarantee that it will stop splitting and turn to serial sort as soon as 
///     the size of the chunks is less than <c>_Chunk_size</c>, but it is still possible to stop splitting before that point. 
///     Move semantics are supported in this function, for more information about move semantics, please refer to 
///     <see cref=”Rvalue Reference Declarator: &amp;&amp;”/>.
///     </para>
/// </remarks>
/**/
template<typename _Allocator, typename _Random_iterator, typename _Function>
inline void parallel_radixsort(const _Random_iterator &_Begin, const _Random_iterator &_End, const _Function &_Proj_func, const size_t _Chunk_size = 256 * 256)
{
    // We make the guarantee that if the sort is part of a tree that has been canceled before starting, it will
    // not begin at all.
    if (is_current_task_group_canceling())
    {
        return;
    }

    size_t _Size = _End - _Begin;

    // If _Size <= 1, no more sorting needs to be done.
    if (_Size <= 1)
    {
        return;
    }

    _Allocator _Alloc;
    _AllocatedBufferHolder<_Allocator> _Holder(_Size, _Alloc);

    _Parallel_integer_sort_asc(_Begin, _Size, stdext::make_unchecked_array_iterator(_Holder._Get_buffer()), _Proj_func, _Chunk_size);
}

#pragma pop_macro("_SORT_MAX_RECURSION_DEPTH")
#pragma pop_macro("_MAX_NUM_TASKS_PER_CORE")
#pragma pop_macro("_FINE_GRAIN_CHUNK_SIZE")
} // namespace samples
} // namespace Concurrency