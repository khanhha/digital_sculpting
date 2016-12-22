#ifndef BASE_DEFINED_H
#define BASE_DEFINED_H

#include <math.h>

#pragma warning( disable : 4290 ) // To disable warning C4290: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)

typedef double double2f;

#ifndef EPSILON_VAL_
#define EPSILON_VAL_ 1e-6
#endif

#ifndef EPSILON_VAL_BIG
#define EPSILON_VAL_BIG 1e-5
#endif

#ifndef EPSILON_MAKE_COIN_POINT
#define EPSILON_MAKE_COIN_POINT 1e-6
#endif

#ifndef SQUARE_EPSILON_MAKE_COIN_POINT
#define SQUARE_EPSILON_MAKE_COIN_POINT EPSILON_MAKE_COIN_POINT*EPSILON_MAKE_COIN_POINT
#endif

#ifndef EPSILON_VAL_MICRO
#define EPSILON_VAL_MICRO 1e-12
#endif

#ifndef EPSILON_VAL_MINI
#define EPSILON_VAL_MINI 1e-10
#endif

#ifndef EPSILON_VAL_E3
#define EPSILON_VAL_E3 1e-3
#endif
#ifndef EPSILON_VAL_E4
#define EPSILON_VAL_E4 1e-4
#endif

#ifndef EPSILON_VAL_E8
#define EPSILON_VAL_E8 1e-8
#endif

#ifndef EPSILON_VAL_E1
#define EPSILON_VAL_E1 1e-1
#endif
#ifndef SQRT_3_
#define SQRT_3_ 1.7320508
#endif
#ifndef SQRT_2_
#define SQRT_2_ 1.41421356
#endif
#ifndef PI_VAL_
#define PI_VAL_  3.141592653
#endif

#ifndef EPSILON_MOVE_VP
#define EPSILON_MOVE_VP 0.00005
#endif

#define FLOAT_LESS_OR_EQUAL(a, b) ((a) < (b) || FLOAT_EQUAL(a, b))
#define FLOAT_EQUAL(a, b) (fabs((a) - (b)) < EPSILON_VAL_ ? true : false)

#define SQUARE_EPSILON_MOVE_VP EPSILON_MOVE_VP*EPSILON_MOVE_VP

#define MIN_STITCHING_TOLERANCE 0.02

#define REALLOC_STEP_ 4
#define COS_ANGLE_1_DEGREE  0.99984769   // cos of 1 degree
#define COS_ANGLE_3_DEGREE  0.99862953   // cos of 3 degree
#define COS_ANGLE_4_DEGREE  0.99756405   // cos of 4 degree
#define COS_ANGLE_5_DEGREE  0.99619469   // cos of 5 degree
#define COS_ANGLE_30_DEGREE 0.8660254  // cos of 30 degree 
#define COS_ANGLE_45_DEGREE 0.70710678  // cos of 45 degree 
#define COS_ANGLE_60_DEGREE 0.5  // cos of 60 degree 
#define COS_ANGLE_65_DEGREE 0.4226182617  // cos of 65 degree 
#define COS_ANGLE_70_DEGREE 0.34202014332  // cos of 70 degree 
#define COS_ANGLE_75_DEGREE 0.25881904  // cos of 75 degree 
#define COS_ANGLE_80_DEGREE 0.17364817766  // cos of 80 degree 
#define COS_ANGLE_85_DEGREE 0.08715574274765  // cos of 85 degree 
#define COS_ANGLE_88_DEGREE 0.0348994967025  // cos of 88 degree 
#define COS_ANGLE_MARK_COPLANAR 0.99984769 // 1 degree
#define COS_ANGLE_MARK_SHARP_EDGE 0.9396926 //20 degree
#define COS_MAX_1_ANGLE 0.17364817 //80 degree
#define COS_MAX_2_ANGLE -0.70710678 // 135 degree
#define COS_ANGLE_MARK_TWO_HOLE_COPLANAR 0.8660254 // 30 degree
#define TAN_ANGLE_5_DEGREE 0.0874886635259
#define TAN_ANGLE_30_DEGREE 0.577350269189 // tan of 30 degree
#define SIN_ANGLE_5_DEGREE 0.0871557427476 // sin of 5 degree


#define SCALE_MARK_NOISE_SHELL 0.001
#define ANGLE_MARK_SHARD_EDGE 20
#define LENGTH_MAKE_WALL_HANGING 10.0
#define OVERLAP_DISTANCE_VAL 0.2001
#define OVERLAP_COS_ANGLE_VAL 0.9986295 // 3 degree
#define TWO_PI_VAL 6.283185306

#define DEG2RAD(angle) ((angle) * PI_VAL_/180)
#define RAD2DEG(angle) ((angle) * 180 / PI_VAL_)

#define K_HUGE_NUMBER 999999999999.0
#define K_SMALL_NUMBER -K_HUGE_NUMBER

#define MM2INCH_RATE 0.0393700787
#define _1_PER_3 0.333333333333333333

#define ZERO_VECTOR_3Dd Point3Dd(0.0, 0.0, 0.0)
#define ZERO_VECTOR_2Dd Point2Dd(0.0, 0.0)
#define ZERO_VECTOR_2Df Point2Df(0.0, 0.0)

#define CIRCLE_TOLERANCE_2_THETA(tolerance, radius) \
                                (2 * acos((radius - tolerance) / radius))

const unsigned int MinRequiredMemory = 200*1024*1024; // Minimum required memory for K-Studio to operate

/* Round the float number to n decimal space */
#define ROUND_FLOAT(num, ndec) (floor((double)(num) * \
                                pow((double)10, (int)(ndec)) + 0.5) / \
                                pow((double)10, (int)(ndec)))

static unsigned const AVERAGE_MEMORY_STLTRIANGLE_OF_OBJECT = 330; // 
static unsigned const MAXIMUM_TRIANLGES_AVAILABLE = 15000000; // maximum allowable number of triangles 
static unsigned const SMALL_TRIANGLE_NUMBER = 100;

enum THREAD_JOB
{
       THREAD_JOB_POST_IMPORT_PROCESS,
       THREAD_JOB_FIX_WIZARD_PROGRESS,
       THREAD_JOB_SAVE_TO_UNDO,
       THREAD_JOB_UPDATE_AFTER_DUPLICATE,
       THREAD_JOB_UPDATE_TREE_STRUCTURE,
       THREAD_JOB_CLEAN_SUPPORT,
       THREAD_JOB_PUSH_VP_TRIG_TO_TREE,
       THREAD_JOB_UPDATE_IRREGULAR_EDGE,
       THREAD_JOB_NUMBER
};

#define USING_SEPARATE_VERTICES_FOR_VBO

#define CALC_BOUNDARY_OF_SHELL

#define USING_OFFSET_CONTOUR_TO_GEN_CENTERLINE

extern int g_debug_var;

// Macro



template <class T>
inline T min2(T a, T b)
{
    return( a < b ? a : b );
}


//(@)-------------------------------------------------------------------------------
//(@) find min of 3 values
//(@)-------------------------------------------------------------------------------
template <class T>
inline T min3(T a, T b, T c)
{
    return( min2( min2( a, b ), c ) );
}

///////////////////////
template <class T>
inline T min4(T a, T b, T c, T d)
{
    return( min2( min3( a, b, c ), d ) );
}
//(@)-------------------------------------------------------------------------------
//(@) find max of 2 values
//(@)-------------------------------------------------------------------------------
template <class T>
inline T max2(T a, T b)
{
    return( a > b ? a : b );
}


//(@)-------------------------------------------------------------------------------
//(@) find max of 3 values
//(@)-------------------------------------------------------------------------------
template <class T>
inline T max3(T a, T b, T c)
{
    return( max2( max2( a, b ), c ) );
}
///////////////////////////
template <class T>
inline T max4(T a, T b, T c, T d)
{
    return( max2( max3( a, b, c ), d ) );
}

template <class T>
inline void minmax2(T& min, T& max, T a, T b)
{
    if ( a > b ) {
        min = b; max = a;
    }
    else {
        min = a; max = b;
    }
}

template <class T>
inline void minmax3(T& min, T& max, T a, T b, T c)
{
    minmax2(min, max, a, b);
    if (c < min)
        min = c;
    else if ( c > max)
        max = c;
}

template <class T>
inline void minmax4(T& min, T& max, T a, T b, T c, T d)
{
    minmax3(min, max, a, b, c);
    if (d < min)
        min = d;
    else if ( d > max)
        max = d;
}
template <class T> void vb_swap(T& a, T& b)
{
    T c(a); a = b; b = c;
}
inline int doubleCompare(const double & a, const double & b)
{
   if (abs(a - b) < EPSILON_VAL_)
      return 0;
   else
      return (a > b) ? 1 : -1;
}

//inline void swapf(float& a, float& b)
//{
//	float tmp = a;
//	a = b;
//	b = tmp;
//}

// i+1 and i-1 modulo 3
// This way of computing it tends to be faster than using %
#define NEXT(i) ((i)<2 ? (i)+1 : (i)-2)
#define PREV(i) ((i)>0 ? (i)-1 : (i)+2)

#endif