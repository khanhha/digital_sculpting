#ifndef SCULPT_COMMON_DEFINE_H
#define SCULPT_COMMON_DEFINE_H

#include "BaseLib/Point2D.h"
#include "BaseLib/Point3Dd.h"
#include "tbb/task_scheduler_init.h"
#define TBB_PREVIEW_MEMORY_POOL 1
#include "tbb/memory_pool.h"
#include "tbb/cache_aligned_allocator.h"
#include "tbb/mutex.h"
#include <Eigen/Dense>


class StlVertex;
class NormalUpdateOp;
class Transform;

namespace sculpt
{
    const size_t thread_number = tbb::task_scheduler_init::default_num_threads();
    const size_t min_leaf_size = 100;
    const size_t max_leaf_size = 150;
}

//now we have four bit for flag
//so flag's value
enum EFlag
{
    E_VISITED = 1,
    E_NEW_SUBDIVISION_EDGE = 2,
	E_GARBAGE_SUBDIVISION_EDGE = 4
};

enum VFlag
{
    V_MARK_DIRTY= 1,
	V_NEW_SUBDIVISION_VERTEX = 2,
	V_GARBAGE_SUBDIVISION_VERTEX = 4
};

enum FFlag
{
    F_MARK_DIRTY = 1 << 0,
    F_NEW_SUBDIVISION_TRIANGLE = 1 << 1,
	F_GARBAGE_SUBDIVISION_TRIANGLE = 1 << 2 /*triangle created and then deleted during subdivision: we don't need it*/
};

enum SFlag
{
    PICK_ORIGINAL_LOCATION = (1 << 0),
    DYNAMIC_TOPOLOGY = (1 << 1),
    AVERAGE_DATA = (1 << 2)
};

//typedef std::allocator<size_t> PointerAllocator;
typedef tbb::scalable_allocator<size_t> PointerAllocator;
typedef tbb::memory_pool<PointerAllocator> PointerMemPool; //used inside each brush for vertex array

#endif