#include "util.h"
#include "defined.h"
#include <assert.h>
#include <windows.h>
#include "StackWalker.h"

#include <list>
#include <map>

#include <shlobj.h>   //for SHGetKnownFolderPath
#include <winerror.h> //for HRESULT
//#include <comutil.h>  //for _bstr_t (used in the string conversion)
//#include <iostream>
//#pragma comment(lib, "comsuppw")

int g_debug_var = 0;
#ifdef _DEBUG
 Point3Dd temp_dump_point1;
 Point3Dd temp_dump_point2;
 Point3Dd temp_dump_point3;
 Point3Dd temp_dump_point4;
#endif
//#define MALLOC_USING_OPERATOR_NEW

#ifdef K_DETECT_MEM_LEAK

StackWalker gStackWalker; // For getting call stack programmatically

unsigned Util_k_malloc_count = 0;
unsigned Util_k_realloc_count = 0;
unsigned Util_k_free_count = 0;

void * __cdecl operator new(unsigned int size, const char *file, int line)
{
    void *ptr = (void *)malloc(size);

    gStackWalker.ShowCallstack();
    BaseType::addMemoryTrack((unsigned long)ptr, size, gStackWalker.StackSymbols());
    return(ptr);
};

void __cdecl operator delete(void *p)
{
    BaseType::removeMemoryTrack((unsigned long)p);
    free(p);
};

#endif K_DETECT_MEM_LEAK

namespace BaseType {
std::ofstream Util::Util_fout;
char* Util::_reservedBuffer = nullptr;

/**************************************************************************************************
* @fn   std::string Util::getAppPath(void)
*
* @brief   Gets the application path.
*
* @author  Nghi
* @date 9/21/2012
*
* @return  The application path.
**************************************************************************************************/

std::string Util::getAppPath(void)
{
    char lpFilename[512];

    GetModuleFileNameA(0, lpFilename, 512);

    std::string retname(lpFilename);
    while (retname.back() != '\\') {
        retname.pop_back();
    }
    return retname;

}

/**************************************************************************************************
* @fn   std::wstring BaseType::Util::getAppPathW(void)
*
* @brief   Gets the application path.
*          !NOTE: Already including the last '\'
*
* @author Duc  - 4/26/2014
*
* @return  The application path.
**************************************************************************************************/

std::wstring Util::getAppPathW()
{
    wchar_t lpwFilename[512];

    GetModuleFileNameW(0, lpwFilename, 512);

    std::wstring retname(lpwFilename);
    // Remove file name part, until meet '\\' -> folder path
    while (retname.back() != '\\') {
        retname.pop_back();
    }
    return retname;
}

/**************************************************************************************************
* @fn  std::wstring Util::getProgramDataPath()
*
* @brief   Gets application data folder path.
*          http://msdn.microsoft.com/en-us/library/windows/desktop/bb762188(v=vs.85).aspx
*          SHGetKnownFolderPath:
*          This function replaces SHGetFolderPath. That older function is now simply a wrapper for SHGetKnownFolderPath.
* @author  Duc Than
* @date    4/26/2014
*
* @return  The program data folder path.
**************************************************************************************************/

std::wstring Util::getProgramDataPath()
{
    LPWSTR wszPath = NULL;
    HRESULT hr;

    hr = SHGetKnownFolderPath(FOLDERID_ProgramData, KF_FLAG_CREATE, NULL, &wszPath);

    if (SUCCEEDED(hr)){
        //_bstr_t bstrPath(wszPath);
        return std::wstring(wszPath);
    }
    else {
        return L"";
    }
}

/**************************************************************************************************
 * @fn  std::wstring Util::getLogFilePath(const std::wstring& companyName, const std::wstring& appName)
 *
 * @brief   Gets log folder path for Non-Qt libs (StlLib, BaseLib, etc.)
 *
 * @author  Duc Than
 * @date    4/26/2014
 *
 * @return  The log folder path.
 **************************************************************************************************/

std::wstring Util::getLogFilePath(const std::wstring& companyName, const std::wstring& appName)
{
    static const wchar_t* wszBaseLibLogFileName = L"BaseLib.log";  // Log information on BaseLib, StlLib, KFilesLib

    return Util::getProgramDataPath().append(L"\\").append(companyName)
                                     .append(L"\\").append(appName)
                                     .append(L"\\").append(wszBaseLibLogFileName);
}

void Util::initLogMessage(const std::wstring& companyName, const std::wstring& appName)
{
    std::wstring tempname = Util::getLogFilePath(companyName, appName);
    Util_fout.open(tempname);
}

void Util::printLogMessage(std::string &msg, bool const withEndl /*= false*/)
{
    /*std::string name = getAppPath();
    name += ("dump_msg.dump");
    std::ofstream fout(name,std::ios_base::);*/

    Util_fout <<msg;

    if (withEndl)
        Util_fout <<std::endl;
}

void Util::printLogMessage(char const *msg, bool const withEndl /*= false*/)
{
    Util_fout <<msg;

    if (withEndl)
        Util_fout <<std::endl;
}

void Util::printLogMessage(double value, char const *msg /*= nullptr*/)
{
    /*std::string name = getAppPath();
    name += ("dump_msg.dump");
    std::ofstream fout(name,std::ios_base::);*/

    if (msg)
        Util_fout << msg << "=" << value << std::endl;
    else
        Util_fout << value << std::endl;
}

void Util::printLogMessage(int value, char const *msg /*= nullptr*/)
{
    /*std::string name = getAppPath();
    name += ("dump_msg.dump");
    std::ofstream fout(name,std::ios_base::);*/

    if (msg)
        Util_fout << msg << "=" << value << std::endl;
    else
        Util_fout << value << std::endl;
}

void Util::printLogMessage(Point3Dd value, char const *msg /*= nullptr*/)
{
    /*std::string name = getAppPath();
    name += ("dump_msg.dump");
    std::ofstream fout(name,std::ios_base::);*/

    if (msg)
        Util_fout << msg << "= (" << value.x << "," << value.y << "," << value.z <<")" << std::endl;
    else
        Util_fout << "= (" << value.x << "," << value.y << "," << value.z << ")" << std::endl;
}

#ifdef MALLOC_USING_OPERATOR_NEW

void *Util::k_malloc(unsigned size)
{
    try{
        if(g_debug_var > 1){
            printLogMessage((int)size,"size");
            printLogMessage("before new");
        }
    char *memptr = nullptr;
    memptr = new char[size + sizeof(int)];
    if(g_debug_var > 1){
        
        printLogMessage((int)memptr,"memptr");
    }
    unsigned *first = (unsigned*)memptr;
    first[0] = size;
    memptr += sizeof(int);
#ifdef K_DETECT_MEM_LEAK
    ++Util_k_malloc_count;
    gStackWalker.ShowCallstack();

    addMemoryTrack((DWORD)memptr,  size, gStackWalker.StackSymbols());
#endif K_DETECT_MEM_LEAK
    return (void*)memptr;

    }
    catch(...){
        printLogMessage("Fail in k_malloc...");
        return 0;
    }
    
}

void *Util::k_realloc(void *memptr, unsigned size)
{
    
    if(0 == memptr){
        return k_malloc(size);
    }
    char* cmem = static_cast<char*>(memptr);
    cmem -= sizeof(unsigned);
    if(0 == size){
        delete [] cmem;
        return 0;
    }
    unsigned oldSize = *(reinterpret_cast<unsigned*>(cmem));
    if(size == oldSize){
        return memptr;
    }

#ifdef K_DETECT_MEM_LEAK
    --Util_k_malloc_count;
    ++Util_k_realloc_count;
    removeMemoryTrack((DWORD)memptr);
#endif

    char *newmem = new char[size + sizeof(int)];
    *(reinterpret_cast<unsigned*>(newmem)) = size;
    newmem += sizeof(unsigned);
    memcpy(newmem,memptr,std::min(size,oldSize));
    delete [] cmem;

#ifdef K_DETECT_MEM_LEAK
    gStackWalker.ShowCallstack();
    addMemoryTrack((DWORD)memptr,  size, gStackWalker.StackSymbols());
#endif K_DETECT_MEM_LEAK

    return (void*)newmem;
}

void Util::k_free(void *memptr)
{
#ifdef K_DETECT_MEM_LEAK
    ++Util_k_free_count;
    removeMemoryTrack((DWORD)memptr);
#endif K_DETECT_MEM_LEAK
    char* cmem = static_cast<char*>(memptr);
    cmem -= sizeof(unsigned);
    delete [] cmem;
    
}


#else
void *Util::k_malloc(unsigned size)
{
    
        void *memptr = malloc(size);
         
#ifdef K_DETECT_MEM_LEAK
        ++Util_k_malloc_count;
        gStackWalker.ShowCallstack();

        addMemoryTrack((DWORD)memptr,  size, gStackWalker.StackSymbols());
#endif K_DETECT_MEM_LEAK
        return memptr;
}

void *Util::k_realloc(void *memptr, unsigned size)
{
#ifdef K_DETECT_MEM_LEAK
    --Util_k_malloc_count;
    ++Util_k_realloc_count;
    removeMemoryTrack((DWORD)memptr);
#endif

    memptr = realloc(memptr,size);

#ifdef K_DETECT_MEM_LEAK
    gStackWalker.ShowCallstack();
    addMemoryTrack((DWORD)memptr,  size, gStackWalker.StackSymbols());
#endif K_DETECT_MEM_LEAK

    return memptr;
}

void Util::k_free(void *memptr)
{
#ifdef K_DETECT_MEM_LEAK
    ++Util_k_free_count;
    removeMemoryTrack((DWORD)memptr);
#endif K_DETECT_MEM_LEAK

    free(memptr);
}

#endif

int Util::sum_2d(const double a[2], const double b[2], double c[2])
{
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    return 0;
}

int Util::minus_2d(const double a[2], const double b[2], double c[2])
{
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    return 0;
}

double Util::multiply_2d(const double a[2], const double b[2])
{
    double dret;
    dret = a[0]*b[0] + a[1]*b[1];
    return dret;
}

double Util::calcSquareDistance_2d(const double a[2], const double b[2])
{
    double v[2];
    Util::minus_2d(a,b,v);
    return Util::multiply_2d(v,v);
}

bool Util::is2PointCoincidence(float *p1, float *p2)
{
    bool result = false;
    float dx = fabs(p1[0] - p2[0]);
    float dy = fabs(p1[1] - p2[1]);
    float dz = fabs(p1[2] - p2[2]);
    if(dx <= EPSILON_MAKE_COIN_POINT && dy <= EPSILON_MAKE_COIN_POINT && dz <= EPSILON_MAKE_COIN_POINT) {
        double dist = (dx * dx + dy * dy + dz * dz);
        if(dist <= SQUARE_EPSILON_MAKE_COIN_POINT) {
            result = true;
        }
    }
    return result;
}

/* calculate intersection point between segment and plane.
  @param $sp - start point of segment
  @param $ep - end point of segment
  @param $pv - a point on plane
  @param $nv - normal vector of plane
  @param $intp - intersection point (output)
  @return number intersection point
  @author Nghi
*/
int Util::calcIntSegmentPlane(
    const Point3Dd &sp, const Point3Dd &ep,
    const Point3Dd &pv, const Point3Dd &nv, Point3Dd &intp)
{
    Point3Dd s(pv);
    s -= sp;
    Point3Dd v(ep);
    v -= sp;
    double t = s.scalarProduct(nv)/v.scalarProduct(nv);
    if(t > 0.0 && t < 1.0) {
        intp = sp;
        intp += v*t;
        return 1;
    }
    return 0;
}

/* calculate intersection point between ray and plane.
  @param $sp - start point of ray
  @param $dir - direction of ray
  @param $pv - a point on plane
  @param $nv - normal vector of plane
  @param $intp - intersection point (output)
  @return number intersection point
  @author Nghi
*/
int Util::calcIntRayPlane(
    const Point3Dd &sp, const Point3Dd &dir,
    const Point3Dd &pv, const Point3Dd &nv, Point3Dd &intp)
{
    Point3Dd s(pv);
    s -= sp;

    double t = s.scalarProduct(nv)/dir.scalarProduct(nv);
    if(t > 0.0) {
        intp = sp;
        intp += dir*t;
        return 1;
    }
    return 0;
}

/* calculate intersection point between segment and plane.
  @param $sp - a point of line
  @param $dir - direction vector of line
  @param $pv - a point on plane
  @param $nv - normal vector of plane
  @param $intp - intersection point (output)
  @return number intersection point
  @author Nghi
*/
int Util::calcIntLinePlane(const Point3Dd &sp, const Point3Dd &dir,
                           const Point3Dd &pv, const Point3Dd &nv, Point3Dd &intp)
{
    double den = dir.scalarProduct(nv);
    if (fabs(den) < EPSILON_VAL_){
        return 0;
    }
    Point3Dd s(pv);
    s -= sp;
    double t = s.scalarProduct(nv)/den;
    intp = sp;
    intp += dir*t;
    return 1;
}

/* calculate intersection points between two Quadrangles co-planar.
  @param $rec1 - first quadrangle
  @param $rec2 - second quadrangle
  @param $intp - intersection points (output)
               The output points are not ordered as a polygon
  @return number of intersection points
  @author Son
*/
int Util::calcIntQuadQuadCoplanar(const std::vector<Point3Dd *> &quad1,
                                   const std::vector<Point3Dd *> &quad2,
                                   std::vector<Point3Dd> &intp)
{
    assert(quad1.size() == 4);
    assert(quad2.size() == 4);

    const Point3Dd *sp1, *ep1, *sp2, *ep2;
    Vector3Dd nv1, nv2;
    sp1 = quad1.back();
    nv1 = (*quad1[1] - *quad1[0]) * (*quad1[2] - *quad1[1]);
    nv2 = (*quad2[1] - *quad2[0]) * (*quad2[2] - *quad2[1]);
    if (nv1 == ZERO_VECTOR_3Dd || nv2 == ZERO_VECTOR_3Dd) 
        return 0;

    for (int i1 = 0; i1 < 4; i1++) {
        ep1 = quad1[i1];
        if (isPointInConvexPolygon3D(*ep1, quad2, nv2)) {
            intp.push_back(*ep1);
        }

        if (isPointInConvexPolygon3D(*quad2[i1], quad1, nv1))
            intp.push_back(*quad2[i1]);

        sp2 = quad2.back();
        for (int i2 = 0; i2 < 4; i2++) {
            ep2 = quad2[i2];
            Point3Dd p3d;
            int ind = nv1.getIndexOfMaxAbsCoord();
            if (calcIntPointOf2PlanarSegment(*sp1, *ep1, *sp2, *ep2, ind, p3d)) {
                intp.push_back(p3d);
            }
            sp2 = ep2;
        }
        sp1 = ep1;
    }

    return intp.size();
}

/* check collision between quadrangle and segment co-planar.
  @param $quad - quadrangle
  @param $sp - start point of segment
  @param $ep - end point of segment
  @return true if intersection
  @author Son
*/
bool Util::isQuadCollisionWithSegment(const std::vector<Point3Dd *> &quad,
                                      const Point3Dd &sp, const Point3Dd &ep,
                                      bool skipBound)
{
    assert (quad.size() == 4);
    Point3Dd v = (*quad[1] - *quad[0]) * (*quad[2] - *quad[1]);
    v.normalize();
    if (isPointInConvexPolygon3D(sp, quad, v, true) ||
        isPointInConvexPolygon3D(ep, quad, v, true)) {
            return true;
    }

    Point3Dd p0, p1;
    int ind = v.getIndexOfMaxAbsCoord();
    p0 = *quad[3];
    for (unsigned i = 0; i < quad.size(); i++)
    {
       p1 = *quad[i];
       if (is2SegmentCollision(sp, ep, p0, p1, ind, skipBound))
           return true;
       p0 = p1;
    }

    return false;
}

/**
   Check collision between box and line
   @param $pv a point of the line
   @param $nv direction vector of the line
   @param $ov origin of box
   @param $sz size of box
   @return TRUE if has collision vice versa return FALSE
   @author Nghi
*/
bool Util::isBoxCollisionWithLine(const Point3Dd &pv, const Point3Dd &nv,
                                  const Point3Dd &ov, const double &sz)
{
    Point3Dd dv(ov);
    dv += sz/2.0;
    dv -= pv;
    double sv = dv.scalarProduct(nv);
    double ss = nv.scalarProduct(nv);
    double t = sv/ss;

    Point3Dd point(pv); // a point on line that is nearest from box's center
    point += nv*t;

    if ( point.v[0] > (ov.v[0] - EPSILON_VAL_) && point.v[0] < (ov.v[0] + sz + EPSILON_VAL_) &&
            point.v[1] > (ov.v[1] - EPSILON_VAL_) && point.v[1] < (ov.v[1] + sz + EPSILON_VAL_) &&
            point.v[2] > (ov.v[2] - EPSILON_VAL_) && point.v[2] < (ov.v[2] + sz + EPSILON_VAL_)) {
        return true;
    }

    for ( int i = 0; i < 3; ++i) {
        if (point.v[i] < ov.v[i] ) {
            int i1 = (i + 1) % 3;
            int i2 = (i + 2) % 3;
            double t = (ov.v[i] - pv.v[i])/nv.v[i];
            double int_1 = pv.v[i1] + t*nv.v[i1];
            double int_2 = pv.v[i2] + t*nv.v[i2];
            if (int_1 > (ov.v[i1] - EPSILON_VAL_) && int_1 < (ov.v[i1] + sz + EPSILON_VAL_) &&
                    int_2 > (ov.v[i2] - EPSILON_VAL_) && int_2 < (ov.v[i2] + sz + EPSILON_VAL_)) {
                return true;
            }
            continue;
        }

        if (point.v[i] > (ov.v[i] + sz)) {
            int i1 = (i + 1) % 3;
            int i2 = (i + 2) % 3;
            double t = (ov.v[i] + sz - pv.v[i])/nv.v[i];
            double int_1 = pv.v[i1] + t*nv.v[i1];
            double int_2 = pv.v[i2] + t*nv.v[i2];
            if (int_1 > (ov.v[i1] - EPSILON_VAL_) && int_1 < (ov.v[i1] + sz + EPSILON_VAL_) &&
                    int_2 > (ov.v[i2] - EPSILON_VAL_) && int_2 < (ov.v[i2] + sz + EPSILON_VAL_)) {
                return true;
            }
        }
    }
    return false;
}
bool Util::isBoxCollisionWithRay(const Point3Dd &pv, const Point3Dd &nv,
                                 const Point3Dd &ov, const double &sz)
{
    Point3Dd dv(ov);
    dv += sz/2.0;
    dv -= pv;
    double sv = dv.scalarProduct(nv);
    double ss = nv.scalarProduct(nv);
    double t = sv/ss;

    Point3Dd point(pv); // a point on line that is nearest from box's center
    point += nv*t;
    double dis2 = point.distance2(pv);
    double halfSize2 = sz/2.0;
    halfSize2 *= halfSize2;
    if(dis2 > halfSize2 && t < 0) {
        return false;
    }
    if ( point.v[0] > (ov.v[0] - EPSILON_VAL_) && point.v[0] < (ov.v[0] + sz + EPSILON_VAL_) &&
            point.v[1] > (ov.v[1] - EPSILON_VAL_) && point.v[1] < (ov.v[1] + sz + EPSILON_VAL_) &&
            point.v[2] > (ov.v[2] - EPSILON_VAL_) && point.v[2] < (ov.v[2] + sz + EPSILON_VAL_)) {
        return true;
    }

    for ( int i = 0; i < 3; ++i) {
        if (point.v[i] < ov.v[i] ) {
            int i1 = (i + 1) % 3;
            int i2 = (i + 2) % 3;
            double t = (ov.v[i] - pv.v[i])/nv.v[i];
            double int_1 = pv.v[i1] + t*nv.v[i1];
            double int_2 = pv.v[i2] + t*nv.v[i2];
            if (int_1 > (ov.v[i1] - EPSILON_VAL_) && int_1 < (ov.v[i1] + sz + EPSILON_VAL_) &&
                    int_2 > (ov.v[i2] - EPSILON_VAL_) && int_2 < (ov.v[i2] + sz + EPSILON_VAL_)) {
                return true;
            }
            continue;
        }

        if (point.v[i] > (ov.v[i] + sz)) {
            int i1 = (i + 1) % 3;
            int i2 = (i + 2) % 3;
            double t = (ov.v[i] + sz - pv.v[i])/nv.v[i];
            double int_1 = pv.v[i1] + t*nv.v[i1];
            double int_2 = pv.v[i2] + t*nv.v[i2];
            if (int_1 > (ov.v[i1] - EPSILON_VAL_) && int_1 < (ov.v[i1] + sz + EPSILON_VAL_) &&
                    int_2 > (ov.v[i2] - EPSILON_VAL_) && int_2 < (ov.v[i2] + sz + EPSILON_VAL_)) {
                return true;
            }
        }
    }
    return false;
}

bool Util::isBoxCollisionWithSegment(const double sv[3], const double ev[3],
                                     const double ov[3], const double &sz)
{
    if((sv[0] >= ov[0] && sv[0] <= (ov[0] + sz) &&
            sv[1] >= ov[1] && sv[1] <= (ov[1] + sz) &&
            sv[2] >= ov[2] && sv[2] <= (ov[2] + sz)) ||
            (ev[0] >= ov[0] && ev[0] <= (ov[0] + sz) &&
             ev[1] >= ov[1] && ev[1] <= (ov[1] + sz) &&
             ev[2] >= ov[2] && ev[2] <= (ov[2] + sz))) {
        return true;
    }
    for(int i = 0; i < 3; ++i) {
        if(sv[i] > ev[i]) {
            if(sv[i] < ov[i] || ev[i] > (ov[i] + sz)) {
                return false;
            }
        } else {
            if(ev[i] < ov[i] || sv[i] > (ov[i] + sz)) {
                return false;
            }
        }
    }
    // check intersection betweensegment with surface of box
    for(int i = 0; i < 3; ++i) {
        int i1 = (i + 1) % 3;
        int i2 = (i + 2) % 3;
        double t = -1.0;
        if(sv[i] < ov[i]) {
            t = (ov[i] - sv[i])/(ev[i] - sv[i]);
            if(t < 0.0 || t > 1.0) {
                assert("Util::isBoxCollisionWithSegment" == "0");
                return false;
            }
        } else if(sv[i] > (ov[i] + sz)) {
            t = (ov[i] + sz - sv[i])/(ev[i] - sv[i]);
            if(t < 0.0 || t > 1.0) {
                assert("Util::isBoxCollisionWithSegment" == "0");
                return false;
            }
        }
        if(t > 0.0) {
            double int_1 = sv[i1] + t*(ev[i1] - sv[i1]);
            double int_2 = sv[i2] + t*(ev[i2] - sv[i2]);
            if (int_1 > (ov[i1] - EPSILON_VAL_) && int_1 < (ov[i1] + sz + EPSILON_VAL_) &&
                    int_2 > (ov[i2] - EPSILON_VAL_) && int_2 < (ov[i2] + sz + EPSILON_VAL_)) {
                return true;
            }
        }
    }
    return 0;
}

/* check collision between box and plane
   @param $p - point on plane
   @param $n - normal vector of plane
   @param $ov - origin point of box
   @param $sz - size of box
   @return true if box collision with the plane, otherwise return false
   @author Son
*/
bool Util::isBoxCollisionWithPlane(const Point3Dd &p, const Vector3Dd &n, Point3Dd &ov, const double &sz)
{
    Point3Dd ip;
    double d, d2, d1 = 1;

    for (unsigned i1 = 0; i1 < 2; i1++)
        for (unsigned i2 = 0; i2 < 2; i2++)
            for (unsigned i3 = 0; i3 < 2; i3++)
            {
                ip[0] = ov[0] + i1 * sz;
                ip[1] = ov[1] + i2 * sz;
                ip[2] = ov[2] + i3 * sz;
                d2 = (ip - p).scalarProduct(n);
                d = d1 * d2;

                if (i1 != 0 || i2 != 0 || i3 !=0) /* not the first */
                    if (d < EPSILON_VAL_ * EPSILON_VAL_) return true;
                d1 = d2;
            }

    return false;
}

bool Util::isBoxInsideOtherBox(const double mincoord1[3], const double maxcoord1[3],
                               const double mincoord2[3], const double maxcoord2[3])
{
    if (mincoord1[0] > ( mincoord2[0] - EPSILON_VAL_E3) && maxcoord1[0] < (maxcoord2[0] + EPSILON_VAL_E3) &&
            mincoord1[1] > ( mincoord2[1] - EPSILON_VAL_E3) && maxcoord1[1] < (maxcoord2[1] + EPSILON_VAL_E3) &&
            mincoord1[2] > ( mincoord2[2] - EPSILON_VAL_E3) && maxcoord1[2] < (maxcoord2[2] + EPSILON_VAL_E3)) {
        return true;
    }
    return false;
}

void Util::writeBytesFromInt(::std::ofstream &file, int valueIn)
{
    union {
        int intValue;
        char charValue[4];
    } value;
    value.intValue = valueIn;
    int newValue  = value.charValue[0] & 0xFF;
    newValue |= (value.charValue[1] & 0xFF) << 0x08;
    newValue |= (value.charValue[2] & 0xFF) << 0x10;
    newValue |= (value.charValue[3] & 0xFF) << 0x18;
    file.write(reinterpret_cast<char *>(&newValue), sizeof(newValue));
}

void Util::writeBytesFromFloat(::std::ofstream &file, float valueIn)
{
    union {
        float floatValue;
        char charValue[4];
    } value;
    value.floatValue = valueIn;
    int newValue  = value.charValue[0] & 0xFF;
    newValue |= (value.charValue[1] & 0xFF) << 0x08;
    newValue |= (value.charValue[2] & 0xFF) << 0x10;
    newValue |= (value.charValue[3] & 0xFF) << 0x18;
    file.write(reinterpret_cast<char *>(&newValue), sizeof(newValue));
}

void Util::writeBytesFromBool(::std::ofstream &file, bool valueIn)
{
    file.write((char *)&valueIn, sizeof(bool));
}

bool Util::isTwo2DAABBCollision(const double *minCoord1, const double *maxCoord1,
                                const double *minCoord2, const double *maxCoord2)
{
    if(minCoord1[0] >= maxCoord2[0] ||
            minCoord1[1] >= maxCoord2[1] ||
            minCoord2[0] >= maxCoord1[0] ||
            minCoord2[1] >= maxCoord1[1]) {
        return false;
    }
    return true;
}
bool Util::isTwoAABBCollision(const double *minCoord1, const double *maxCoord1,
                              const double *minCoord2, const double *maxCoord2)
{
    if(minCoord1[0] > maxCoord2[0] ||
            minCoord1[1] > maxCoord2[1] ||
            minCoord1[2] > maxCoord2[2] ||
            minCoord2[0] > maxCoord1[0] ||
            minCoord2[1] > maxCoord1[1] ||
            minCoord2[2] > maxCoord1[2]) {
        return false;
    }
    return true;
}
#if 0
void Util::calcRotMatrix(double angle, double *p,
                         double *dir, double m[4][4])
{
    double c_ = cos(angle);
    double s_ = sin(angle);
    double a = p[0];
    double b = p[1];
    double c = p[2];
    double u = dir[0];
    double v = dir[1];
    double w = dir[2];
    double u2 = u*u;
    double v2 = v*v;
    double w2 = w*w;

    m[0][0] = u2 + (v2 + w2)*c_;
    m[0][1] = u*v*(1 - c_) - w*s_;
    m[0][2] = u*w*(1 - c_) + v*s_;
    m[0][3] = (a*(v2 + w2) - u*(b*v + c*w))*(1 - c_) + (b*w -c*v)*s_;
    m[1][0] = u*v*(1 - c_) + w*s_;
    m[1][1] = v2 + (u2 + w2)*c_;
    m[1][2] = v*w*(1 - c_) - u*s_;
    m[1][3] = (b*(u2 + w2) - v*(a*u + c*w))*(1 - c_) + (c*u - a*w)*s_;
    m[2][0] = u*w*(1 - c_) - v*s_;
    m[2][1] = v*w*(1 - c_) + u*s_;
    m[2][2] = w2 + (u2 + v2)*c_;
    m[2][3] = (c*(u2 + v2) - w*(a*u + b*v))*(1 - c_) + (a*v - b*u)*s_;
    m[3][0] = 0.0;
    m[3][1] = 0.0;
    m[3][2] = 0.0;
    m[3][3] = 1.0;
}
#else
void Util::calcRotMatrix(double angle, double *p,
                         double *dir, double *m)
{
    double c_ = cos(angle);
    double s_ = sin(angle);
    double a = p[0];
    double b = p[1];
    double c = p[2];
    double u = dir[0];
    double v = dir[1];
    double w = dir[2];
    double u2 = u*u;
    double v2 = v*v;
    double w2 = w*w;

    m[0] = u2 + (v2 + w2)*c_;
    m[4] = u*v*(1 - c_) - w*s_;
    m[8] = u*w*(1 - c_) + v*s_;
    m[12] = (a*(v2 + w2) - u*(b*v + c*w))*(1 - c_) + (b*w -c*v)*s_;
    m[1] = u*v*(1 - c_) + w*s_;
    m[5] = v2 + (u2 + w2)*c_;
    m[9] = v*w*(1 - c_) - u*s_;
    m[13] = (b*(u2 + w2) - v*(a*u + c*w))*(1 - c_) + (c*u - a*w)*s_;
    m[2] = u*w*(1 - c_) - v*s_;
    m[6] = v*w*(1 - c_) + u*s_;
    m[10] = w2 + (u2 + v2)*c_;
    m[14] = (c*(u2 + v2) - w*(a*u + b*v))*(1 - c_) + (a*v - b*u)*s_;
    m[3] = 0.0;
    m[7] = 0.0;
    m[11] = 0.0;
    m[15] = 1.0;
}

#endif

/*!
  Calculate square minimum distance between two point in 2D space
  @param $p1 - first point
  @param $p2 - second point
  @return square distance
  @author Nghi
*/
double Util::calc2DSquareMinDistance2Point(double *p1, double *p2)
{
    double dret = (p2[0] - p1[0])*(p2[0] - p1[0]) + (p2[1] - p1[1])*(p2[1] - p1[1]);
    return dret;
}

/*!
  Calculate square minimum distance from point to a line segment in 2D space
  @param $point - the point
  @param $sp    - start point of segment
  @param $ep    - end point of segment
  @return distance
  @author Nghi
*/
double Util::calc2DSquareMinDistancePointSegment(const double *point, const double *sp, const double *ep)
{
    double ret=0.0;
    double v[2];
    Util::minus_2d(ep,sp,v);

    double s[2];
    Util::minus_2d(point,sp,s);

    double num = Util::multiply_2d(v,s);
    if (num < 0) {
        ret = Util::calcSquareDistance_2d(point,sp);
    } else {
        double den = Util::multiply_2d(v,v);
        if ( num > den) {
            ret = Util::calcSquareDistance_2d(point,ep);
        } else {
            ret = (den*(Util::multiply_2d(s,s)) - num*num)/den;
        }
    }
    return ret;
}

/*!
  check whether a point is inside a convex polygon
  @param $p - the point
  @param $plg - the polygon
  @return 'true' if the point is inside the polygon
  @author Nghi
*/
bool Util::isPointInConvexPolygon3D(const Point3Dd &p,
                                    const std::vector<Point3Dd *> &plg, const Vector3Dd &plgNorm,
                                    bool skipBound)
{
    Point3Dd v0;
    Point3Dd v1;

    Point3Dd p1;
    p.orthoToPlane(*(plg.at(0)),plgNorm,p1);
    size_t n = plg.size();
    Vector3Dd nv1;
    for(size_t i = 0; i < n; ++i) {
        int j = (i+1) % n;
        v0 = *(plg.at(j));
        v0 -= *(plg.at(i));
        Vector3Dd v(p1);
        v -= *(plg.at(i));
        nv1 = v0*v;
        if(nv1.scalarProduct(plgNorm) < (skipBound ? EPSILON_VAL_ : -EPSILON_VAL_)) {
            return false;
        }
    }
    return true;
}

bool Util::isPointInsideRectang(const double *p, const double *op,
                                const double *sz)
{
    if (p[0] > (op[0] - EPSILON_VAL_) && p[0] < (op[0] + sz[0] + EPSILON_VAL_) &&
            p[1] > (op[1] - EPSILON_VAL_) && p[1] < (op[1] + sz[1] + EPSILON_VAL_)) {
        return true;
    }
    return false;
}

/* check whether a point is inside a triangle in 2D space
   @param: p - the point
   @param: p1,p2,p3 - three vertices of triangle
   @param: normdir - direction of normal vector
*/
bool Util::isPointInsideTriangle2D(const double *p, const double *p1, const double *p2,
                                   const double *p3, const double &normdir)
{
    double x1 = p2[0] - p1[0];
    double y1 = p2[1] - p1[1];
    double x2 = p[0] - p1[0];
    double y2 = p[1] - p1[1];
    double d = x1*y2 - x2*y1;
    if (fabs(d) < EPSILON_VAL_BIG) {
        if (fabs(x1)> fabs(y1)) {
            if (x1*x2 < 0.0) {
                return false;
            } else {
                if(fabs(x2) > (fabs(x1)+EPSILON_VAL_)) {
                    return false;
                }
                return true;
            }
        } else {
            if (y1*y2 < 0.0) {
                return false;
            } else {
                if(fabs(y2) > (fabs(y1)+EPSILON_VAL_)) {
                    return false;
                }
                return true;
            }
        }
    }
    if(d*normdir < -EPSILON_VAL_) {
        return false;
    }

    x1 = p3[0] - p2[0];
    y1 = p3[1] - p2[1];
    x2 = p[0] - p2[0];
    y2 = p[1] - p2[1];
    d = x1*y2 - x2*y1;
    if (fabs(d) < EPSILON_VAL_BIG) {
        if (fabs(x1)> fabs(y1)) {
            if (x1*x2 < 0.0) {
                return false;
            } else {
                if(fabs(x2) > (fabs(x1)+EPSILON_VAL_)) {
                    return false;
                }
                return true;
            }
        } else {
            if (y1*y2 < 0.0) {
                return false;
            } else {
                if(fabs(y2) > (fabs(y1)+EPSILON_VAL_)) {
                    return false;
                }
                return true;
            }
        }
    }
    if(d*normdir < -EPSILON_VAL_) {
        return false;
    }

    x1 = p1[0] - p3[0];
    y1 = p1[1] - p3[1];
    x2 = p[0] - p3[0];
    y2 = p[1] - p3[1];
    d = x1*y2 - x2*y1;
    if (fabs(d) < EPSILON_VAL_BIG) {
        if (fabs(x1)> fabs(y1)) {
            if (x1*x2 < 0.0) {
                return false;
            } else {
                if(fabs(x2) > (fabs(x1)+EPSILON_VAL_)) {
                    return false;
                }
                return true;
            }
        } else {
            if (y1*y2 < 0.0) {
                return false;
            } else {
                if(fabs(y2) > (fabs(y1)+EPSILON_VAL_)) {
                    return false;
                }
                return true;
            }
        }
    }
    if(d*normdir < -EPSILON_VAL_) {
        return false;
    }
    return true;
}

bool Util::calcIntLineWithSegment(const double p[2],
                                  const double dv[2], const double sp[2], const double ep[2], double intp[2])
{
    double dx = ep[0] - sp[0];
    double dy = ep[1] - sp[1];
    double del = dx*dv[1] - dv[0]*dy;
    if(fabs(del) < EPSILON_VAL_) {
        return false;
    }
    double del1 = dv[0]*(sp[1] - p[1]) - dv[1]*(sp[0] - p[0]);
    double t = del1/del;
    if(t > -EPSILON_VAL_ && t < (1.0+EPSILON_VAL_)) {
        intp[0] = sp[0] + dx*t;
        intp[1] = sp[1] + dy*t;
        return true;
    }
    return false;
}

/*!
  calculate intersection point between a ray and segment.
  (2D)
  @param $p - start point of ray
  @param $dv - direction vector of ray
  @param $sp - start point of segment
  @param $ep - end point of segment
  @param $intP - intersection point
  @return 
        true if intersection
  @author Son
*/
bool Util::calcIntRayWithSegment(const double p[2], const double dv[2],
                                const double sp[2], const double ep[2],
                                double intp[2])
{
    double dx1 = dv[0];
    double dy1 = dv[1];
    double dx2 = ep[0] - sp[0];
    double dy2 = ep[1] - sp[1];
    double del = dx2*dy1 - dx1*dy2;
    if(fabs(del) < EPSILON_VAL_) {
        return false;
    }
    double del0 = dx2*(sp[1] - p[1]) - dy2*(sp[0] - p[0]);

    double t0 = del0/del;
    if(t0 > EPSILON_VAL_) {
        double del1 = dx1*(sp[1] - p[1]) - dy1*(sp[0] - p[0]);
        double t1 = del1/del;
        //if(t1 > 0 && t1 < (1.0)) {
        if(t1 > -EPSILON_VAL_ && t1 < (1.0+EPSILON_VAL_)) {
            intp[0] = p[0] + dx1*t0;
            intp[1] = p[1] + dy1*t0;
            return true;
        }
    }

    return false;
}

bool Util::calcIntRayWithRay(const double p1[2], const double dv1[2],
                             const double p2[2], const double dv2[2],
                             double intp[2])
{
    double dx1 = dv1[0];
    double dy1 = dv1[1];
    double dx2 = dv2[0];
    double dy2 = dv2[1];
    double del = dx2*dy1 - dx1*dy2;
    if(fabs(del) < EPSILON_VAL_) {
        return false;
    }
    double del0 = dx2*(p2[1] - p1[1]) - dy2*(p2[0] - p1[0]);

    double t0 = del0/del;
    if(t0 > 0) {
        double del1 = dx1*(p2[1] - p1[1]) - dy1*(p2[0] - p1[0]);
        double t1 = del1/del;
        if(t1 > 0) {
            intp[0] = p1[0] + dx1*t0;
            intp[1] = p1[1] + dy1*t0;
            return true;
        }
    }

    return false;
}

bool Util::calcInt2Segment2D(const double sp1[2],
                             const double ep1[2], const double sp2[2], const double ep2[2], double intp[2])
{
    double dx1 = ep1[0] - sp1[0];
    double dy1 = ep1[1] - sp1[1];
    double dx2 = ep2[0] - sp2[0];
    double dy2 = ep2[1] - sp2[1];
    double del = dx2*dy1 - dx1*dy2;
    if(fabs(del) < EPSILON_VAL_) {
        return false;
    }
    double del0 = dx2*(sp2[1] - sp1[1]) - dy2*(sp2[0] - sp1[0]);

    double t0 = del0/del;
    if(t0 > -EPSILON_VAL_ && t0 < (1.0+EPSILON_VAL_)) {
        double del1 = dx1*(sp2[1] - sp1[1]) - dy1*(sp2[0] - sp1[0]);
        double t1 = del1/del;
        if(t1 > -EPSILON_VAL_ && t1 < (1.0+EPSILON_VAL_)) {
            intp[0] = sp1[0] + dx1*t0;
            intp[1] = sp1[1] + dy1*t0;
            return true;
        }
    }
    return false;
}

bool Util::calcInt2Segment2DIgnore1stTip(const double sp1[2],
                                         const double ep1[2], const double sp2[2], const double ep2[2], double intp[2])
{
    double dx1 = ep1[0] - sp1[0];
    double dy1 = ep1[1] - sp1[1];
    double dx2 = ep2[0] - sp2[0];
    double dy2 = ep2[1] - sp2[1];
    double del = dx2*dy1 - dx1*dy2;
    if(fabs(del) < EPSILON_VAL_) {
        return false;
    }
    double del0 = dx2*(sp2[1] - sp1[1]) - dy2*(sp2[0] - sp1[0]);

    double t0 = del0/del;
    if(t0 > EPSILON_VAL_ && t0 < (1.0+EPSILON_VAL_)) {
        double del1 = dx1*(sp2[1] - sp1[1]) - dy1*(sp2[0] - sp1[0]);
        double t1 = del1/del;
        if(t1 > -EPSILON_VAL_ && t1 < (1.0+EPSILON_VAL_)) {
            intp[0] = sp1[0] + dx1*t0;
            intp[1] = sp1[1] + dy1*t0;
            return true;
        }
    }
    return false;
}
/*!
  calculate intersection point between two coplanar segments.
  (two segments on same plane)
  @param $sp0 - start point of first segment
  @param $ep0 - end point of first segment
  @param $sp1 - start point of second segment
  @param $ep1 - end point of second segment
  @param $i - index of projected plane
  @param $intP - intersection point
  @return 'true' if exist intersection
  @author Nghi
*/
bool Util::calcIntPointOf2PlanarSegment(const Point3Dd &sp0, const Point3Dd &ep0,
                                        const Point3Dd &sp1, const Point3Dd &ep1, const int i, Point3Dd &intP)
{
    int ix = (i + 1) % 3;
    int iy = (i + 2) % 3;
    double dx0 = ep0.v[ix] - sp0.v[ix];
    double dy0 = ep0.v[iy] - sp0.v[iy];
    double dx1 = ep1.v[ix] - sp1.v[ix];
    double dy1 = ep1.v[iy] - sp1.v[iy];
    double del = dx1*dy0 - dx0*dy1;
    if(fabs(del) < EPSILON_VAL_MICRO) {
        return false;
    }
    double del0 = dx1*(sp1.v[iy] - sp0.v[iy]) - dy1*(sp1.v[ix] - sp0.v[ix]);

    double t0 = del0/del;
    if(t0 > -EPSILON_VAL_ && t0 < (1.0+EPSILON_VAL_)) {
        double del1 = dx0*(sp1.v[iy] - sp0.v[iy]) - dy0*(sp1.v[ix] - sp0.v[ix]);
        double t1 = del1/del;
        if(t1 > -EPSILON_VAL_ && t1 < (1.0+EPSILON_VAL_)) {
            intP.v[ix] = sp0.v[ix] + dx0*t0;
            intP.v[iy] = sp0.v[iy] + dy0*t0;
            intP.v[i] = sp0.v[i] + (ep0.v[i] - sp0.v[i])*t0;
            return true;
        }
    }
    return false;
}
bool Util::calcIntPointOf2PlanarLine(const Point3Dd &p0, const Vector3Dd &v0,
                                      const Point3Dd &p1, const Vector3Dd &v1,
                                      const int i,
                                      Point3Dd &intP)
{
    int ix = (i + 1) % 3;
    int iy = (i + 2) % 3;
    double dx0 = v0[ix];
    double dy0 = v0[iy];
    double dx1 = v1[ix];
    double dy1 = v1[iy];
    double del = dx1*dy0 - dx0*dy1;
    if (fabs(del) < EPSILON_VAL_MICRO) {
        return false;
    }
    double del0 = dx1*(p1.v[iy] - p0.v[iy]) - dy1*(p1.v[ix] - p0.v[ix]);

    double t0 = del0 / del;
    
    intP.v[ix] = p0.v[ix] + dx0*t0;
    intP.v[iy] = p0.v[iy] + dy0*t0;
    intP.v[i] = p0.v[i] + (v0[i]) * t0;
    //intP = p0 + v0 * t0;
    return true;
}

bool Util::calcIntLineSegmentPlanar(const Point3Dd &p, const Vector3Dd &dir, const Point3Dd &sp, const Point3Dd &ep,
    const int i, Point3Dd &intp)
{
    int ix = (i + 1) % 3;
    int iy = (i + 2) % 3;
    double dx0 = dir[ix];
    double dy0 = dir[iy];
    double dx1 = ep[ix] - sp[ix];
    double dy1 = ep[iy] - sp[iy];
    double del = dx1*dy0 - dx0*dy1;
    if (fabs(del) < EPSILON_VAL_MICRO) {
        return false;
    }
    double del1 = dx0*(sp.v[iy] - p.v[iy]) - dy0*(sp.v[ix] - p.v[ix]);
    double t1 = del1 / del;
    if (t1 < -EPSILON_VAL_ || t1 > (1 + EPSILON_VAL_))
        return false;
    intp.v[ix] = sp.v[ix] + dx1*t1;
    intp.v[iy] = sp.v[iy] + dy1*t1;
    intp.v[i] = sp.v[i] + (ep.v[i] - sp.v[i]) * t1;

    return true;
}

/*!
  calculate intersection point between two coplanar segments ignore them tips
  (two segments on same plane)
  @param $sp0 - start point of first segment
  @param $ep0 - end point of first segment
  @param $sp1 - start point of second segment
  @param $ep1 - end point of second segment
  @param $i - index of projected plane
  @param $intP - intersection point
  @return 'true' if exist intersection
  @author Son
*/
int Util::calcIntPointOf2PlanarSegmentIgnore1Tip(const Point3Dd &sp0, const Point3Dd &ep0,
                                            const Point3Dd &sp1, const Point3Dd &ep1, const int i, Point3Dd &intP)
{
    int ix = (i + 1) % 3;
    int iy = (i + 2) % 3;
    double dx0 = ep0.v[ix] - sp0.v[ix];
    double dy0 = ep0.v[iy] - sp0.v[iy];
    double dx1 = ep1.v[ix] - sp1.v[ix];
    double dy1 = ep1.v[iy] - sp1.v[iy];
    double del = dx1*dy0 - dx0*dy1;
    if(fabs(del) < EPSILON_VAL_MICRO) {
        return false;
    }
    double del0 = dx1*(sp1.v[iy] - sp0.v[iy]) - dy1*(sp1.v[ix] - sp0.v[ix]);

    double t0 = del0/del;
    if(t0 > -EPSILON_VAL_ && t0 < (1.0 + EPSILON_VAL_)) {
        double del1 = dx0*(sp1.v[iy] - sp0.v[iy]) - dy0*(sp1.v[ix] - sp0.v[ix]);
        double t1 = del1/del;
        if(t1 > -EPSILON_VAL_ && t1 < (1.0 + EPSILON_VAL_)) {
            if ((FLOAT_EQUAL(0.0, t0) || FLOAT_EQUAL(1.0, t0)) &&
                (FLOAT_EQUAL(0.0, t1) || FLOAT_EQUAL(1.0, t1))) {
                return false;
            }
            
            if (FLOAT_EQUAL(0.0, t0)) {
                intP = sp0;
                return 2;
            } else if (FLOAT_EQUAL(1.0, t0)) {
                intP = sp1;
                return 3;
            } else if (FLOAT_EQUAL(0.0, t1)) {
                intP = ep0;
                return 4;
            } else if (FLOAT_EQUAL(1.0, t1)) {
                intP = ep1;
                return 5;
            } else {
                intP.v[ix] = sp0.v[ix] + dx0*t0;
                intP.v[iy] = sp0.v[iy] + dy0*t0;
                intP.v[i] = sp0.v[i] + (ep0.v[i] - sp0.v[i])*t0;

                return 1;
            }
        }
    }
    return 0;
}
bool Util::calcIntPointOf2PlanarSegmentIgnoreAllTips(const Point3Dd &sp0, const Point3Dd &ep0,
                                              const Point3Dd &sp1, const Point3Dd &ep1, const int i, Point3Dd &intP)
{
    int ix = (i + 1) % 3;
    int iy = (i + 2) % 3;
    double dx0 = ep0.v[ix] - sp0.v[ix];
    double dy0 = ep0.v[iy] - sp0.v[iy];
    double dx1 = ep1.v[ix] - sp1.v[ix];
    double dy1 = ep1.v[iy] - sp1.v[iy];
    double del = dx1*dy0 - dx0*dy1;
    if(fabs(del) < EPSILON_VAL_MICRO) {
        return false;
    }
    double del0 = dx1*(sp1.v[iy] - sp0.v[iy]) - dy1*(sp1.v[ix] - sp0.v[ix]);

    double t0 = del0/del;
    if(t0 > EPSILON_VAL_ && t0 < (1.0-EPSILON_VAL_)) {
        double del1 = dx0*(sp1.v[iy] - sp0.v[iy]) - dy0*(sp1.v[ix] - sp0.v[ix]);
        double t1 = del1/del;
        if(t1 > EPSILON_VAL_ && t1 < (1.0-EPSILON_VAL_)) {
            intP.v[ix] = sp0.v[ix] + dx0*t0;
            intP.v[iy] = sp0.v[iy] + dy0*t0;
            intP.v[i] = sp0.v[i] + (ep0.v[i] - sp0.v[i])*t0;
            return true;
        }
    }
    return false;
}

/*!
  calculate intersection point between two lines in 2D.
  @param $p1 - a point on first line
  @param $dv1 - direction of first line
  @param $p2 - a point on second line
  @param $dv2 - direction of second line
  @param $t - parameter of intersect point (according to first line)
  @return 'true' if exist intersection
  @author Nghi
*/
bool Util::calc2DIntPointOf2Line(const double *p1, const double *dv1,
                                 const double *p2, const double *dv2, double &t)
{
    double del = dv1[1]*dv2[0] - dv1[0]*dv2[1];
    if(fabs(del) < EPSILON_VAL_) {
        return false;
    }

    double del1 = dv2[0]*(p2[1] - p1[1]) - dv2[1]*(p2[0] - p1[0]);

    t = del1/del;
    return true;
}

/*!
  check collision between two coplanar segments.
  (two segments on same plane)
  @param $sp0 - start point of first segment
  @param $ep0 - end point of first segment
  @param $sp1 - start point of second segment
  @param $ep1 - end point of second segment
  @param $i - index of projected plane
  @return 'true' if collision
  @author Nghi
*/
bool Util::is2SegmentCollision(const Point3Dd &sp0, const Point3Dd &ep0,
                               const Point3Dd &sp1, const Point3Dd &ep1, const int i,
                               bool skipTip)
{
    int ix = (i + 1) % 3;
    int iy = (i + 2) % 3;
    double norm[2] = {ep0.v[iy] - sp0.v[iy], sp0.v[ix] - ep0.v[ix]};
    double v1[2] = {sp1.v[ix] - sp0.v[ix], sp1.v[iy] - sp0.v[iy]};
    double v2[2] = {ep1.v[ix] - sp0.v[ix], ep1.v[iy] - sp0.v[iy]};
    double d1 = norm[0]*v1[0] + norm[1]*v1[1];
    double d2 = norm[0]*v2[0] + norm[1]*v2[1];
    if (d1*d2 > (skipTip ? -EPSILON_VAL_ : 0.0)) {
        return false;
    }

    norm[0] = ep1.v[iy] - sp1.v[iy];
    norm[1] = sp1.v[ix] - ep1.v[ix];
    v1[0] = -v1[0];
    v1[1] = -v1[1];
    v2[0] = ep0.v[ix] - sp1.v[ix];
    v2[1] = ep0.v[iy] - sp1.v[iy];
    d1 = norm[0]*v1[0] + norm[1]*v1[1];
    d2 = norm[0]*v2[0] + norm[1]*v2[1];
    if (d1*d2 > (skipTip ? -EPSILON_VAL_ : 0.0)) {
        return false;
    }
    return true;
}


/*!
Check  collision between segment with plane.
@param $sp - start point of segment
@param $ep - end point of segment
@param $pv - a point on plane
@param $nv - normal vector of plane
@return 'true' if collision
@author Nghi
*/
bool Util::isSegmentCollisionPlane(const Point3Dd &sp, const Point3Dd &ep, const Point3Dd &pv, const Vector3Dd &nv)
{
    Vector3Dd v1(sp);
    v1 -= pv;
    Vector3Dd v2(ep);
    v2 -= pv;
    if (v1.scalarProduct(nv)*v2.scalarProduct(nv) < EPSILON_VAL_){
        return true;
    }
    return false;
}

bool Util::is2SegmentCollision2D(const double *sp0, const double *ep0,
                                 const double *sp1, const double *ep1)
{
    double norm[2] = {ep0[1] - sp0[1], sp0[0] - ep0[0]};
    double v1[2] = {sp1[0] - sp0[0], sp1[1] - sp0[1]};
    double v2[2] = {ep1[0] - sp0[0], ep1[1] - sp0[1]};
    double d1 = norm[0]*v1[0] + norm[1]*v1[1];
    double d2 = norm[0]*v2[0] + norm[1]*v2[1];
    if(d1*d2 > EPSILON_VAL_) {
        return false;
    } else if(d1*d2 > -EPSILON_VAL_) {
        if(fabs(norm[0]) > fabs(norm[1])) {
            if(max2(sp0[1],ep0[1]) < min2(sp1[1],ep1[1]) ||
                    max2(sp1[1],ep1[1]) < min2(sp0[1],ep0[1])) {
                return false;
            }
            return true;
        } else {
            if(max2(sp0[0],ep0[0]) < min2(sp1[0],ep1[0]) ||
                    max2(sp1[0],ep1[0]) < min2(sp0[0],ep0[0])) {
                return false;
            }
            return true;
        }
    }

    norm[0] = ep1[1] - sp1[1];
    norm[1] = sp1[0] - ep1[0];

    v1[0] = -v1[0];
    v1[1] = -v1[1];
    v2[0] = ep0[0] - sp1[0];
    v2[1] = ep0[1] - sp1[1];
    d1 = norm[0]*v1[0] + norm[1]*v1[1];
    d2 = norm[0]*v2[0] + norm[1]*v2[1];
    if(d1*d2 > 0.0) {
        return false;
    }
    return true;
}

/*!
  Check whether a point is inside a segment
  @param $sp - start point of segment
  @param $ep - end point of segment
  @param $p - the point
  @param $tout - output parameter
  @return 0 - p is outside segment
          1 - p is coincident with sp
          2 - p is coincident with ep
          3 - p is inside segment
  @author Nghi
*/
int Util::isPointInsideSegment(const Point3Dd &sp, const Point3Dd &ep,
                               const Point3Dd &p, double *tout)
{
    Vector3Dd dv(ep);
    dv -= sp;
    int i = dv.getIndexOfMaxAbsCoord();
    double t = (p.v[i] - sp.v[i])/dv.v[i];
    if(t < -EPSILON_VAL_ || t > (1.0 + EPSILON_VAL_)) {
        return 0;
    }
    int i1 = (i + 1) % 3;
    int i2 = (i + 2) % 3;
    if(tout) {
        *tout = t;
    }
#ifdef _DEBUG
    double dx = sp.v[i1] + dv.v[i1]*t - p.v[i1];
    double dy = sp.v[i2] + dv.v[i2]*t - p.v[i2];
#endif
    if(fabs(sp.v[i1] + dv.v[i1]*t - p.v[i1]) > EPSILON_VAL_BIG ||
            fabs(sp.v[i2] + dv.v[i2]*t - p.v[i2]) > EPSILON_VAL_BIG) {
        return 0;
    }
    if(t < EPSILON_VAL_E4) {
        return 1;
    } else if( t > (1.0 - EPSILON_VAL_E4)) {
        return 2;
    }
    return 3;
}

int Util::isPointInsideSegment_HighAccur(const Point3Dd &sp, const Point3Dd &ep,
        const Point3Dd &p)
{
    Vector3Dd dv(ep);
    dv -= sp;
    int i = dv.getIndexOfMaxAbsCoord();
    double t = (p.v[i] - sp.v[i])/dv.v[i];
    if(t < -EPSILON_VAL_ || t > (1.0 + EPSILON_VAL_)) {
        return 0;
    }
    int i1 = (i + 1) % 3;
    int i2 = (i + 2) % 3;
#ifdef _DEBUG
    double dx = sp.v[i1] + dv.v[i1]*t - p.v[i1];
    double dy = sp.v[i2] + dv.v[i2]*t - p.v[i2];
#endif
    if(fabs(sp.v[i1] + dv.v[i1]*t - p.v[i1]) > EPSILON_VAL_ ||
            fabs(sp.v[i2] + dv.v[i2]*t - p.v[i2]) > EPSILON_VAL_) {
        return 0;
    }
    if(t < EPSILON_VAL_) {
        return 1;
    } else if( t > (1.0 - EPSILON_VAL_)) {
        return 2;
    }
    return 3;
}
int Util::isPointInsideSegmentStrong(const Point3Dd &sp, const Point3Dd &ep,
                                     const Point3Dd &p)
{
    Vector3Dd dv(ep);
    dv -= sp;
    int i = dv.getIndexOfMaxAbsCoord();
    double t = (p.v[i] - sp.v[i])/dv.v[i];
    if(t < -EPSILON_VAL_ || t > (1.0 + EPSILON_VAL_)) {
        return 0;
    }
    int i1 = (i + 1) % 3;
    int i2 = (i + 2) % 3;
    if(fabs(sp.v[i1] + dv.v[i1]*t - p.v[i1]) > EPSILON_VAL_BIG ||
            fabs(sp.v[i2] + dv.v[i2]*t - p.v[i2]) > EPSILON_VAL_BIG) {
        return 0;
    }
    if(t < EPSILON_VAL_) {
        return 1;
    } else if( t > (1.0 - EPSILON_VAL_)) {
        return 2;
    }
    return 3;
}

bool Util::isPointInsidePolygon2D(const std::vector<Point2Dd> &polygon, const Point2Dd &p)
{
    unsigned n = polygon.size();
    if(n < 3){
        return false;
    }
    double x1,x2,y1,y2,y;
    int nint = 0,j;
    Point2Dd p2d = polygon[0];
    x1 = p2d.x;
    y1 = p2d.y;
    for(unsigned i = 0; i < n; ++i)
    {
        p2d = polygon[(i + 1) % n];

        x2 = p2d.x;
        y2 = p2d.y;

        if (p.y < std::min(y1,y2) - EPSILON_VAL_  ||
            p.y > std::max(y1,y2) + EPSILON_VAL_)
        {
            x1 = x2;
            y1 = y2;
            continue;
        }
        else if(fabs(p.y - y1) < EPSILON_VAL_){
            if (FLOAT_EQUAL(p.x, x1)){
                return true;
            } else if(p.x < x1 - EPSILON_VAL_){
                nint++;
            }
        }
        else if(fabs(p.y - y2) < EPSILON_VAL_){
            if (FLOAT_EQUAL(p.x, x2)) {
                return true;
            } else {
                j = (i+1) % n;
                y = p.y;
                if (p.x < x2 - EPSILON_VAL_ && 
                    (y - polygon[j].y) * (y - y1) * 1e6 > EPSILON_VAL_) {
                    nint++;
                } else {
                    x1 = x2;
                    y1 = y2;
                    continue;
                }
            }
        }
        else if (fabs(y1 - y2) < EPSILON_VAL_){
            if (p.x > std::min(x1,x2) && p.x <= std::max(x1,x2) ){ // on boundary
                return true;
            }
        }
        else
        {
            double t = (p.y - y1)/(y2-y1);

            if(t < -EPSILON_VAL_){
                x1 = x2;
                y1 = y2;
                continue;
            }
            else if(t > (1.0 + EPSILON_VAL_))
            {
                double x = x1 + t*(x2 - x1);
                j = (i+1) % n;
                y = p2d.y;

                if((y - y2) * (y1 - y2) > SQUARE_EPSILON_MAKE_COIN_POINT){
                }
                else if(x > p.x + EPSILON_VAL_){
                    nint++;
                }
            }
            else{
                double x = x1 + t*(x2 - x1);
                if(fabs(x - p.x) < EPSILON_VAL_){ // on boundary
                    return true;
                }
                else if (x > (p.x) + EPSILON_VAL_)
                {
                    nint++;
                }
            }
        }

        x1 = x2;
        y1 = y2;
    }
    if(1 == (nint % 2)){
        return true;
    }

    return false;
}

bool Util::isPointInsidePolygon2D(const std::vector<Point2Df*> &polygon, const Point2Df &p, bool skipBound)
{
    unsigned n = polygon.size();
    if (n < 3){
        return false;
    }
    double x1, x2, y1, y2, y;
    int nint = 0, j;
    double epsilon = EPSILON_VAL_E4;
    double sqEpsilon = epsilon * epsilon;
    Point2Df p2d = *polygon[0];
    x1 = p2d.x;
    y1 = p2d.y;
    for (unsigned i = 0; i < n; ++i)
    {
        p2d = *polygon[(i + 1) % n];

        x2 = p2d.x;
        y2 = p2d.y;

        if (p.y < std::min(y1, y2) - epsilon ||
            p.y > std::max(y1, y2) + epsilon)
        {
            x1 = x2;
            y1 = y2;
            continue;
        }
        else if (fabs(p.y - y1) < epsilon){
            if (FLOAT_EQUAL(p.x, x1)){ // coincident with x1, y1
                return skipBound ? false : true;
            }
            else if (p.x < x1 - epsilon){
                nint++;
            }
        }
        else if (fabs(p.y - y2) < epsilon){
            if (FLOAT_EQUAL(p.x, x2)) { // coincident with x2, y2
                return skipBound ? false : true;
            }
            else {
                j = (i + 1) % n;
                y = p.y;
                if (p.x < x2 - epsilon &&
                    (y - (*polygon[j]).y) * (y - y1) * epsilon > epsilon) {
                    nint++;
                }
                else {
                    x1 = x2;
                    y1 = y2;
                    continue;
                }
            }
        }
        else if (fabs(y1 - y2) < epsilon){
            if (p.x > std::min(x1, x2) && p.x <= std::max(x1, x2)){ // on boundary
                return skipBound ? false : true;
            }
        }
        else
        {
            double t = (p.y - y1) / (y2 - y1);

            if (t < -epsilon){
                x1 = x2;
                y1 = y2;
                continue;
            }
            else if (t >(1.0 + epsilon))
            {
                double x = x1 + t*(x2 - x1);
                j = (i + 1) % n;
                y = p2d.y;

                if ((y - y2) * (y1 - y2) > sqEpsilon){
                }
                else if (x > p.x + epsilon){
                    nint++;
                }
            }
            else{
                double x = x1 + t*(x2 - x1);
                if (fabs(x - p.x) < epsilon){ // on boundary
                    return skipBound ? false : true;
                }
                else if (x > (p.x) + epsilon)
                {
                    nint++;
                }
            }
        }

        x1 = x2;
        y1 = y2;
    }
    if (1 == (nint % 2)){
        return true;
    }

    return false;
}

/* Check collision between prism and aabb
   @param rect - boundary of prism
   @param nv - direction of prism
   @parm ov - origin of aabb
   @param lv - size of aabb
   @return 0 - intersection
	        1 - in
			  -1 - out
   @author Nghi
*/
int Util::isPrismAABBCollision(const std::vector<Point3Dd *> &rect, const Point3Dd &nv,
                               const Point3Dd &ov,const double sz)
{
    Point3Dd lv(sz,sz,sz);
    int i = -1;
    if (fabs(1.0 - fabs(nv.v[0])) < EPSILON_VAL_) {
        i = 0;
    } else if(fabs(1.0 - fabs(nv.v[1])) < EPSILON_VAL_) {
        i = 1;
    } else if(fabs(1.0 - fabs(nv.v[2])) < EPSILON_VAL_) {
        i = 2;
    }

    if (i > -1) {
        int num_inVS = 0;
        int i0 = (i+1) % 3;
        int i1 = (i+2) % 3;
        Vector3Dd pv(ov);
        if (isPointInConvexPolygon3D(pv,rect,nv)) {
            num_inVS++;
        }
        pv.v[i0] += lv.v[i0];
        if (isPointInConvexPolygon3D(pv,rect,nv)) {
            //return true;
            num_inVS++;
        }
        pv.v[i1] += lv.v[i1];
        if (isPointInConvexPolygon3D(pv,rect,nv)) {
            //return true;
            num_inVS++;
        }
        pv.v[i0] -= lv.v[i0];
        if (isPointInConvexPolygon3D(pv,rect,nv)) {
            // return true;
            num_inVS++;
        }
        if (4 == num_inVS) {
            return 1;
        }
        if(num_inVS > 0) {
            return 0;
        }
        Point3Dd cen(rect[2]);
        cen += *(rect[0]);
        cen /= 2;
        if (cen.v[i0] > ov.v[i0] &&
                cen.v[i0] < (ov.v[i0] + lv.v[i0]) &&
                cen.v[i1] > ov.v[i1] &&
                cen.v[i1] < (ov.v[i1] + lv.v[i1])) {
            return 0;
        }
        Point3Dd sp(ov),ep(ov);
        ep.v[i0] += lv.v[i0];
        for(int jj = 0; jj < 4; jj++) {
            int jj1 = (jj + 1) % 4;
            if(is2SegmentCollision(sp,ep,*rect[jj],*rect[jj1],i)) {
                return 0;
            }
        }
        /*if(isTwoSegmentCollision(sp,ep,m_vpp[0]->m_v,m_vpp[2]->m_v,i)){
         return 0;
        }*/
        sp.v[i0] += lv.v[i0];
        ep.v[i1] += lv.v[i1];
        for(int jj = 0; jj < 4; jj++) {
            int jj1 = (jj + 1) % 4;
            if(is2SegmentCollision(sp,ep,*rect[jj],*rect[jj1],i)) {
                return 0;
            }
        }

        ep.v[i0] -= lv.v[i0];
        sp.v[i1] += lv.v[i1];
        for(int jj = 0; jj < 4; jj++) {
            int jj1 = (jj + 1) % 4;
            if(is2SegmentCollision(sp,ep,*rect[jj],*rect[jj1],i)) {
                return 0;
            }
        }

        sp.v[i0] -= lv.v[i0];
        ep.v[i1] -= lv.v[i1];
        for(int jj = 0; jj < 4; jj++) {
            int jj1 = (jj + 1) % 4;
            if(is2SegmentCollision(sp,ep,*rect[jj],*rect[jj1],i)) {
                return 0;
            }
        }
        return -1;
    }

    int num_inVS = 0;
    Point3Dd pv;
    Point3Dd ortho[2][2][2];
    for(int ix=0; ix<2; ix++) {
        pv.v[0] = ov.v[0] + lv.v[0] * ix;
        for(int iy=0; iy<2; iy++) {
            pv.v[1] = ov.v[1] + lv.v[1] * iy;
            for(int iz=0; iz<2; iz++) {
                pv.v[2] = ov.v[2] + lv.v[2] * iz;
                pv.orthoToPlane(*rect[0],nv,ortho[ix][iy][iz]);
                if (isPointInConvexPolygon3D(ortho[ix][iy][iz],rect,nv)) {
                    //return true;
                    num_inVS++;
                }
            }
        }
    } // for ix
    if (8 == num_inVS) {
        return 1;
    }
    if (num_inVS > 0) {
        return 0;
    }

    Point3Dd cen(rect[2]);
    cen += *rect[0];
    cen /= 2;
    if (isBoxCollisionWithLine(cen,nv,ov,lv.v[0])) {
        return 0;
    }

    int ind[3];
    if (nv.v[0] > 0) {
        ind[0] = 1;
    } else {
        ind[0] = 0;
    }
    if(nv.v[1] > 0) {
        ind[1] = 1;
    } else {
        ind[1] = 0;
    }
    if (nv.v[2] > 0) {
        ind[2] = 1;
    } else {
        ind[2] = 0;
    }
    int nInd = nv.getIndexOfMaxAbsCoord();
    Point3Dd *sp, *ep;
    for(int i0 = 0; i0 < 3; ++i0) {
        int ind1[3] = {ind[0],ind[1],ind[2]};
        ind1[i0] = (ind[i0] + 1) % 2;
        sp = &ortho[ind1[0]][ind1[1]][ind1[2]];
        for (int i1 = 0; i1 < 3; ++i1) {
            if (i1 == i0) {
                continue;
            }
            int ind2[3] = {ind1[0],ind1[1],ind1[2]};
            ind2[i1]++;
            ind2[i1] = ind2[i1] % 2;
            ep = &ortho[ind2[0]][ind2[1]][ind2[2]];
            if (is2SegmentCollision(*sp,*ep,*rect[0],*rect[2],nInd)) {
                return 0;
            }
        }
    }
    return -1;
}

bool Util::is2DSegmentAabbCollision(const double sp[2], const double ep[2],
                                    const double ov[2], const double lv)
{
    // bounding box of edge
    double mincoord[2] = {sp[0],sp[1]};
    double maxcoord[2] = {mincoord[0],mincoord[1]};
    if(sp[0] < ep[0]) {
        maxcoord[0] = ep[0];
    } else {
        mincoord[0] = ep[0];
    }
    if(sp[1] < ep[1]) {
        maxcoord[1] = ep[1];
    } else {
        mincoord[1] = ep[1];
    }

    if(ov[0] > maxcoord[0] || ov[1] > maxcoord[1] ||
            (ov[0] + lv) < mincoord[0] || (ov[1] + lv) < mincoord[1]) {
        return false;
    }
    double dv[2]= {ep[0],ep[1]};
    dv[0] -= sp[0];
    dv[1] -= sp[1];

    double v1[2] = {ov[0], ov[1]};
    double v2[2] = {ov[0], ov[1]};
    if(dv[0]*dv[1] > 0) { // quarter angle 1 or 3
        v1[0] += lv;
        v2[1] += lv;
    } else { // quarter angle 2 or 4
        v2[0] += lv;
        v2[1] += lv;
    }
    v1[0] -= sp[0];
    v1[1] -= sp[1];
    v2[0] -= sp[0];
    v2[1] -= sp[1];
    double d1 = dv[0]*v1[1] - dv[1]*v1[0];
    double d2 = dv[0]*v2[1] - dv[1]*v2[0];
    if(d1*d2 > 0) {
        return false;
    }

    return true;
}


bool Util::is2DSegmentAabbCollisionf(const float sp[2], const float ep[2],
    const float ov[2], const float lv)
{
    // bounding box of edge
    float mincoord[2] = { sp[0], sp[1] };
    float maxcoord[2] = { mincoord[0], mincoord[1] };
    if (sp[0] < ep[0]) {
        maxcoord[0] = ep[0];
    }
    else {
        mincoord[0] = ep[0];
    }
    if (sp[1] < ep[1]) {
        maxcoord[1] = ep[1];
    }
    else {
        mincoord[1] = ep[1];
    }

    if (ov[0] > maxcoord[0] || ov[1] > maxcoord[1] ||
        (ov[0] + lv) < mincoord[0] || (ov[1] + lv) < mincoord[1]) {
        return false;
    }
    float dv[2] = { ep[0], ep[1] };
    dv[0] -= sp[0];
    dv[1] -= sp[1];

    float v1[2] = { ov[0], ov[1] };
    float v2[2] = { ov[0], ov[1] };
    if (dv[0] * dv[1] > 0) { // quarter angle 1 or 3
        v1[0] += lv;
        v2[1] += lv;
    }
    else { // quarter angle 2 or 4
        v2[0] += lv;
        v2[1] += lv;
    }
    v1[0] -= sp[0];
    v1[1] -= sp[1];
    v2[0] -= sp[0];
    v2[1] -= sp[1];
    float d1 = dv[0] * v1[1] - dv[1] * v1[0];
    float d2 = dv[0] * v2[1] - dv[1] * v2[0];
    if (d1*d2 > 0) {
        return false;
    }

    return true;
}

bool Util::is2DLineAabbCollision(const double p[2], const double dv[2],
                                 const double ov[2], const double lv)
{
    if(fabs(dv[0]) < EPSILON_VAL_BIG) {
        if(ov[0] > p[0] || (ov[0] + lv) < p[0]) {
            return false;
        }
        return true;
    } else if(fabs(dv[1]) < EPSILON_VAL_BIG) {
        if(ov[1] > p[1] || (ov[1] + lv) < p[1]) {
            return false;
        }
        return true;
    }
    double lv_2 = lv/2.0;
    if(fabs(dv[0]) > fabs(dv[1])) {
        double xmid = ov[0] + lv_2;
        double ymid = p[1] + dv[1]*(xmid - p[0])/dv[0];
        double dy = lv_2*dv[1]/dv[0];
        if(fabs(ov[1] + lv_2 - ymid) > (lv_2 + fabs(dy))) {
            return false;
        }
        return true;
    } else {
        double ymid = ov[1] + lv_2;
        double xmid = p[0] + dv[0]*(ymid - p[1])/dv[1];
        double dx = lv_2*dv[0]/dv[1];
        if(fabs(ov[0] + lv_2 - xmid) > (lv_2 + fabs(dx))) {
            return false;
        }
        return true;
    }
    return false;
}

/* Calculate points of intersection between line and align axis bounding box.
   @param $p - a point on line
   @param $dir - direction vector of line
   @param $mincoord - minimum coordinate of box
   @param $maxcoord - maximum coordinate of box
   @param $intP - points of intersection(if exist)
   @return number of intersection point.
   @author Nghi
*/
int Util::calcIntPointsLineAABB(const double p[3], const double dir[3], const double mincoord[3],
                          const double maxcoord[3], double intP[2][3])
{
    int nint = 0;
    double t,val1,val2;

    if(fabs(dir[0]) > EPSILON_VAL_){ // calculate intersection with faces parallel with yz-plane
        t = (mincoord[0] - p[0])/dir[0];
        val1 = p[1] + dir[1]*t;
        if(val1 > mincoord[1] && val1 < maxcoord[1]){
            val2 = p[2] + dir[2]*t;
            if(val2 > mincoord[2] && val2 < maxcoord[2]){
                intP[nint][0] = mincoord[0];
                intP[nint][1] = val1;
                intP[nint][2] = val2;
                nint++;
            }
        }

        t = (maxcoord[0] - p[0])/dir[0];
        val1 = p[1] + dir[1]*t;
        if(val1 > mincoord[1] && val1 < maxcoord[1]){
            val2 = p[2] + dir[2]*t;
            if(val2 > mincoord[2] && val2 < maxcoord[2]){
                intP[nint][0] = maxcoord[0];
                intP[nint][1] = val1;
                intP[nint][2] = val2;
                nint++;
            }
        }
        if(2 == nint){
            return 2;
        }
    }

    if(fabs(dir[1]) > EPSILON_VAL_){ // calculate intersection with faces parallel with zx-plane
        t = (mincoord[1] - p[1])/dir[1];
        val1 = p[2] + dir[2]*t;
        if(val1 > mincoord[2] && val1 < maxcoord[2]){
            val2 = p[0] + dir[0]*t;
            if(val2 > mincoord[0] && val2 < maxcoord[0]){
                intP[nint][1] = mincoord[1];
                intP[nint][2] = val1;
                intP[nint][0] = val2;
                nint++;
            }
        }

        if(2 == nint){
            return 2;
        }
        t = (maxcoord[1] - p[1])/dir[1];
        val1 = p[2] + dir[2]*t;
        if(val1 > mincoord[2] && val1 < maxcoord[2]){
            val2 = p[0] + dir[0]*t;
            if(val2 > mincoord[0] && val2 < maxcoord[0]){
                intP[nint][1] = maxcoord[1];
                intP[nint][2] = val1;
                intP[nint][0] = val2;
                nint++;
            }
        }
        if(2 == nint){
            return 2;
        }
    }

    if(fabs(dir[2]) > EPSILON_VAL_){ // calculate intersection with faces parallel with xy-plane
        t = (mincoord[2] - p[2])/dir[2];
        val1 = p[0] + dir[0]*t;
        if(val1 > mincoord[0] && val1 < maxcoord[0]){
            val2 = p[1] + dir[1]*t;
            if(val2 > mincoord[1] && val2 < maxcoord[1]){
                intP[nint][2] = mincoord[2];
                intP[nint][0] = val1;
                intP[nint][1] = val2;
                nint++;
            }
        }

        if(2 == nint){
            return 2;
        }
        t = (maxcoord[2] - p[2])/dir[2];
        val1 = p[0] + dir[0]*t;
        if(val1 > mincoord[0] && val1 < maxcoord[0]){
            val2 = p[1] + dir[1]*t;
            if(val2 > mincoord[1] && val2 < maxcoord[1]){
                intP[nint][2] = maxcoord[2];
                intP[nint][0] = val1;
                intP[nint][1] = val2;
                nint++;
            }
        }
        if(2 == nint){
            return 2;
        }
    }
    return nint;
}
bool Util::is2DTriangleAabbCollision(const double v1[2], const double v2[2],
                                     const double v3[2],const double ov[2], const double &lv)
{
    // bounding box of triangle
    double mincoord[2],maxcoord[2];
    get2DBoundingBoxOf3Point(v1,v2,v3,mincoord,maxcoord);
    if(ov[0] > maxcoord[0] || ov[1] > maxcoord[1] ||
            (ov[0] + lv) < mincoord[0] || (ov[1] + lv) < mincoord[1]) {
        return false;
    }

    const double *vps[3] = {v1,v2,v3};
    for(int i = 0; i < 3; ++i) {
        int j = (i+1) % 3;
        double dv[2]= {vps[j][0],vps[j][1]};
        dv[0] -= vps[i][0];
        dv[1] -= vps[i][1];

        // case of edge ij parallel with axis
        if(fabs(dv[0]) < EPSILON_VAL_BIG || fabs(dv[1]) < EPSILON_VAL_BIG) {
            return true;
        }

        double dv1[2] = {ov[0], ov[1]};
        double dv2[2] = {ov[0], ov[1]};
        if(dv[0]*dv[1] > 0) { // quarter angle 1 or 3
            dv1[0] += lv;
            dv2[1] += lv;
        } else { // quarter angle 2 or 4
            dv2[0] += lv;
            dv2[1] += lv;
        }
        dv1[0] -= vps[i][0];
        dv1[1] -= vps[i][1];
        dv2[0] -= vps[i][0];
        dv2[1] -= vps[i][1];
        double d1 = dv[0]*dv1[1] - dv[1]*dv1[0];
        double d2 = dv[0]*dv2[1] - dv[1]*dv2[0];
        if(d1*d2 > 0) {
            j = (i + 2) % 3;
            double dv3[2] = {vps[j][0], vps[j][1]};
            dv3[0] -= vps[i][0];
            dv3[1] -= vps[i][1];
            double d3 = dv[0]*dv3[1] - dv[1]*dv3[0];
            // assert(fabs(d3) > EPSILON_VAL_E4);
            if(d1*d3 < EPSILON_VAL_) {
                return false;
            }
        }
    }// for i

    return true;
}

bool Util::calc2DIntOxLineWithSegment(const double *sp, const double *ep,
                                      const double &ycoord, double &intXCoord)
{
    if(ycoord > max2(sp[1],ep[1]) + EPSILON_VAL_ ||
            ycoord < min2(sp[1],ep[1]) - EPSILON_VAL_) {
        return false;
    }
    if(fabs(sp[1] - ep[1]) < EPSILON_VAL_) {
        return false;
    }
    if(fabs(sp[0] - ep[0]) < EPSILON_VAL_) {
        intXCoord = sp[0];
        return true;
    }
    intXCoord = (sp[1] - ycoord)*(ep[0] - sp[0])/(sp[1] - ep[1]);
    intXCoord += sp[0];
    return true;
}

bool Util::calc2DIntOyLineWithSegment(const double *sp, const double *ep,
                                      const double &xcoord, double &intYCoord)
{
    if(xcoord > max2(sp[0],ep[0]) + EPSILON_VAL_ ||
            xcoord < min2(sp[0],ep[0]) - EPSILON_VAL_) {
        return false;
    }
    if(fabs(sp[0] - ep[0]) < EPSILON_VAL_) {
        return false;
    }
    if(fabs(sp[1] - ep[1]) < EPSILON_VAL_) {
        intYCoord = sp[1];
        return true;
    }
    intYCoord = (sp[0] - xcoord)*(ep[1] - sp[1])/(sp[0] - ep[0]);
    intYCoord += sp[1];
    return true;
}

int Util::get2DBoundingBoxOf3Point(const double v1[2], const double v2[2],
                                   const double v3[2], double mincoord[2], double maxcoord[2])
{
    mincoord[0] = v1[0];
    mincoord[1] = v1[1];
    maxcoord[0] = v1[0];
    maxcoord[1] = v1[1];
    if(v2[0] < mincoord[0]) {
        mincoord[0] = v2[0];
    } else {
        maxcoord[0] = v2[0];
    }
    if(v2[1] < mincoord[1]) {
        mincoord[1] = v2[1];
    } else {
        maxcoord[1] = v2[1];
    }
    if(v3[0] < mincoord[0]) {
        mincoord[0] = v3[0];
    } else if(v3[0] > maxcoord[0]) {
        maxcoord[0] = v3[0];
    }
    if(v3[1] < mincoord[1]) {
        mincoord[1] = v3[1];
    } else if(v3[1] > maxcoord[1]) {
        maxcoord[1] = v3[1];
    }
    return 0;
}

/* calculate cricumcentre of a 2D triangle*/
int Util::calc2DCircumcentre(const double *p1,
                             const double *p2, const double *p3, double *centre)
{
    double xm = p1[0] + p2[0]; //  /2.0
    double ym = p1[1] + p2[1]; //  /2.0
    double xn = p1[0] + p3[0]; //  /2.0
    double yn = p1[1] + p3[1]; //  /2.0

    double dx2 = p1[0] - p2[0];
    double dy2 = p1[1] - p2[1];
    double dx1 = p1[0] - p3[0];
    double dy1 = p1[1] - p3[1];

    double del = dx2*dy1 - dx1*dy2;
    assert(fabs(del) > EPSILON_VAL_);

    double delx = (xm*dx2 + ym*dy2)*dy1 - (xn*dx1 + yn*dy1)*dy2;
    double dely = (xn*dx1 + yn*dy1)*dx2 - (xm*dx2 + ym*dy2)*dx1;
    centre[0] = delx/del/2.0;
    centre[1] = dely/del/2.0;
    return 0;
}

/**********************************************************************************************//**
 * @fn   static double Util::calcSquareDistancePointToSegmentIncludeCase(const double p[3],
 *       const double sp[3], const double ep[3])
 *
 * @brief   Calculates the square distance point to segment include case.
 *
 * @author  Nghi
 * @date 9/27/2012
 *
 * @param   p  The point
 * @param   sp The start point of segment.
 * @param   ep The end point of segment.
 *
 * @return  The calculated square distance point to segment (incase of orthogonal projection
 * 			of point on segment is inside of segment). Incase of orthogonal projection
 * 			of point on segment is exclude of segment return HUGE.
 **************************************************************************************************/

double Util::calcSquareDistancePointToSegmentIncludeCase(const double p[3],
        const double sp[3], const double ep[3])
{
    double v[3] = {ep[0]-sp[0],ep[1]-sp[1],ep[2]-sp[2]};
    double s[3] = {p[0] - sp[0],p[1] - sp[1],p[2] - sp[2]};
    double num = v[0]*s[0] + v[1]*s[1] + v[2]*s[2];
    double den = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if(den < EPSILON_VAL_MINI) {
        return HUGE;
    }
    double t = num/den;
    if(t >= 1 || t <= 0.0) {
        return HUGE;
    }
    double ss = s[0]*s[0] + s[1]*s[1] + s[2]*s[2];
    return (ss + den*t*t - 2.0*t*num);
}

/**********************************************************************************************//**
 * @fn   bool Util::isOrthorOfPointIncludeSegment(const double p[3], const double sp[3],
 *       const double ep[3])
 *
 * @brief   Query if orthogonal projection of 'p' on segment is include segment.
 *
 * @author  Nghi
 * @date 9/27/2012
 *
 * @param   p  The point
 * @param   sp The start point of segment.
 * @param   ep The end point of segment.
 *
 * @return  true if orthor of point include segment, false if not.
 **************************************************************************************************/

bool Util::isOrthorOfPointIncludeSegment(const double p[3],
        const double sp[3], const double ep[3])
{
    double v[3] = {ep[0]-sp[0],ep[1]-sp[1],ep[2]-sp[2]};
    double s[3] = {p[0] - sp[0],p[1] - sp[1],p[2] - sp[2]};
    double num = v[0]*s[0] + v[1]*s[1] + v[2]*s[2];
    double den = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    assert(den > EPSILON_VAL_);
    double t = num/den;
    if(t >= 1 || t <= 0.0) {
        return false;
    }
    return true;
}


/** Check collision between circle and triangle in 2D
*   @param pv - center of circle
*   @param r - radius of circle
*   @param trig - contain three vertices of triangle
*   @return true if collision
*   @author Nghi
*/
//int circle_trig_col_count = 0;
bool Util::is2DCircleTriangleCollision(const double pv[2],
                                 const double r,
                                 const double trig[3][2])
{
    //circle_trig_col_count++;
    double r2 = r*r;
    if (is2DPointInTriangle(pv,trig)){
        return true;
    }

    for (int i = 0; i < 3; ++i){
        int j = (i + 1) % 3;
        double dis2 = calc2DSquareMinDistancePointSegment(pv,trig[i],trig[j]);
        if ( dis2 < r2){
            return true;
        }
    }
    return false;
}

// calculate number intersection point between a ray throught pv and parallel with ox axis 
// with edges of triangle

bool Util::is2DPointInTriangle(const double pv[2], const double trig[3][2])
{
    double v1[2] = {pv[0] - trig[0][0],pv[1] - trig[0][1]};
    double v2[2] = {pv[0] - trig[1][0],pv[1] - trig[1][1]};
    double dot = v1[0]*v2[1] - v1[1]*v2[0];
    if (dot < -EPSILON_VAL_){
        return false;
    }
    double v3[2] = {pv[0] - trig[2][0],pv[1] - trig[2][1]};
    dot = v2[0]*v3[1] - v2[1]*v3[0];
    if (dot < -EPSILON_VAL_){
        return false;
    }

    dot = v3[0]*v1[1] - v3[1]*v1[0];
    if (dot < -EPSILON_VAL_){
        return false;
    }
    return true;
}



// Rotate a coordinate system to be perpendicular to the given normal
void Util::rot_coord_sys(const Vector3Dd &old_u, const Vector3Dd &old_v,
                         const Vector3Dd &new_norm, Vector3Dd &new_u, Vector3Dd &new_v)
{
    new_u = old_u;
    new_v = old_v;
    Vector3Dd old_norm = old_u.crossProduct(old_v);
    float ndot = (float)old_norm.scalarProduct(new_norm);
    if ((ndot <= -1.0f + EPSILON_VAL_)) {// invert direction
        new_u = -new_u;
        new_v = -new_v;
        return;
    }
    Vector3Dd perp_old = new_norm - old_norm * ndot ;
    Vector3Dd dperp = (old_norm + new_norm) * (1.0/(1 + ndot));
    new_u -= dperp * (new_u.scalarProduct(perp_old));
    new_v -= dperp * (new_v.scalarProduct(perp_old));
}

// Reproject a curvature tensor from the basis spanned by old_u and old_v
// (which are assumed to be unit-length and perpendicular) to the
// new_u, new_v basis.
void Util::proj_curv(const Vector3Dd &old_u, const Vector3Dd &old_v,
                     float old_ku, float old_kuv, float old_kv,
                     const Vector3Dd &new_u, const Vector3Dd &new_v,
                     float &new_ku, float &new_kuv, float &new_kv)
{
    Vector3Dd r_new_u, r_new_v;
    rot_coord_sys(new_u, new_v, old_u.crossProduct(old_v), r_new_u, r_new_v);

    float u1 = (float)r_new_u.scalarProduct(old_u);
    float v1 = (float)r_new_u.scalarProduct(old_v);
    float u2 = (float)r_new_v.scalarProduct(old_u);
    float v2 = (float)r_new_v.scalarProduct(old_v);
    new_ku  = old_ku * u1*u1 + old_kuv * (2.0f  * u1*v1) + old_kv * v1*v1;
    new_kuv = old_ku * u1*u2 + old_kuv * (u1*v2 + u2*v1) + old_kv * v1*v2;
    new_kv  = old_ku * u2*u2 + old_kuv * (2.0f  * u2*v2) + old_kv * v2*v2;
}

// Like the above, but for dcurv
void Util::proj_dcurv(const Vector3Dd &old_u, const Vector3Dd &old_v,
                      const Vector4Df old_dcurv,
                      const Vector3Dd &new_u, const Vector3Dd &new_v,
                      Vector4Df &new_dcurv)
{
    Vector3Dd r_new_u, r_new_v;
    rot_coord_sys(new_u, new_v, old_u.crossProduct(old_v), r_new_u, r_new_v);

    float u1 = (float)r_new_u.scalarProduct(old_u);
    float v1 = (float)r_new_u.scalarProduct(old_v);
    float u2 = (float)r_new_v.scalarProduct(old_u);
    float v2 = (float)r_new_v.scalarProduct(old_v);

    new_dcurv[0] = old_dcurv[0]*u1*u1*u1 +
                   old_dcurv[1]*3.0f*u1*u1*v1 +
                   old_dcurv[2]*3.0f*u1*v1*v1 +
                   old_dcurv[3]*v1*v1*v1;
    new_dcurv[1] = old_dcurv[0]*u1*u1*u2 +
                   old_dcurv[1]*(u1*u1*v2 + 2.0f*u2*u1*v1) +
                   old_dcurv[2]*(u2*v1*v1 + 2.0f*u1*v1*v2) +
                   old_dcurv[3]*v1*v1*v2;
    new_dcurv[2] = old_dcurv[0]*u1*u2*u2 +
                   old_dcurv[1]*(u2*u2*v1 + 2.0f*u1*u2*v2) +
                   old_dcurv[2]*(u1*v2*v2 + 2.0f*u2*v2*v1) +
                   old_dcurv[3]*v1*v2*v2;
    new_dcurv[3] = old_dcurv[0]*u2*u2*u2 +
                   old_dcurv[1]*3.0f*u2*u2*v2 +
                   old_dcurv[2]*3.0f*u2*v2*v2 +
                   old_dcurv[3]*v2*v2*v2;
}

// Given a curvature tensor, find principal directions and curvatures
// Makes sure that pdir1 and pdir2 are perpendicular to normal
void Util::diagonalize_curv(const Vector3Dd &old_u, const Vector3Dd &old_v,
                            float ku, float kuv, float kv,
                            const Vector3Dd &new_norm,
                            Vector3Dd &pdir1, Vector3Dd &pdir2, float &k1, float &k2)
{
    Vector3Dd r_old_u, r_old_v;
    rot_coord_sys(old_u, old_v, new_norm, r_old_u, r_old_v);

    float c = 1, s = 0, tt = 0;
    if (likely(kuv != 0.0f)) {
        // Jacobi rotation to diagonalize
        float h = 0.5f * (kv - ku) / kuv;
        tt = (h < 0.0f) ?
             1.0f / (h - sqrt(1.0f + h*h)) :
             1.0f / (h + sqrt(1.0f + h*h));
        c = 1.0f / sqrt(1.0f + tt*tt);
        s = tt * c;
    }

    k1 = ku - tt * kuv;
    k2 = kv + tt * kuv;

    if (fabs(k1) >= fabs(k2)) {
        pdir1 = r_old_u * c - r_old_v * s;
    } else {
        std::swap(k1, k2);
        pdir1 = r_old_u * s + r_old_v * c;
    }
    pdir2 = new_norm.crossProduct(pdir1);
}

/* solve linear equation order 2
   a*x^2 + b*x + c = 0
   @param roots - contain roots of equation
   @return number of root
**/
int Util::solveEquationOrder2(double a, double b, double c, double* roots)
{
    if(fabs(a) < EPSILON_VAL_){
        if(fabs(b) < EPSILON_VAL_){
            return 0;
        }
        roots[0] = -c/b;
        return 1;
    }

    double delta = b*b - 4*a*c;
    if(fabs(delta) < EPSILON_VAL_){
        roots[0] = -b/a/2.0;
        return 1;
    }
    else if(delta < 0.0){
        return 0;
    }

    delta = sqrt(delta);
    a *= 2.0;
    roots[0] = (-b + delta)/a;
    roots[1] = (-b - delta)/a;
    return 2;
}


/*
  @brief  Calculate points of intersection between line segment with circle.
  @param  sp - start point of line segment
  @param  ep - end point of line segment
  @param  center - a center point of circle
  @param  v - normal vector of circle (normal vector of plane that contain circle)
  @param  radius - radius of cylinder
  @param  t - parameter of intersection point (output)
  @author  Nghi
  @date 07/11/2013
  @return  number of intersection point
*/
int Util::calcIntSegmentCircle(const Point3Dd &sp, const Point3Dd &ep,
                           const Point3Dd &center, const Vector3Dd &v, const double &radius, double *t)
{
    Vector3Dd n(ep);
    n -= sp;
    
    Vector3Dd u(sp);
    u -= center;
    double a = n.scalarProduct(n);
    double b = 2.0*n.scalarProduct(u);
    double c = u.scalarProduct(u) - radius*radius;
    return solveEquationOrder2(a,b,c,t);
}

/*
  @brief  Calculate points of intersection between line segment with cylinder.
  @param  sp - start point of line segment
  @param  ep - end point of line segment
  @param  center - a center point of cylinder (a point on center line)
  @param  v - direction vector of center line of cylinder
  @param  radius - radius of cylinder
  @param  t - parameter of intersection point (output)
  @author  Nghi
  @date 07/11/2013
  @return  number of intersection point
*/
#if 0
int Util::calcIntSegmentCylinder(const Point3Dd &sp, const Point3Dd &ep,
                           const Point3Dd &center, const Vector3Dd &v, const double &radius, double *t)
{
    Vector3Dd n(ep);
    n -= sp;
    if(n.scalarProduct(v) < EPSILON_VAL_E3){
        Vector3Dd n1(n);
        n1.unit();
        if(n1.scalarProduct(v) < EPSILON_VAL_){
            Point3Dd cen = center + v*(v.scalarProduct(sp - center));
            Vector3Dd u(sp);
            u -= cen;
            double a = n.scalarProduct(n);
            double b = 2.0*n.scalarProduct(u);
            double c = u.scalarProduct(u) - radius*radius;
            return solveEquationOrder2(a,b,c,t);
        }

    }
    Vector3Dd m(center);
    m -= sp;
    
    double beta = n.scalarProduct(v);
    double gamma = m.scalarProduct(v);
    double xi = n.scalarProduct(n);
    double lambda = m.scalarProduct(m);
    double eta = m.scalarProduct(n);
    double a = xi - beta*beta;
    double b = 2.9*(gamma*beta - eta);
    double c = lambda - gamma*gamma - radius*radius;
    return solveEquationOrder2(a,b,c,t);
}

#else

int Util::calcIntSegmentCylinder(const Point3Dd &sp, const Point3Dd &ep,
                                 const Point3Dd &center, const Vector3Dd &v, const double &radius, double *t)
{
    Point3Dd sp1,ep1;
    sp.orthoToPlane(center,v,sp1);
    ep.orthoToPlane(center,v,ep1);

    return calcIntSegmentCircle(sp1,ep1,center,v,radius,t);
}

#endif
/*
  @brief  Calculate points of intersection between line segment with cone.
  @param  sp - start point of line segment
  @param  ep - end point of line segment
  @param  C - a center point of cylinder (a point on center line)
  @param  v - direction vector of center line of cylinder ( must be unit vector)
  @param  cangle - cosine of angle at tip of cone (cangle = cos(angle))
  @param  t - parameter of intersection point (output)
  @author  Nghi
  @date 07/11/2013
  @return  number of intersection point


                          C
                         /|\
                       /  |  \
                     / *  |    \
                   /    * |      \ P
                 /  angle |        \
               /          |          \
             /            |            \
                         \|/
                          v V

          (V*(P - C))^2 = cangle^2*|P-C|^2
*/
int Util::calcIntSegmentCone(const Point3Dd &sp, const Point3Dd &ep,
                       const Point3Dd &C, const Vector3Dd &v, const double &cangle, double *t)
{
    Vector3Dd n(ep);
    n -= sp;

    if(n.scalarProduct(v) < EPSILON_VAL_E3){
        Vector3Dd n1(n);
        n1.unit();
        if(n1.scalarProduct(v) < EPSILON_VAL_){
            Point3Dd u0(sp);
            u0 -= C;
            double leng = u0.scalarProduct(v);
            if(leng < 0){
                return 0;
            }
            double radius_2 = (1.0 - cangle*cangle)*leng*leng/cangle/cangle;
            Point3Dd cen = C + v*leng;
            Vector3Dd u(sp);
            u -= cen;
            double a = n.scalarProduct(n);
            double b = 2.0*n.scalarProduct(u);
            double c = u.scalarProduct(u) - radius_2;
            return solveEquationOrder2(a,b,c,t);
        }
    }

    Vector3Dd m(sp);
    m -= C;

    double alpha = m.scalarProduct(v);
    double beta = n.scalarProduct(v);
    double gamma = m.scalarProduct(m);
    double lambda = m.scalarProduct(n);
    double eta = n.scalarProduct(n);
    double cangle_2 = cangle*cangle;
    double a = beta*beta - eta*cangle_2;
    double b = (alpha*beta - lambda*cangle_2)*2.0;
    double c = alpha*alpha - gamma*cangle_2;
    
    int num = solveEquationOrder2(a,b,c,t);
    if(0 == num){
        return 0;
    }
    else if(1 == num){
        if(t[0] > -alpha/beta){
            return 1;
        }
        return 0;
    }

    if(t[0] > -alpha/beta){
        if(t[1] > -alpha/beta){
            return 2;
        }
        return 1;
    }

    if(t[1] > -alpha/beta){
        t[0] = t[1];
        return 1;
    }

    return 0;
}

bool Util::isPointInsideCone(const Point3Dd &p, const double &tipR, const double &baseR,
                             const Point3Dd &TipCenter, const Point3Dd &baseCenter)
{

    double r;
    Vector3Dd dir = TipCenter - baseCenter;
    double length = dir.unit();
    if(tipR < EPSILON_VAL_){
        Vector3Dd v(p);
        v -= TipCenter;
        double leng = v.scalarProduct(dir);

        if(leng > 0.0 || leng < -length){
            return false;
        }
        r = -baseR*leng/length;
    }
    else if(baseR < EPSILON_VAL_){
        Vector3Dd v(p);
        v -= baseCenter;
        double leng = v.scalarProduct(dir);
        if(leng < 0.0 || leng > length){
            return false;
        }
        r = tipR*leng/length;
    }
    else{
        Vector3Dd v(p);
        v -= baseCenter;
        double leng = v.scalarProduct(dir);
        if(leng < 0.0 || leng > length){
            return false;
        }
        r = baseR - (baseR - tipR)*leng/length;
    }
    double dis2 = p.calcSquareDistanceToLine(baseCenter,dir);

    if(dis2 > (r + EPSILON_VAL_BIG)*(r + EPSILON_VAL_BIG)){
        return false;
    }

    return true;
}

/*
  Calculate square minimum distance between segment and line.

  @param $sp - start point of segment
  @param $ep - end point of segment
  @param $pv - a point on line
  @param $dir - direction vector of line
  @return the square minimum distance
  @author Nghi
*/
double Util::calcSquareMinDistanceSegmentLine(const Point3Dd &sp, const Point3Dd &ep,
                                               const Point3Dd &pv, const Point3Dd &dir)
{
    Point3Dd dv(ep);
    dv -= sp;
    Vector3Dd v1 = dv*dir;
    if(v1.unit() < EPSILON_VAL_BIG){
        return sp.calcSquareDistanceToLine(pv,dir);
    }
    Vector3Dd nv = v1*dv;
    Point3Dd intp;
    calcIntLinePlane(pv,dir,sp,nv,intp);
    Vector3Dd dv1(intp);
    dv1 -= sp;
    Vector3Dd dv2(intp);
    dv2 -= ep;
    double d1 = dv1.scalarProduct(dv);
    double d2 = dv2.scalarProduct(dv);
    if(d1*d2 < EPSILON_VAL_){
        return intp.calcSquareDistanceToLine(sp,dv);
    }
    if(d1 > 0.0){
        return ep.calcSquareDistanceToLine(pv,dir);
    }
    return sp.calcSquareDistanceToLine(pv,dir);
}

/*
  Calculate square minimum distance between two lines.
  @param $pv1 - a point on first line
  @param $dir1 - direction vector of first line
  @param $pv2 - a point on second line
  @param $dir2 - direction vector of second line
  @return the square minimum distance
  @author Nghi
*/
double Util::calcSquareMinDistanceTwoLine(const Point3Dd &pv1, const Point3Dd &dir1,
                                           const Point3Dd &pv2, const Point3Dd &dir2)
{
    Vector3Dd nv = dir1*dir2;
    if(nv.unit() < EPSILON_VAL_BIG){
        return pv1.calcSquareDistanceToLine(pv2,dir2);
    }

    return pv1.squareDistanceToPlane(pv2,nv);
}

/* Calculate point on a line that is nearest from other.
   (the point is on first line)
   @param $p - a point on first line
   @param $dv - direction vector of first line
   @param $otherP - a point on other line
   @param $otherdv - direction vector of other line
   @param $intp - output point
   @return 'true' if exist point.
*/
bool Util::calcNearestPointFromLineToOtherLine(const Point3Dd &p, const Vector3Dd &dv,
                                         const Point3Dd &otherP, const Vector3Dd &otherdv, Point3Dd &intp)
{
    Vector3Dd nv = dv*otherdv;
    if(nv.abs2() < EPSILON_VAL_){
        intp = p;
        return false;
    }
    Vector3Dd nv1 = nv*otherdv;
    calcIntLinePlane(p,dv,otherP,nv1,intp);
    return true;
}

/* Calculate intersection between quadrangle and prism
   @param $quad - quadrangle
   @param $rect - boundary of prism
   @param $nv - normal vector of the boundary
   @param $intp - output point
   @return number of intersection points.
   @author Son
*/
int Util::calcIntQuadWithPrism(const std::vector<Point3Dd *> &quad,
                            const std::vector<Point3Dd *> &rect, 
                            const Vector3Dd &nv,
                            std::vector<Point3Dd> &intp)
{
    intp.clear();
    Point3Dd orthor[4];
    std::vector<Point3Dd *> vorthor;
    for (unsigned i = 0; i < quad.size(); i++)
    {
        quad[i]->orthoToPlane(*rect[0], nv, orthor[i]);
        vorthor.push_back(&orthor[i]);
    }

    BaseType::Util::calcIntQuadQuadCoplanar(vorthor, rect, intp);

    Vector3Dd v = (*quad[1] - *quad[0]) * (*quad[2] - *quad[1]);
    for (unsigned i = 0; i < intp.size(); i++)
    {
        BaseType::Util::calcIntLinePlane(intp[i], nv, *quad[0], v, intp[i]);
    }

    return intp.size();
}

/* Calculate intersection between Bounding box and prism
   @param $minCoord - the minimum coordinate of box
   @param $maxCoord - the maximum coordinate of box
   @param $rect - boundary of prism
   @param $nv - normal vector boundary
   @return number of intersection points.
   @author Son
*/
int Util::calcIntBBoxWithPrism(const Point3Dd &minCoord, const Point3Dd &maxCoord,
                               const std::vector<Point3Dd *> rect, const Point3Dd &nv,
                               std::vector<Point3Dd> &intp)
{
    std::vector<Point3Dd *> quad;
    std::vector<Point3Dd> quadInt;
    Point3Dd p[4];
    const Vector3Dd v0 = Point3Dd(1, 0, 0), 
        v1 = Point3Dd(0, 1, 0), 
        v2 = Point3Dd(0, 0 ,1);

    intp.clear();

    Point3Dd boxsize = maxCoord - minCoord;
    /* calc with xz plane */
    p[0] = minCoord;
    p[1] = minCoord + v2 * boxsize.z;
    p[2] = p[1] + v0 * boxsize.x;
    p[3] = p[2] - v2 * boxsize.z;
    quad.push_back(&p[0]);
    quad.push_back(&p[1]);
    quad.push_back(&p[2]);
    quad.push_back(&p[3]);
    calcIntQuadWithPrism(quad, rect, nv, quadInt);
    for (unsigned i = 0; i < quadInt.size(); i++)
        if (intp.end() == std::find(intp.begin(), intp.end(), quadInt[i])) {
            intp.push_back(quadInt[i]);
        }

    p[0] += v1 * boxsize.y;
    p[1] += v1 * boxsize.y;
    p[2] += v1 * boxsize.y;
    p[3] += v1 * boxsize.y;
    quad.clear();
    quad.push_back(&p[0]);
    quad.push_back(&p[1]);
    quad.push_back(&p[2]);
    quad.push_back(&p[3]);
    calcIntQuadWithPrism(quad, rect, nv, quadInt);
    for (unsigned i = 0; i < quadInt.size(); i++)
        if (intp.end() == std::find(intp.begin(), intp.end(), quadInt[i])) {
            intp.push_back(quadInt[i]);
        }

    /* calc with xy plane */
    p[0] = minCoord;
    p[1] = p[0] + v1 * boxsize.y;
    p[2] = p[1] + v0 * boxsize.x;
    p[3] = p[2] - v1 * boxsize.y;
    quad.clear();
    quad.push_back(&p[0]);
    quad.push_back(&p[1]);
    quad.push_back(&p[2]);
    quad.push_back(&p[3]);
    calcIntQuadWithPrism(quad, rect, nv, quadInt);
    for (unsigned i = 0; i < quadInt.size(); i++)
        if (intp.end() == std::find(intp.begin(), intp.end(), quadInt[i])) {
            intp.push_back(quadInt[i]);
        }

    p[0] += v2 * boxsize.z;
    p[1] += v2 * boxsize.z;
    p[2] += v2 * boxsize.z;
    p[3] += v2 * boxsize.z;
    quad.clear();
    quad.push_back(&p[0]);
    quad.push_back(&p[1]);
    quad.push_back(&p[2]);
    quad.push_back(&p[3]);
    calcIntQuadWithPrism(quad, rect, nv, quadInt);
    for (unsigned i = 0; i < quadInt.size(); i++)
        if (intp.end() == std::find(intp.begin(), intp.end(), quadInt[i])) {
            intp.push_back(quadInt[i]);
        }

    /* calc with yz plane */
    p[0] = minCoord;
    p[1] = p[0] + v2 * boxsize.z;
    p[2] = p[1] + v1 * boxsize.y;
    p[3] = p[2] - v2 *boxsize.z;
    quad.clear();
    quad.push_back(&p[0]);
    quad.push_back(&p[1]);
    quad.push_back(&p[2]);
    quad.push_back(&p[3]);
    calcIntQuadWithPrism(quad, rect, nv, quadInt);
    for (unsigned i = 0; i < quadInt.size(); i++)
        if (intp.end() == std::find(intp.begin(), intp.end(), quadInt[i])) {
            intp.push_back(quadInt[i]);
        }

    p[0] += v0 * boxsize.x;
    p[1] += v0 * boxsize.x;
    p[2] += v0 * boxsize.x;
    p[3] += v0 * boxsize.x;
    quad.clear();
    quad.push_back(&p[0]);
    quad.push_back(&p[1]);
    quad.push_back(&p[2]);
    quad.push_back(&p[3]);
    calcIntQuadWithPrism(quad, rect, nv, quadInt);
    for (unsigned i = 0; i < quadInt.size(); i++)
        if (intp.end() == std::find(intp.begin(), intp.end(), quadInt[i])) {
            intp.push_back(quadInt[i]);
        }

    return intp.size();
}

/**************************************************************************************************
 * Gets a major-minor pair from version string.
 *
 * @param	versionString  	The version string.
 * @param [in,out]	version	The version pair.
 *
 * @return	true if it succeeds, false if it fails.
 **************************************************************************************************/

bool BaseType::Util::getMajorMinorVersion(std::string const &versionString, VersionPair &version)
{
	std::vector<std::string> tokenList;
	if (2 != BaseType::splitString(versionString.c_str(), ".", tokenList))
		return false;

	bool isOk;
	version.first = stringToInt(tokenList.front(), &isOk);

	if (!isOk)
		return false;

	version.second = stringToInt(tokenList.back(), &isOk);

	if (!isOk)
		return false;

	return true;
}

Point3Dd BaseType::Util::calPointCoefficient(const Point3Dd& point, const Point3Dd& a, const Point3Dd& b, const Point3Dd& c)
{
    Point3Dd result;

    Point3Dd ab = b - a;
    Point3Dd ac = c - a;
    Point3Dd ea = a - point;
    Point3Dd eb = b - point;
    Point3Dd ec = c - point;

    double Sabc = abs((ab.crossProduct(ac)).length());
    double Seab = abs((ea.crossProduct(eb)).length());
    double Seac = abs((ea.crossProduct(ec)).length());
    double Sebc = abs((eb.crossProduct(ec)).length());

    result.z = Seab / Sabc;
    result.y = Seac / Sabc;
    result.x = Sebc / Sabc;

    double total = result.x + result.y + result.z;
    if ((total - 1) > EPSILON_VAL_E1)
    {
        result.x = 0;
        result.y = 0;
        result.z = 1;
    }

    return result;
}

/* Calculate eight vertices of box
   @param $minCoord - the minimum coordinate of box
   @param $maxCoord - the maximum coordinate of box
   @param $boxVertices - output eight vertices of th box
   @return true if successful
   @author Son
*/
bool BaseType::Util::calcEightVerticesOfBox(const Point3Dd& minCoord, const Point3Dd &maxCoord, Point3Dd boxVertices[8])
{
    int ret = 0;
    double size[3];
    double center[3];

    size[0] = maxCoord[0] - minCoord[0];
    size[1] = maxCoord[1] - minCoord[1];
    size[2] = maxCoord[2] - minCoord[2];

    center[0] = (maxCoord[0] + minCoord[0]) * 0.5;
    center[1] = (maxCoord[1] + minCoord[1]) * 0.5;
    center[2] = (maxCoord[2] + minCoord[2]) * 0.5;

    // Cube size x size x size, centered on center
    boxVertices[0] = Point3Dd(-0.5 * size[0] + center[0], -0.5 * size[1] + center[1], -0.5 * size[2] + center[2]);
    boxVertices[1] = Point3Dd(0.5 * size[0] + center[0], -0.5 * size[1] + center[1], -0.5 * size[2] + center[2]);
    boxVertices[2] = Point3Dd(0.5 * size[0] + center[0],  0.5 * size[1] + center[1], -0.5 * size[2] + center[2]);
    boxVertices[3] = Point3Dd(-0.5 * size[0] + center[0],  0.5 * size[1] + center[1], -0.5 * size[2] + center[2]);
    boxVertices[4] = Point3Dd(-0.5 * size[0] + center[0], -0.5 * size[1] + center[1],  0.5 * size[2] + center[2]);
    boxVertices[5] = Point3Dd(0.5 * size[0] + center[0], -0.5 * size[1] + center[1],  0.5 * size[2] + center[2]);
    boxVertices[6] = Point3Dd(0.5 * size[0] + center[0],  0.5 * size[1] + center[1],  0.5 * size[2] + center[2]);
    boxVertices[7] = Point3Dd(-0.5 * size[0] + center[0],  0.5 * size[1] + center[1],  0.5 * size[2] + center[2]);

    return true;
}

/* Calculate the bounding box after projected onto a plane
   @param $oldMincoord - the minimum coordinate of box before projected
   @param $oldMaxcoord - the maximum coordinate of box before projected
   @param $p - the point on plane
   @param $nv - the normal vector of plane
   @param $newMincoord - new minimum coord after projected on plane
   @param $newMaxcoord - new maximum coord after projected on plane
   @return true if successful
   @author Son
*/
bool BaseType::Util::calcNewBBoxAfterProjToPlane(const Point3Dd &oldMincoord, const Point3Dd &oldMaxcoord,
                                                 const Point3Dd &p, const Vector3Dd &nv,
                                                 Point3Dd &newMincoord, Point3Dd &newMaxcoord)
{
    Point3Dd boxVertices[8];
    calcEightVerticesOfBox(oldMincoord, oldMaxcoord, boxVertices);
    std::for_each(boxVertices, boxVertices + 8, [p, nv](Point3Dd &p3d) { p3d.orthoToPlane(p, nv, p3d);});
    double min[3], max[3];
    std::fill(min, min + 3, DBL_MAX);
    std::fill(max, max + 3, -DBL_MAX);
    
    for (unsigned i = 0; i < 8; ++i)
    {
        if (min[0] > boxVertices[i][0]) {
            min[0] = boxVertices[i][0];
        } else if (max[0] < boxVertices[i][0]) {
            max[0] = boxVertices[i][0];
        }

        if (min[1] > boxVertices[i][1]) {
            min[1] = boxVertices[i][1];
        } else if (max[1] < boxVertices[i][1]) {
            max[1] = boxVertices[i][1];
        }

        if (min[2] > boxVertices[i][2]) {
            min[2] = boxVertices[i][2];
        } else if (max[2] < boxVertices[i][2]) {
            max[2] = boxVertices[i][2];
        }
    }

    std::copy(min, min + 3, newMincoord.v);
    std::copy(max, max + 3, newMaxcoord.v);

    return true;
}

/* Create contours from list of segments
   @param segmentList - list of segments
   @param $contours - the contour list output
   @return number of contours
   @author Son
*/
int BaseType::Util::createContoursFromSegmentList(Segment3DdListVector &segmentList, 
                                                  std::deque<std::deque<Point3Dd *>> &contours)
{
    unsigned numsegment = segmentList.size();
    unsigned i = 0;
    std::vector<bool> checkedFlag(segmentList.size(), 0);

    for (auto it = segmentList.begin(); it != segmentList.end(); it++, i++) {
        if (checkedFlag[i]) continue;

        std::list<Point3Dd *> contour;
        contour.push_back(new Point3Dd(it->getFisrtPoint()));
        contour.push_back(new Point3Dd(it->getSecondPoint()));
        checkedFlag[i] = true;
        while (true) {
            unsigned k = 0;
            int count = 0;

            for (auto kt = segmentList.begin(); kt != segmentList.end(); kt++, k++) {
                if (checkedFlag[k]) continue;
                Point3Dd p1 = kt->getFisrtPoint();
                Point3Dd p2 = kt->getSecondPoint();
                Point3Dd p3 = *contour.back();
                Point3Dd p4 = *contour.front();
                if (p1.isStrongEqual(p3)) {
                    contour.push_back(new Point3Dd(p2));
                    count++;
                    checkedFlag[k] = true;
                } else if (p2.isStrongEqual(p4)) {
                    contour.push_front(new Point3Dd(p1));

                    count++;
                    checkedFlag[k] = true;
                }
            }

            if (*contour.back() == *contour.front() || count == 0) break;
        }
        
        contours.push_back(std::deque<Point3Dd *>(contour.begin(), contour.end()));
    }

    closeContours(contours);

    return true;
}

/* Close contours
   @param $contours - the list of contours
   @return number of contours
   @author Son
*/
int BaseType::Util::closeContours(std::deque<std::deque<Point3Dd *>> &contours)
{
    double mindis, dis, dis1;
    std::vector<bool> finished(contours.size(), false);
    std::vector<bool> deleted(contours.size(), false);

    std::deque<std::deque<Point3Dd *>> contoursHdl = contours;

    contours.clear();
    while (true)
    {
        int i = -1;
        unsigned numpoint = 0;
        bool loopout = true;
        /* find the longest contour */
        for (unsigned cc = 0; cc < contoursHdl.size(); cc++)
        {
            if (finished[cc] == false && deleted[cc] == false && 
                numpoint < contoursHdl[cc].size() && 
                *contoursHdl[cc].front() != *contoursHdl[cc].back() && 
                contoursHdl[cc].size() > 2) 
            {
                numpoint = contoursHdl[cc].size();
                i = cc;
            }
        }
        
        if (i == -1) break;

        std::deque<Point3Dd *> *icontour = &(contoursHdl[i]);
        Point3Dd sp = *icontour->front();
        Point3Dd ep = *icontour->back();
        if (sp == ep) {
            continue;
        }

        Vector3Dd vsp, vep;
        Point3Dd psp(*(*icontour)[1]), pep(*(*(icontour->end() - 2)));
        vsp = sp - psp;
        vep = ep - pep;
        vsp.unit(); vep.unit();
        double sdote = vsp.dot(vep);
        double l1 = sp.distance2(ep);
        double l2 = psp.distance2(pep);

        if (sdote < -COS_ANGLE_30_DEGREE && l1 < l2)
            mindis = sp.distance2(ep);
        else mindis = DBL_MAX;

        while (mindis > EPSILON_VAL_) {
            bool freshLoop = false;
            unsigned chossedIdx = UINT_MAX;
            bool isTail = false;
            Point3Dd csp;
            Point3Dd cep;
            for (unsigned ic = 0; ic < contoursHdl.size(); ic++)
            {
                if (deleted[ic] || ic == i) continue;

                csp = *(contoursHdl[ic].front());
                cep = *(contoursHdl[ic].back());

                if (ic != i && csp != cep) {
                    Vector3Dd vsp1 = *(contoursHdl[ic])[1] - csp;
                    vsp1.unit();
                    dis = ep.distance2(csp);
                    if (vsp1.dot(vep) < -COS_ANGLE_60_DEGREE)
                        dis += UINT_MAX/2;
                    Vector3Dd vep1 = cep - **(contoursHdl[ic].end() - 2);
                    vep1.unit();
                    dis1 = sp.distance2(cep);
                    if (vep1.dot(vsp) < -COS_ANGLE_60_DEGREE) {
                        dis1 += UINT_MAX/2;
                    }
                    if (dis < mindis + EPSILON_VAL_) {
                        mindis = dis;
                        chossedIdx = ic;
                        freshLoop = true;
                        isTail = false;
                    } else if (dis1 < mindis + EPSILON_VAL_) {
                        mindis = dis1;
                        chossedIdx = ic;
                        freshLoop = true;
                        isTail = true;
                    }
                }
            }

            if (chossedIdx != UINT_MAX) {
                if (!isTail) {
                    icontour->insert(icontour->end(), contoursHdl[chossedIdx].begin(), contoursHdl[chossedIdx].end());
                    ep = *icontour->back();
                }
                else {
                    icontour->insert(icontour->begin(), contoursHdl[chossedIdx].begin(), contoursHdl[chossedIdx].end());
                    sp = *icontour->front();
                }

                psp = (*(*icontour)[1]), pep = (*(*(icontour->end() - 2)));
                vsp = sp - psp;
                vep = ep - pep;
                vsp.unit(); vep.unit();
                double sdote = vsp.dot(vep);
                double l1 = sp.distance2(ep);
                double l2 = psp.distance2(pep);

                if (sdote < -COS_ANGLE_30_DEGREE && l1 < l2)
                    mindis = sp.distance2(ep);
                else mindis = DBL_MAX;

                if (sp == ep) break;
                deleted[chossedIdx] = true;
                freshLoop = true;
                chossedIdx = UINT_MAX;
            }

            if (freshLoop == false) break;
        }

        if (sp != ep) {
            (icontour)->push_back(new Point3Dd(sp));
        }

        finished[i] = true;
    }

    for (unsigned k = 0; k < contoursHdl.size(); k++)
    {
        if (!deleted[k]) contours.push_back(contoursHdl[k]);
    }
    contoursHdl.clear();

    return contours.size();
}

int Util::calQuadCollisionWithSegment(const std::vector<Point3Dd *> &quad,
    const Point3Dd &sp, const Point3Dd &ep, int* rect)
{
    int count = 0;

    Point3Dd v = (*quad[1] - *quad[0]) * (*quad[2] - *quad[1]);
    v.normalize();

    Point3Dd p0, p1;
    int ind = v.getIndexOfMaxAbsCoord();
    p0 = *quad[3];
    for (size_t i = 0; i < quad.size(); i++)
    {
        p1 = *quad[i];
        if (is2SegmentCollision(sp, ep, p0, p1, ind))
        {
            rect[count] = i;
            count++;
        }
        p0 = p1;
    }

    return count;
}


void Util::calcXYOffsetContourOfcurve(const std::vector<Point3Dd*> &curve,
    const double &offVal, std::vector<Point3Dd> &contour)
{
    unsigned n = curve.size();
    if (n > 1){
        contour.resize(2 * n);
        Vector2Dd n1, n2, v1, v2,dv;
        Point2Dd p1, p2;
        v1.x = curve[1]->x - curve[0]->x;
        v1.y = curve[1]->y - curve[0]->y;
        n1.x = -v1.y;
        n1.y = v1.x;
        n1.normalize();
        n2 = n1;
        contour[0].x = curve[0]->x + n1.x*offVal;
        contour[0].y = curve[0]->y + n1.y*offVal;
        contour[0].z = curve[0]->z;
        contour[2 * n - 1].x = curve[0]->x - n1.x*offVal;
        contour[2 * n - 1].y = curve[0]->y - n1.y*offVal;
        contour[2 * n - 1].z = curve[0]->z;
        p1.x = contour[0].x;
        p1.y = contour[0].y;
        double delta,t;
        for (unsigned i = 2; i < n; ++i){
            v2.x = curve[i]->x - curve[i-1]->x;
            v2.y = curve[i]->y - curve[i-1]->y;
            n2.x = -v2.y;
            n2.y = v2.x;
            n2.normalize();
            p2.x = curve[i-1]->x + n2.x*offVal;
            p2.y = curve[i-1]->y + n2.y*offVal;
            delta = v1.y*v2.x - v1.x*v2.y;
            if (fabs(delta) > EPSILON_VAL_MINI){
                t = (v2.x*(p2.y - p1.y) - v2.y*(p2.x - p1.x))/delta;
                contour[i - 1].x = p1.x + v1.x*t;
                contour[i - 1].y = p1.y + v1.y*t;
                contour[2*n - i].x = 2.0*curve[i - 1]->x - contour[i - 1].x;
                contour[2*n - i].y = 2.0*curve[i - 1]->y - contour[i - 1].y;
            }
            else{
                contour[i - 1].x = p2.x;
                contour[i - 1].y = p2.y;
                contour[2*n - i].x = 2.0*curve[i - 1]->x - contour[i - 1].x;
                contour[2*n - i].y = 2.0*curve[i - 1]->y - contour[i - 1].y;
            }
            contour[i - 1].z = contour[2*n - i].z = curve[i - 1]->z;
            v1 = v2;
            p1 = p2;
        }
        contour[n-1].x = curve[n-1]->x + n2.x*offVal;
        contour[n-1].y = curve[n-1]->y + n2.y*offVal;
        contour[n].x = curve[n - 1]->x - n2.x*offVal;
        contour[n].y = curve[n - 1]->y - n2.y*offVal;
        contour[n - 1].z = contour[n].z = curve[n - 1]->z;
    }
}

/************************************************************************/
/* Calculate point of intersection between two circles
@param r1 - radius of first circle
@param r2 - radius of second circle
@param p1 - center of first circle
@param p2 - center of second circle
@param intp - point of intersection (output)
@note intp have to satisfy the condition that vector<intp,p1> will be in CCW of vector<p1p2>
@author Nghi 03/10/2014
*/
/************************************************************************/
#if 0
bool Util::calcRightIntPointOfTwoCircles(double r1, double r2, double p1[2], double p2[2], double intp[2]){
    double nv[2] = { p1[1] - p2[1],p2[0] - p1[0] };
    double dv[2];

    double lengA2 = r1*r1;
    double lengB2 = r2*r2;
    double xx1 = p1[0] * p1[0];
    double xx2 = p2[0] * p2[0];
    double yy1 = p1[1] * p1[1];
    double yy2 = p2[1] * p2[1];
    double delta = -(p1[0] - p2[0]) *(p1[0] - p2[0]) * (lengA2 + 2 * r1*r2 + lengB2 - xx1 + 2 * p1[0] * p2[0] - xx2 - yy1 + 
        2 * p1[1] * p2[1] - yy2)*(lengA2 - 2 * r1*r2 + lengB2 - xx1 + 2 * p1[0] * p2[0] - xx2 - yy1 + 2 * p1[1] * p2[1] - yy2);
    if (delta < -EPSILON_VAL_){
        return false;
    }
    else if (delta < EPSILON_VAL_){
        delta = 0.0;
    }
    else{
        delta = sqrt(delta);
    }
    intp[0] = (lengA2 * xx1 - 2 * lengA2 * p1[0] * p2[0] + lengA2 * xx2 - lengB2 * xx1 + 2 * lengB2 * p1[0] * p2[0] - 
        lengB2 * xx2 - xx1*xx1 + 2 * p1[0] * xx1 * p2[0] - xx1 * yy1 + 2 * xx1 * p1[1] * p2[1] - xx1 * yy2 - 
        2 * p1[0] * p2[0] * xx2 + xx2*xx2 + xx2 * yy1 - 2 * xx2 * p1[1] * p2[1] + xx2 * yy2 + delta*p1[1] - delta*p2[1]) /
        (2 * (xx1 - 2 * p1[0]*p2[0] + xx2 + yy1 - 2 * p1[1]*p2[1] + yy2)*(p1[0] - p2[0]));
    intp[1] = (-lengA2 * p1[1] + lengA2 * p2[1] + lengB2 * p1[1] - lengB2 * p2[1] + xx1 * p1[1] + xx1 * p2[1] - 
        2 * p1[0] * p2[0] * p1[1] - 2 * p1[0] * p2[0] * p2[1] + xx2 * p1[1] + xx2 * p2[1] + p1[1] *yy1 - yy1 * p2[1] -
        p1[1] * yy2 + p2[1] *yy2 + delta) / (xx1 - 2 * p1[0]*p2[0] + xx2 + yy1 - 2 * p1[1]*p2[1] + yy2) / 2;

    dv[0] = intp[0] - p1[0];
    dv[1] = intp[1] - p1[1];
    if (dv[0] * nv[0] + dv[1] * nv[1] > 0){
        return true;
    }

    intp[0] = (-lengA2 * xx1 + 2 * lengA2 * p1[0] * p2[0] - lengA2 * xx2 + lengB2 * xx1 - 2 * lengB2 * p1[0] * p2[0] +
        lengB2 * xx2 + xx1*xx1 - 2 * p1[0] * xx1 * p2[0] + xx1 * yy1 - 2 * xx1 * p1[1] * p2[1] + xx1 * yy2 + 2 * p1[0] * p2[0] *xx2 -
        xx2*xx2 - xx2 * yy1 + 2 * xx2 * p1[1] * p2[1] - xx2 * yy2 + delta*p1[1] - delta*p2[1]) /
        ((xx1 - 2 * p1[0]*p2[0] + xx2 + yy1 - 2 * p1[1]*p2[1] + yy2)*(p1[0]-p2[0])*2);
    intp[1] = (lengA2 * p1[1] - lengA2 * p2[1] - lengB2 * p1[1] + lengB2 * p2[1] - xx1 * p1[1] - xx1 * p2[1] +
        2 * p1[0]*p2[0]*p1[1] + 2 * p1[0]*p2[0]*p2[1] - xx2 * p1[1] - xx2 * p2[1] - p1[1] *yy1 + yy1 * p2[1] + p1[1]*yy2 - p2[1] *yy2 + delta) /
        (2.0*(xx1 - 2 * p1[0]*p2[0] + xx2 + yy1 - 2 * p1[1]*p2[1] + yy2));
    return true;
}

#else
bool Util::calcRightIntPointOfTwoCircles(double r1, double r2, double p1[2], double p2[2], double intp[2]){

    if (fabs(p1[0] - p2[0]) > EPSILON_VAL_){
        double RR = (r2*r2 - r1*r1 + p1[0] * p1[0] - p2[0] * p2[0] + p1[1] * p1[1] - p2[1] * p2[1])*0.5 / (p1[0] - p2[0]);
        double S = (p1[1] - p2[1]) / (p1[0] - p2[0]);
        double A = S*S + 1;
        double B = S*(RR - p1[0]) + p1[1];
        double C = (RR - p1[0])*(RR - p1[0]) + p1[1] * p1[1] - r1*r1;
        double delta = B*B - A*C;
        if (fabs(A) < EPSILON_VAL_){
            if (fabs(B) > EPSILON_VAL_){
                intp[1] = -C*0.5 / B;
                intp[0] = RR - intp[1] * S;
                return true;
            }
            else
                return false;
        }
        if (delta < -EPSILON_VAL_){
            return false;
        }
        else if (delta < EPSILON_VAL_){
            delta = 0.0;
            intp[1] = -B / A;
            intp[0] = RR - intp[1] * S;
            return true;
        }
        else{
            double nv[2] = { p1[1] - p2[1], p2[0] - p1[0] };
            delta = sqrt(delta);
            intp[1] = (-B + delta) / A;
            intp[0] = RR - intp[1] * S;
            double dv[2] = { intp[0] - p1[0], intp[1] - p1[1] };
            return true;
            if (dv[0] * nv[0] + dv[1] * nv[1] > 0){
                return true;
            }

            intp[1] = (-B - delta) / A;
            intp[0] = RR - intp[1] * S;
            dv[0] = intp[0] - p1[0];
            dv[1] = intp[1] - p1[1];
            assert(dv[0] * nv[0] + dv[1] * nv[1] > 0);
            return true;
        }
        return false;
    }
	return false;
}
#endif

#if 0
void Util::convertPointToCalib(double platfX, double paltfY, double expectX, double expectY, double AB, double BC,
    double CD, double DA, Point2Dd &p)
{
    double cenX = platfX*0.5;
    double cenY = paltfY*0.5;
    double scaleX, scaleY;
    if (fabs(DA - BC) < fabs(AB - CD)){
        double dx = (AB - CD)*0.5;
        double dy = sqrt((DA + BC)*(DA + BC)*0.25 - dx*dx);
        scaleY = (expectY - dy)/expectY;
        double ymax = cenY + dy*0.5;
        double delta = dx*(ymax - p.y) / dy;
        double bound_px = CD + 2.0*delta;
        scaleX = (expectX - bound_px) / expectX;
        
    }
    else{
        double dy = ( DA - BC)*0.5;
        double dx = sqrt((AB + CD)*(AB + CD)*0.25 - dy*dy);
        scaleX = (expectX - dx) / expectX;
        double xmax = cenX + dx*0.5;
        double delta = (xmax - p.x)*dy / dx;
        double bound_py = BC + 2.0*delta;
        scaleY = (expectY - bound_py) / expectY;
    }

    p.x -= scaleX*(cenX - p.x);
    p.y -= scaleY*(cenY - p.y);
    
}

#else
void Util::convertPointToCalib(double platfX, double paltfY, double expectX, double expectY, double AB, double BC,
    double CD, double DA, Point2Dd &p)
{
    double cenX = platfX*0.5;
    double cenY = paltfY*0.5;
    double scaleX, scaleY;

    double dx = (AB - CD)*0.5;
    double dy = sqrt((DA + BC)*(DA + BC)*0.25 - dx*dx);

    double ymax = cenY + dy*0.5;
    double delta = dx*(ymax - p.y) / dy;
    double bound_px = CD + 2.0*delta;
    scaleX = (expectX - bound_px) / expectX;


    dy = (DA - BC)*0.5;
    dx = sqrt((AB + CD)*(AB + CD)*0.25 - dy*dy);

    double xmax = cenX + dx*0.5;
    delta = (xmax - p.x)*dy / dx;
    double bound_py = BC + 2.0*delta;
    scaleY = (expectY - bound_py) / expectY;


    p.x -= scaleX*(cenX - p.x);
    p.y -= scaleY*(cenY - p.y);

}
#endif

void Util::convertPointToCalib(double platfX, double paltfY, double expectX, double expectY, const Point2Dd &A, const Point2Dd &B,
const Point2Dd &C, const Point2Dd &D, Point2Dd &p){

}

Vector3Dd Util::getVectorOnPlane(const Point3Dd &p, const Vector3Dd &norm)
{
    int id = norm.getIndexOfMaxAbsCoord();
    Vector3Dd vv;
    vv[id] = norm[(id + 1) % 3];
    vv[(id + 1) % 3] = -norm[id];
    vv[(id + 2) % 3] = 0;

    return vv;
}

Point3Dd Util::getAPointOnPlane(const Point3Dd &p, const Vector3Dd &norm, double d)
{
    Point3Dd p2;
    int id = norm.getIndexOfMaxAbsCoord();
    Vector3Dd vv;
    vv[id] = norm[(id + 1) % 3];
    vv[(id + 1) % 3] = -norm[id];
    vv[(id + 2)%3] = 0;
    p2 = vv + p;
    p2 = p + (p2 - p).normalize() * d;

    return p2;
}

#ifdef K_DETECT_MEM_LEAK

typedef struct {
    DWORD	address;
    DWORD	size;
    //char	file[64];
    //DWORD	line;
    std::string symbols;
} ALLOC_INFO;

typedef std::list<ALLOC_INFO *> AllocList;

AllocList *allocList;

void addMemoryTrack(DWORD addr,  DWORD asize, std::vector<std::string> const &symbols/*,  const char *fname, DWORD lnum*/)
{
    if (!asize)
        return;

    ALLOC_INFO *info;

    if(!allocList) {
        allocList = new(AllocList);
    }

    info = new(ALLOC_INFO);
    info->address = addr;
    //strncpy(info->file, fname, 63);
    //info->line = lnum;
    info->size = asize;

    for (size_t i = 0; i < symbols.size(); ++i)
    {
        info->symbols += '\t';
        info->symbols += symbols[i];
        info->symbols += '\n';
    }

    allocList->insert(allocList->begin(), info);
};

void removeMemoryTrack(DWORD addr)
{
    AllocList::iterator i;

    if(!allocList)
        return;
    for(i = allocList->begin(); i != allocList->end(); i++) {
        if((*i)->address == addr) {
            allocList->remove((*i));
            break;
        }
    }
};

/**************************************************************************************************
 * Dumps a list of unfreed memory items with callstack traces.
 *
 * @param   whichFunction   The which function.
 **************************************************************************************************/

void dumpUnfreedMemory(const char *whichFunction)
{
    using namespace std;

    char buf[1024];

    sprintf(buf, "==== Checking memory leaks at: [%s]====\n", whichFunction);

    OutputDebugStringA(buf);
    Util::printLogMessage(buf, false);

    DWORD totalSize = 0;

    if(!allocList) {
        sprintf(buf, "Good job: No memory leaks.\n");

        OutputDebugStringA(buf);
        Util::printLogMessage(buf, false);
        return;
    }

    sprintf(buf, "Number of malloc'ed items: %u\trealloc'ed: %u\tfreed %u\tUnfreed: %u\n",
            Util_k_malloc_count, Util_k_realloc_count, Util_k_free_count, Util_k_malloc_count+Util_k_realloc_count-Util_k_free_count);

    OutputDebugStringA(buf);
    Util::printLogMessage(buf, false);

    sprintf(buf, "Number of unfreed items: %d\n", allocList->size());

    OutputDebugStringA(buf);
    Util::printLogMessage(buf, false);

    typedef std::map<std::string, std::pair<unsigned, unsigned>> MapSymbol2Size;
    MapSymbol2Size mapSymbol2Size;

    for(auto i = allocList->begin(); i != allocList->end(); i++) {
        ALLOC_INFO const &info = **i;
        /*sprintf(buf, "%d bytes unfreed\tADDRESS %d\n",
        info.size, info.address);
        OutputDebugStringA(buf);
        Util::printLogMessage(buf, false);

        for (auto j = 0; j < info.symbols.size(); ++j) {
        sprintf(buf, "\t%s\n",info.symbols[j].c_str());
        OutputDebugStringA(buf);
        Util::printLogMessage(buf, false);
        }*/

        auto const &symbols = info.symbols;

        if (!mapSymbol2Size.count(symbols)) {
            mapSymbol2Size[symbols].first = info.size;
            mapSymbol2Size[symbols].second = 1;
        } else {
            mapSymbol2Size[symbols].first += info.size;
            mapSymbol2Size[symbols].second++;
        }

        /*OutputDebugStringA(buf);
        Util::printLogMessage(buf, false);*/

        totalSize += info.size;
    }

    sprintf(buf, "Total Unfreed: %d bytes\n", totalSize);
    OutputDebugStringA(buf);
    Util::printLogMessage(buf, false);

    std::vector<pair<unsigned, pair<unsigned, string>>> sortedList(mapSymbol2Size.size());

    int j = 0;
    for (auto i = mapSymbol2Size.begin(); i!= mapSymbol2Size.end(); ++i, ++j) {
        sortedList[j].first = i->second.first;
        sortedList[j].second.first = i->second.second;
        sortedList[j].second.second = i->first;
    }

    typedef decltype(sortedList.front()) ItemType;

    std::sort(std::begin(sortedList),
              std::end(sortedList), [](ItemType const & l, ItemType const &r)->bool {return l.first < r.first;});

    for(auto i = sortedList.begin(); i != sortedList.end(); ++i) {
        sprintf(buf, "%d bytes unfreed (#%d)\n",i->first,i->second.first);

        OutputDebugStringA(buf);
        Util::printLogMessage(buf, false);

        auto symbols = i->second.second;
        OutputDebugStringA(symbols.c_str());
        Util::printLogMessage(symbols, false);
        //for (auto j = 0; j < symbols.size(); ++j) {
        //    sprintf(buf, "\t%s\n", symbols[j].c_str());
        //    OutputDebugStringA(buf);
        //    Util::printLogMessage(buf, false);
        //}
    }
};

#endif K_DETECT_MEM_LEAK
}// namespace

/***************************************************************************************************
* @fn bool Util::isPointInAABB(const Point3Dd& p, const Point3Dd& mincoord,
* const Point3Dd& maxcoord)
*
* @brief Query if 'p' is point in aabb.
*
* @author PhamKhoang
* @date 12/15/2014
*
* @param p        The const Point3Dd; to process.
* @param mincoord The mincoord.
* @param maxcoord The maxcoord.
*
* @return true if point in a bb, false if not.
***************************************************************************************************/

bool BaseType::Util::isPointInAABB(const Point3Dd& p, const Point3Dd& mincoord, const Point3Dd& maxcoord)
{
	if (p[0] >= mincoord[0] && p[1] >= mincoord[1] && p[2] >= mincoord[2]
		&& p[0] <= maxcoord[0] && p[1] <= maxcoord[1] && p[2] <= maxcoord[2])
		return true;

	return false;
}


/*
*KhanhHH
*clone from Blender v2.76
*/
int BaseType::Util::intersectLineSphere(const Point3Dd &l1, const Point3Dd &l2, const Point3Dd &sp, const float r, Point3Dd &r_p1, Point3Dd &r_p2)
{
	/* l1:         coordinates (point of line)
	* l2:         coordinates (point of line)
	* sp, r:      coordinates and radius (sphere)
	* r_p1, r_p2: return intersection coordinates
	*/


	/* adapted for use in blender by Campbell Barton - 2011
	*
	* atelier iebele abel - 2001
	* atelier@iebele.nl
	* http://www.iebele.nl
	*
	* sphere_line_intersection function adapted from:
	* http://astronomy.swin.edu.au/pbourke/geometry/sphereline
	* Paul Bourke pbourke@swin.edu.au
	*/

	const Point3Dd ldir = l2 - l1;
	//const float ldir[3] = {
	//	l2[0] - l1[0],
	//	l2[1] - l1[1],
	//	l2[2] - l1[2]
	//};

	const double a = ldir.squareModule();// len_squared_v3(ldir);

	const double b = 2.0f *
		(ldir[0] * (l1[0] - sp[0]) +
		ldir[1] * (l1[1] - sp[1]) +
		ldir[2] * (l1[2] - sp[2]));

	const double c =
		sp.squareModule() +
		l1.squareModule() -
		(2.0 * sp.dot(l1)) -
		(r * r);
	//const float c =
	//	len_squared_v3(sp) +
	//	len_squared_v3(l1) -
	//	(2.0f * dot_v3v3(sp, l1)) -
	//	(r * r);

	const double i = b * b - 4.0 * a * c;

	double mu;

	if (i < 0.0) {
		/* no intersections */
		return 0;
	}
	else if (i == 0.0) {
		/* one intersection */
		mu = -b / (2.0f * a);
		r_p1 = l1 + ldir *mu;
		//madd_v3_v3v3fl(r_p1, l1, ldir, mu);
		return 1;
	}
	else if (i > 0.0) {
		const double i_sqrt = sqrt(i);  /* avoid calc twice */

		/* first intersection */
		mu = (-b + i_sqrt) / (2.0 * a);
		r_p1 = l1 + ldir *mu;
		//madd_v3_v3v3fl(r_p1, l1, ldir, mu);

		/* second intersection */
		mu = (-b - i_sqrt) / (2.0 * a);
		r_p2 = l1 + ldir *mu;
		//madd_v3_v3v3fl(r_p2, l1, ldir, mu);
		return 2;
	}
	else {
		/* math domain error - nan */
		return -1;
	}
}

int BaseType::Util::intersectRaySphere(const Point3Dd& p, const Point3Dd& d, const Point3Dd& sCenter, float radius, Point3Dd& q)
{
	Point3Dd L = (sCenter - p);
	float sqrRadius = radius * radius;
	float tca = (float)L.dot(d);
	if (tca > 0)
	{
		float d2 = (float)L.dot(L) - tca * tca;
		if (d2 <= sqrRadius)
		{
			float thc = sqrt(sqrRadius - d2);
			float t0 = tca - thc;
			float t1 = tca + thc;

			//assert(t0 >= 0.0f && t1 >= 0.0f);
			t0 = std::min<float>(t0, t1);
			q = p + t0 * d;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

int BaseType::Util::intersectRayConeFrustum(const Point3Dd& p, const Point3Dd& d, const Point3Dd& sCenter, float radius, const Point3Dd& sCenter1, float radius1, Point3Dd& q)
{
	double maxRadius = max2(radius1, radius);
	double minRadius = min2(radius1, radius);
	Point3Dd maxCenter = sCenter;
	Point3Dd minCenter = sCenter1;

	if (maxRadius == radius1)
	{
		maxCenter = sCenter1;
		minCenter = sCenter;
	}

	Vector3Dd axis = maxCenter - minCenter;
	double length = axis.length();
	axis.unit();
	Point3Dd apex = maxCenter + maxRadius*(minCenter - maxCenter) / (maxRadius - minRadius);
	double cosSqr = (maxRadius - minRadius) / length; // first cal tag of angle
	cosSqr = 1 / (1 + cosSqr*cosSqr);

	Vector3Dd PmV = p - apex;
	double DdU = axis.dot(d);//(cone.ray.direction, lineDirection);
	double DdPmV = axis.dot(PmV);
	double UdPmV = d.dot(PmV);
	double PmVdPmV = PmV.dot(PmV);
	double c2 = DdU*DdU - cosSqr;
	double c1 = DdU*DdPmV - cosSqr*UdPmV;
	double c0 = DdPmV*DdPmV - cosSqr*PmVdPmV;
	double t, t1[2];
	size_t type = 0;
	size_t numParameters = 0;
	if (!FLOAT_EQUAL(c2, 0))
	{
		// c2 != 0
		double discr = c1 * c1 - c0 * c2;
		if (discr < 0)
		{
			// The quadratic has no real-valued roots.  The line does not
			// intersect the double-sided cone.
			return 0;
		}
		else if (discr > 0)
		{
			// The quadratic has two distinct real-valued roots.  However, one
			// or both of them might intersect the negative cone.  We are
			// interested only in those intersections with the positive cone.
			double root = sqrt(discr);
			double invC2 = ((double)1) / c2;
			int numParameters = 0;

			t = (-c1 - root) * invC2;
			if (DdU * t + DdPmV >= (double)0)
			{
				t1[numParameters++] = t;
			}

			t = (-c1 + root) * invC2;
			if (DdU * t + DdPmV >= (double)0)
			{
				t1[numParameters++] = t;
			}

			if (numParameters == 2)
			{
				// The line intersects the positive cone in two distinct
				// points.
				type = 2;
				if (t1[0] > t1[1])
				{
					std::swap(t1[0], t1[1]);
				}
			}
			else if (numParameters == 1)
			{
				// The line intersects the positive cone in a single point and
				// the negative cone in a single point.  We report only the
				// intersection with the positive cone.
				if (DdU > (double)0)
				{
					type = 3;
					t1[1] = std::numeric_limits<double>::max();
				}
				else
				{
					type = 4;
					t1[1] = t1[0];
					t1[0] = -std::numeric_limits<double>::max();

				}
			}
			else
			{
				// The line intersects the negative cone in two distinct
				// points, but we are interested only in the intersections
				// with the positive cone.
				return 0;
			}
		}
		else
		{
			// One repeated real root; the line is tangent to the double-sided
			// cone at a single point.  Report only the point if it is on the
			// positive cone.
			t = -c1 / c2;
			if (DdU * t + DdPmV >= (double)0)
			{
				type = 1;
				t1[0] = t;
				t1[1] = t;
			}
			else
			{
				return 0;
			}
		}
	}
	q = p + t1[0] * d;
	double d1 = (q - minCenter).dot(axis);
	d1 = (minCenter + axis * d1).distance(q);
	if (d1 > maxRadius || d1 < minRadius)
		return 0;
	return type;
}

int BaseType::Util::intersectLineLineEpsilon(const Point3Dd& v1, const Point3Dd& v2, const Point3Dd& v3, const Point3Dd& v4, Point3Dd& i1, Point3Dd& i2, double epsilon)
{
	Point3Dd a, b, c, ab, cb, dir1, dir2;
	float d, div;

	c = v3 - v1;// sub_v3_v3v3(c, v3, v1);
	a = v2 - v1;// sub_v3_v3v3(a, v2, v1);
	b = v4 - v3;// sub_v3_v3v3(b, v4, v3);

	dir1 = a; dir1.normalize(); //normalize_v3_v3(dir1, a);
	dir2 = b; dir2.normalize(); //normalize_v3_v3(dir2, b);
	d = (float)dir1.dot(dir2);
	if (d == 1.0f || d == -1.0f) {
		/* colinear */
		return 0;
	}

	ab = a.cross(b); // cross_v3_v3v3(ab, a, b);
	d = (float)c.dot(ab);   //d = dot_v3v3(c, ab);
	div = (float)ab.dot(ab); //div = dot_v3v3(ab, ab);

	/* test zero length line */
	if (div == 0.0f) {
		return 0;
	}
	/* test if the two lines are coplanar */
	else if (fabsf(d) <= epsilon) {
		cb = c.cross(b); //cross_v3_v3v3(cb, c, b);

		a *= cb.dot(ab) / div;//mul_v3_fl(a, dot_v3v3(cb, ab) / div);
		i1 = v1 + a;//add_v3_v3v3(i1, v1, a);
		i2 = i1; // copy_v3_v3(i2, i1);

		return 1; /* one intersection only */
	}
	/* if not */
	else {
		Point3Dd n, t;
		Point3Dd v3t, v4t;
		t = v1 - v3;//sub_v3_v3v3(t, v1, v3);

		/* offset between both plane where the lines lies */
		n = a.cross(b);//cross_v3_v3v3(n, a, b);
		t = projectVec3Vec3(t, n); //(t, t, n);

		/* for the first line, offset the second line until it is coplanar */
		v3t = v3 + t;//add_v3_v3v3(v3t, v3, t);
		v4t = v4 + t;//add_v3_v3v3(v4t, v4, t);

		c = v3t - v1;//sub_v3_v3v3(c, v3t, v1);
		a = v2 - v1;//sub_v3_v3v3(a, v2, v1);
		b = v4t - v3t;//sub_v3_v3v3(b, v4t, v3t);

		ab = a.cross(b);//cross_v3_v3v3(ab, a, b);
		cb = c.cross(b);// cross_v3_v3v3(cb, c, b);

		a *= cb.dot(ab) / ab.dot(ab);//mul_v3_fl(a, dot_v3v3(cb, ab) / dot_v3v3(ab, ab));
		i1 = v1 + a;//add_v3_v3v3(i1, v1, a);

		/* for the second line, just substract the offset from the first intersection point */
		i2 = i1 - t;// sub_v3_v3v3(i2, i1, t);

		return 2; /* two nearest points */
	}
}

Point3Dd BaseType::Util::projectVec3Vec3(const Point3Dd& v1, const Point3Dd& v2)
{
	Point3Dd c;

	const float mul = (float)(v1.dot(v2) / v2.dot(v2));

	c[0] = mul * v2[0];
	c[1] = mul * v2[1];
	c[2] = mul * v2[2];

	return c;
}

/*planeNormal is normalized*/
void BaseType::Util::projectPointOnPlane(const Point3Dd& p, const Point3Dd& planePoint, const Point3Dd& planeNormal, Point3Dd& proj)
{
	Point3Dd dir = p - planePoint;
	double lenAlongNormal = planeNormal.dot(dir);
	proj = p - lenAlongNormal * planeNormal;
}

bool BaseType::Util::intersectRayTriangle(const Point3Dd& org, const Point3Dd& dir, const Point3Dd v0, const Point3Dd v1, const Point3Dd v2, double& dist)
{
	//Find vectors for two edges sharing v0
	Point3Dd e1 = v1 - v0;
	Point3Dd e2 = v2 - v0;

	//Begin calculating determinant - also used to calculate u parameter
	Point3Dd p = dir.cross(e2);

	//if determinant is near zero, ray lies in plane of triangle
	double det = e1.dot(p);
	//NOT CULLING
	if (det > -EPSILON_VAL_ && det < EPSILON_VAL_) return 0;
	double inv_det = 1.0 / det;

	//calculate distance from v0 to ray origin
	Point3Dd s = org - v0;

	//Calculate u parameter and test bound
	double u = inv_det  * s.dot(p);
	//The intersection lies outside of the triangle
	if ((u < -EPSILON_VAL_) || (u > 1.0 + EPSILON_VAL_)) return 0;

	//Prepare to test v parameter
	Point3Dd q = s.cross(e1);

	//Calculate V parameter and test bound
	double v = inv_det  * dir.dot(q);
	//The intersection lies outside of the triangle
	if ((v < -EPSILON_VAL_) || ((u + v) > 1.0 + EPSILON_VAL_)) return 0;

	dist = inv_det  * e2.dot(q);
	if ((dist > EPSILON_VAL_)) return 1;

	return 0;
}

/**
* \return The number of point of interests
* 0 - lines are colinear
* 1 - lines are coplanar, i1 is set to intersection
* 2 - i1 and i2 are the nearest points on line 1 (v1, v2) and line 2 (v3, v4) respectively
*/
/*KhanhHH copied from Blender source code V 2.75*/
int BaseType::Util::calcIntLineLineEpsilon(const Point3Dd& v1, const Point3Dd& v2, const Point3Dd& v3, const Point3Dd& v4, Point3Dd& i1, Point3Dd& i2, const double epsilon)
{
	Point3Dd a, b, c, ab, cb, dir1, dir2;
	double d, div;

	c = v3 - v1; //sub_v3_v3v3(c, v3, v1);
	a = v2 - v1; //sub_v3_v3v3(a, v2, v1);
	b = v4 - v3; //sub_v3_v3v3(b, v4, v3);

	dir1 = a; dir1.normalize();//normalize_v3_v3(dir1, a);
	dir2 = b; dir2.normalize();//normalize_v3_v3(dir2, b);
	d = dir1.dot(dir2); //d = dot_v3v3(dir1, dir2);
	if (d == 1.0 || d == -1.0) {
		/* colinear */
		return 0;
	}

	ab = a.cross(b);//cross_v3_v3v3(ab, a, b);
	d = c.dot(ab);//d = dot_v3v3(c, ab);
	div = ab.dot(ab);//div = dot_v3v3(ab, ab);

	/* test zero length line */
	if ((div == 0.0f)) {
		return 0;
	}

	/* test if the two lines are coplanar */
	else if ((abs(d) <= epsilon)) {
		cb = c.cross(b);//cross_v3_v3v3(cb, c, b);

		a *= cb.dot(ab) / div;//mul_v3_fl(a, dot_v3v3(cb, ab) / div);
		i1 = v1 + a; //add_v3_v3v3(i1, v1, a);
		i2 = i1; //copy_v3_v3(i2, i1);

		return 1; /* one intersection only */
	}

	/* if not */
	else {
		Point3Dd n, t;
		Point3Dd v3t, v4t;
		t = v1 - v3;//sub_v3_v3v3(t, v1, v3);

		/* offset between both plane where the lines lies */
		n = a.cross(b);//cross_v3_v3v3(n, a, b);
		project_v3_v3v3(t, t, n);

		/* for the first line, offset the second line until it is coplanar */
		v3t = v3 + t;//add_v3_v3v3(v3t, v3, t);
		v4t = v4 + t;//add_v3_v3v3(v4t, v4, t);

		c = v3t - v1; //sub_v3_v3v3(c, v3t, v1);
		a = v2 - v1;  //sub_v3_v3v3(a, v2, v1);
		b = v4t - v3t;//sub_v3_v3v3(b, v4t, v3t);

		ab = a.cross(b); //cross_v3_v3v3(ab, a, b);
		cb = c.cross(b); //cross_v3_v3v3(cb, c, b);

		a *= cb.dot(ab) / ab.dot(ab);//mul_v3_fl(a, dot_v3v3(cb, ab) / dot_v3v3(ab, ab));
		i1 = v1 + a;//add_v3_v3v3(i1, v1, a);

		/* for the second line, just substract the offset from the first intersection point */
		i2 = i1 - t;//sub_v3_v3v3(i2, i1, t);

		return 2; /* two nearest points */
	}
}

void BaseType::Util::project_v3_v3v3(Point3Dd& c, const Point3Dd& v1, const Point3Dd& v2)
{
	const double mul = v1.dot(v2) / v2.dot(v2);
	c = mul * v2;
}

double BaseType::Util::calcClosestToLine3D(Point3Dd& cp, const Point3Dd& p, const Point3Dd& l1, const Point3Dd& l2)
{
	Point3Dd h, u;
	double lambda;
	u = l2 - l1;
	h = p - l1;
	lambda = u.dot(h) / u.dot(u);
	cp = l1 + lambda *u;
	return lambda;
}

/* point closest to v1 on line v2-v3 in 3D */
void BaseType::Util::calcClosestToLineSegment3D(Point3Dd& r_close, const Point3Dd& v1, const Point3Dd& v2, const Point3Dd& v3)
{
	double lambda;
	Point3Dd cp;

	lambda = calcClosestToLine3D(cp, v1, v2, v3);

	if (lambda <= 0.0f)
		r_close = v2;
	else if (lambda >= 1.0f)
		r_close = v3;
	else
		r_close = cp;
}

double BaseType::Util::calcSquareDistToLineSegment3D(const Point3Dd& p, const Point3Dd& l1, const Point3Dd& l2)
{
	Point3Dd closest;

	calcClosestToLineSegment3D(closest, p, l1, l2);

	return closest.squareDistanceToPoint(p);
}

