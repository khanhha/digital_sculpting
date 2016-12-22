#ifndef CRIARTLIB_UTIL_H
#define CRIARTLIB_UTIL_H

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <list>
//#include "config.h"

#include "Point3Dd.h"
#include "PointnD.h"
#include "segment3Dd.h"

#ifdef _DEBUG
extern Point3Dd temp_dump_point1;
extern Point3Dd temp_dump_point2;
extern Point3Dd temp_dump_point3;
extern Point3Dd temp_dump_point4;
#endif
static const unsigned int RESERVED_BUFFER_SIZE = 20*1024*1024; // 10 MB reserved memory (Ex: used for rendering, ...)

static const double PI = 3.1415926535897932384626433832795;
static const double DIV_PI_180 = PI/180;
static const double DIV_180_PI = 180/PI;

#ifdef K_DETECT_MEM_LEAK
// Overwrite new and delete operator to keep track of memory allocations
void * __cdecl operator new(unsigned int size, const char *file, int line);
void __cdecl operator delete(void *p);

namespace BaseType {
void dumpUnfreedMemory(const char *whichFunction);
void addMemoryTrack(unsigned long addr, unsigned long asize, std::vector<std::string> const &symbols);
void removeMemoryTrack(unsigned long addr);
}

#endif K_DETECT_MEM_LEAK

namespace BaseType {
class Util {
    static std::ofstream Util_fout;
public:
    static std::string getAppPath();
    static std::wstring getAppPathW();
    static std::wstring getProgramDataPath();
    static std::wstring getLogFilePath(const std::wstring& companyName, const std::wstring& appName);

    static void initLogMessage(const std::wstring& companyName, const std::wstring& appName);
    static void printLogMessage(std::string &msg, bool const withEndl = true);
    static void printLogMessage(char const *msg, bool const withEndl = true);
    static void printLogMessage(double value, char const *msg = nullptr);
    static void printLogMessage(int value, char const *msg = nullptr);
    static void printLogMessage(Point3Dd value, char const *msg = nullptr);
    static void *k_malloc(unsigned size);
    static void *k_realloc(void *mem_, unsigned size);
    static void k_free(void *mem_);
    static int sum_2d(const double a[2], const double b[2], double c[2]);
    static int minus_2d(const double a[2], const double b[2], double c[2]);
    static double multiply_2d(const double a[2], const double b[2]);
    static double calcSquareDistance_2d(const double a[2], const double b[2]);
    static bool is2PointCoincidence(float *p1, float *p2);
    static int calcIntSegmentPlane(const Point3Dd &sp, const Point3Dd &ep,
                                   const Point3Dd &pv, const Point3Dd &nv, Point3Dd &intp);
    static int calcIntRayPlane(const Point3Dd &sp, const Point3Dd &dir,
                               const Point3Dd &pv, const Point3Dd &nv, Point3Dd &intp);
    static int calcIntLinePlane(const Point3Dd &sp, const Point3Dd &dir,
                                const Point3Dd &pv, const Point3Dd &nv, Point3Dd &intp);
    static int calcIntQuadQuadCoplanar(const std::vector<Point3Dd *> &rec1,
                                       const std::vector<Point3Dd *> &rec2,
                                       std::vector<Point3Dd> &intp);
    static bool isQuadCollisionWithSegment(const std::vector<Point3Dd *> &quad,
                                           const Point3Dd &sp, const Point3Dd &ep,
                                           bool skipBound = false);
    static bool isBoxCollisionWithLine(const Point3Dd &pv, const Point3Dd &nv,
                                       const Point3Dd &ov, const double &sz);
    static bool isBoxCollisionWithRay(const Point3Dd &pv, const Point3Dd &nv,
                                      const Point3Dd &ov, const double &sz);
    static bool isBoxCollisionWithSegment(const double sv[3], const double ev[3],
                                          const double ov[3], const double &sz);
    static bool isBoxCollisionWithPlane(const Point3Dd &p, const Vector3Dd &n, Point3Dd &ov, const double &sz);
    static bool isBoxInsideOtherBox(const double mincoord1[3], const double maxcoord1[3],
                                    const double mincoord2[3], const double maxcoord2[3]);
    static void writeBytesFromInt(::std::ofstream &file, int valueIn);
    static void writeBytesFromFloat(std::ofstream &file, float valueIn);
    static void writeBytesFromBool(::std::ofstream &file, bool valueIn);

    static bool isTwo2DAABBCollision(const double *minCoord1,
                                     const double *maxCoord1, const double *minCoord2, const double *maxCoord2);
    static bool isTwoAABBCollision(const double *minCoord1,
                                   const double *maxCoord1, const double *minCoord2, const double *maxCoord2);
    static void calcRotMatrix(double angle, double *p,
                              double *dir, double *m);
    static double calc2DSquareMinDistance2Point(double *p1, double *p2);
    static double calc2DSquareMinDistancePointSegment(const double *point,
            const double *sp, const double *ep);
    static bool isPointInConvexPolygon3D(const Point3Dd &p,
                                         const std::vector<Point3Dd *> &plg, const Vector3Dd &plgNorm,
                                         bool skipBound = false);
    static bool isPointInsideRectang(const double *p, const double *op,
                                     const double *sz);
    static bool isPointInsideTriangle2D(const double *p, const double *p1, const double *p2,
                                        const double *p3,const double &normdir);
    static bool calcIntLineWithSegment(const double p[2],
                                       const double dv[2], const double sp[2], const double ep[2], double intp[2]);
    static bool calcIntRayWithSegment(const double p[2], const double dv[2],
                                     const double sp[2], const double ep[2],
                                     double intp[2]);
    static bool calcIntRayWithRay(const double p1[2], const double dv1[2],
                                 const double p2[2], const double dv2[2],
                                 double intp[2]);
    static bool calcInt2Segment2D(const double sp1[2],
                                  const double ep1[2], const double sp2[2], const double ep2[2], double intp[2]);
    static bool calcInt2Segment2DIgnore1stTip(const double sp1[2],
                                    const double ep1[2], const double sp2[2], const double ep2[2], double intp[2]);
    static bool calcIntPointOf2PlanarSegment(const Point3Dd &sp0, const Point3Dd &ep0,
            const Point3Dd &sp1, const Point3Dd &ep1, const int i, Point3Dd &intP);
    static bool calcIntPointOf2PlanarLine(const Point3Dd &p0, const Vector3Dd &v0, const Point3Dd &p1, const Vector3Dd &v1,
                                          const int i, Point3Dd &intp);
    static bool calcIntLineSegmentPlanar(const Point3Dd &p, const Vector3Dd &dir, const Point3Dd &sp, const Point3Dd &ep,
                                            const int i, Point3Dd &intp);
    static int calcIntPointOf2PlanarSegmentIgnore1Tip(const Point3Dd &sp0, const Point3Dd &ep0,
                                             const Point3Dd &sp1, const Point3Dd &ep1, const int i, Point3Dd &intP);
    static bool calcIntPointOf2PlanarSegmentIgnoreAllTips(const Point3Dd &sp0, const Point3Dd &ep0,
                                                         const Point3Dd &sp1, const Point3Dd &ep1, const int i, Point3Dd &intP);
    static bool calc2DIntPointOf2Line(const double *p1, const double *dv1,
                                      const double *p2, const double *dv2, double &t);
	static int  calcIntLineLineEpsilon(const Point3Dd& v1, const Point3Dd& v2,
										const Point3Dd& v3, const Point3Dd& v4, Point3Dd& i1, Point3Dd& i2,
										const double epsilon);
	static double calcClosestToLine3D(Point3Dd& cp, const Point3Dd& p, const Point3Dd& l1, const Point3Dd& l2);
	static void calcClosestToLineSegment3D(Point3Dd& r_close, const Point3Dd& v1, const Point3Dd& v2, const Point3Dd& v3);
	static double calcSquareDistToLineSegment3D(const Point3Dd& p, const Point3Dd& l1, const Point3Dd& l2);

    static bool is2SegmentCollision(const Point3Dd &sp0, const Point3Dd &ep0,
                                    const Point3Dd &sp1, const Point3Dd &ep1, const int i,
                                    bool skipTip = false);
    static bool isSegmentCollisionPlane(const Point3Dd &sp, const Point3Dd &ep, const Point3Dd &pv, const Vector3Dd &nv);
    static bool is2SegmentCollision2D(const double *sp0, const double *ep0,
                                      const double *sp1, const double *ep1);
    static int isPointInsideSegment(const Point3Dd &sp, const Point3Dd &ep,
                                    const Point3Dd &p, double *t = 0);
    static int isPointInsideSegment_HighAccur(const Point3Dd &sp, const Point3Dd &ep,
            const Point3Dd &p);
    static int isPointInsideSegmentStrong(const Point3Dd &sp, const Point3Dd &ep,
                                          const Point3Dd &p);
    static bool isPointInsidePolygon2D(const std::vector<Point2Dd> &polygon, const Point2Dd &p);
    static bool isPointInsidePolygon2D(const std::vector<Point2Df *> &polygon, const Point2Df &p, bool skipBound);
    static int isPrismAABBCollision(const std::vector<Point3Dd *> &rect, const Point3Dd &nv,
                                    const Point3Dd &ov,const double sz);

    static bool is2DSegmentAabbCollision(const double sp[2], const double ep[2],
                                         const double ov[2], const double lv);
    static bool is2DSegmentAabbCollisionf(const float sp[2], const float ep[2],
        const float ov[2], const float lv);

    static bool is2DLineAabbCollision(const double p[2], const double dv[2],
                                      const double ov[2], const double lv);
    static int calcIntPointsLineAABB(const double p[3], const double dir[3], const double mincoord[3],
        const double maxcoord[3], double intP[2][3]);
    static bool isPointInAABB(const Point3Dd& p,const Point3Dd& mincoord, const Point3Dd& maxcoord);
    static bool is2DTriangleAabbCollision(const double v1[2], const double v2[2],
                                          const double v3[2],const double ov[2], const double &lv);
    static bool calc2DIntOxLineWithSegment(const double *sp, const double *ep,
                                           const double &ycoord, double &intXCoord);
    static bool calc2DIntOyLineWithSegment(const double *sp, const double *ep,
                                           const double &xcoord, double &intYCoord);
    static int get2DBoundingBoxOf3Point(const double v1[2], const double v2[2],
                                        const double v3[2], double mincoord[2], double maxcoord[2]);
    static int calc2DCircumcentre(const double *p1,
                                  const double *p2, const double *p3, double *centre);
    static double calcSquareDistancePointToSegmentIncludeCase(const double p[3],
            const double sp[3], const double ep[3]);
    static bool isOrthorOfPointIncludeSegment(const double p[3],
            const double sp[3], const double ep[3]);
    static bool is2DCircleTriangleCollision(const double pv[2], const double r,
        const double trig[3][2]);
    static bool is2DPointInTriangle(const double pv[2], const double trig[3][2]);
    // Rotate a coordinate system to be perpendicular to the given normal
    static void rot_coord_sys(const Vector3Dd &old_u, const Vector3Dd &old_v,
                              const Vector3Dd &new_norm, Vector3Dd &new_u, Vector3Dd &new_v);
    static void proj_curv(const Vector3Dd &old_u, const Vector3Dd &old_v,
                          float old_ku, float old_kuv, float old_kv,
                          const Vector3Dd &new_u, const Vector3Dd &new_v,
                          float &new_ku, float &new_kuv, float &new_kv);

    static void proj_dcurv(const Vector3Dd &old_u, const Vector3Dd &old_v,
                           const Vector4Df old_dcurv,
                           const Vector3Dd &new_u, const Vector3Dd &new_v,
                           Vector4Df &new_dcurv);
    static void diagonalize_curv(const Vector3Dd &old_u, const Vector3Dd &old_v,
                                 float ku, float kuv, float kv,
                                 const Vector3Dd &new_norm,
                                 Vector3Dd &pdir1, Vector3Dd &pdir2, float &k1, float &k2);
    static int solveEquationOrder2(double a, double b, double c, double* roots);
    static int calcIntSegmentCircle(const Point3Dd &sp, const Point3Dd &ep,
        const Point3Dd &center, const Vector3Dd &v, const double &radius, double *t);
    static int calcIntSegmentCylinder(const Point3Dd &sp, const Point3Dd &ep,
        const Point3Dd &center, const Vector3Dd &v, const double &radius, double *t);
    static int calcIntSegmentCone(const Point3Dd &sp, const Point3Dd &ep,
        const Point3Dd &tip, const Vector3Dd &v, const double &cangle, double *t);
    static bool isPointInsideCone(const Point3Dd &p, const double &tipR, const double &baseR,
        const Point3Dd &TipCenter, const Point3Dd &baseCenter);
    static double calcSquareMinDistanceSegmentLine(const Point3Dd &sp, const Point3Dd &ep,
        const Point3Dd &pv, const Point3Dd &dir);
    static double calcSquareMinDistanceTwoLine(const Point3Dd &pv1, const Point3Dd &dir1,
        const Point3Dd &pv2, const Point3Dd &dir2);
    static bool calcNearestPointFromLineToOtherLine(const Point3Dd &p, const Vector3Dd &dv,
        const Point3Dd &otherP, const Vector3Dd &otherdv, Point3Dd &intp);
    static int calcIntQuadWithPrism(const std::vector<Point3Dd *> &quad,
                                    const std::vector<Point3Dd *> &rect, 
                                    const Vector3Dd &nv,
                                    std::vector<Point3Dd> &intp);
    static int calcIntBBoxWithPrism(const Point3Dd &minCoord, const Point3Dd &maxCoord,
                                    const std::vector<Point3Dd *> rect, const Point3Dd &nv,
                                    std::vector<Point3Dd> &intp);
    static int calQuadCollisionWithSegment(const std::vector<Point3Dd *> &quad,
        const Point3Dd &sp, const Point3Dd &ep, int* rect);
    static void calcXYOffsetContourOfcurve(const std::vector<Point3Dd*> &curve, const double &offVal, std::vector<Point3Dd> &contour);
    // Convert unit
    static double degreeToRadian(double degree) {
        return degree * DIV_PI_180;
    }

    static double radianToDegree(double radian) {
        return radian * DIV_180_PI;
    }

    static std::string &numberToString(int number, std::string &str) {
        std::ostringstream os;
        os << number;
        str = os.str();
        return str;
    }

    static std::string &numberToString(float number, std::string &str) {
        std::ostringstream os;
        os << number;
        str = os.str();
        return str;
    }

    static int stringToInt(const std::string &str, bool * isOk = nullptr) {
        std::istringstream is(str);
        int n;
        is >> n;

		if (isOk)
			*isOk = is.good();

		return n;
    }

    static float stringToFloat(const std::string &str, bool * isOk = nullptr) {
        std::istringstream is(str);
        float n;
        is >> n;

		if (isOk)
			*isOk = is.good();

        return n;
    }

    static char* getReservedBuffer() {
        if (!_reservedBuffer)
            _reservedBuffer = new char[RESERVED_BUFFER_SIZE];
        
        assert(_reservedBuffer);
        return _reservedBuffer;
    }

    static void deleteReservedBuffer() {
        if (_reservedBuffer) {
            delete _reservedBuffer;
            _reservedBuffer = nullptr;
        }
    }

    static unsigned int reservedBufferSize() {
        return _reservedBuffer ? RESERVED_BUFFER_SIZE : 0;
    }

	// Used to contain a pair of revision number: major and minor
	typedef std::pair<unsigned, unsigned> VersionPair;

	// Get VersionPair from version string in form of "major.minor" like "1.0", "2.5"
	static bool getMajorMinorVersion(std::string const & versionString, VersionPair & version);
    
    static Point3Dd calPointCoefficient(const Point3Dd& point, const Point3Dd& a, const Point3Dd& b, const Point3Dd& c);
    static bool calcEightVerticesOfBox(const Point3Dd& minCoord, const Point3Dd &maxCoord, Point3Dd boxVertices[8]);
    static bool calcNewBBoxAfterProjToPlane(const Point3Dd &oldMincoord, const Point3Dd &oldMaxcoord,
                                            const Point3Dd &p, const Vector3Dd &nv,
                                            Point3Dd &newMincoord, Point3Dd &newMaxcoord);
    static int createContoursFromSegmentList(Segment3DdListVector &segmentList, std::deque<std::deque<Point3Dd *>> &contours);
    static int closeContours(std::deque<std::deque<Point3Dd *>> &contours);

    static bool calcRightIntPointOfTwoCircles(double r1, double r2, double p1[2], double p2[2], double intp[2]);
    static void convertPointToCalib(double platfX, double paltfY, double expectX, double expectY, double AB, double BC,
        double CD, double DA, Point2Dd &p);
    static void convertPointToCalib(double platfX, double paltfY, double expectX, double expectY, const Point2Dd &A, const Point2Dd &B,
        const Point2Dd &C, const Point2Dd &D, Point2Dd &p);
    static Vector3Dd getVectorOnPlane(const Point3Dd &p, const Vector3Dd &norm);
    static Point3Dd getAPointOnPlane(const Point3Dd &p, const Vector3Dd &norm, double d = 1.0);
	static int intersectLineSphere(const Point3Dd &l1, const Point3Dd &l2, const Point3Dd &sp, const float r, Point3Dd &r_p1, Point3Dd &r_p2);
	static int intersectRaySphere(const Point3Dd& p, const Point3Dd& d, const Point3Dd& sCenter, float radius, Point3Dd& q);
	static int intersectRayConeFrustum(const Point3Dd& p, const Point3Dd& d, const Point3Dd& sCenter, float radius, const Point3Dd& sCenter1, float radius1, Point3Dd& q);
	static int intersectLineLineEpsilon(const Point3Dd& v1, const Point3Dd& v2, const Point3Dd& v3, const Point3Dd& v4, Point3Dd& i1, Point3Dd& i2, double epsilon);
	static bool intersectRayTriangle(
		const Point3Dd& org, const Point3Dd& dir,
		const Point3Dd v0, const Point3Dd v1, const Point3Dd v2,
		double& mindist);

	static Point3Dd projectVec3Vec3(const Point3Dd& v1, const Point3Dd& v2);
	static void     projectPointOnPlane(const Point3Dd& p, const Point3Dd& planePoint, const Point3Dd& planeNormal, Point3Dd& proj);
	/* Project v1 on v2 */
	static void project_v3_v3v3(Point3Dd& c, const Point3Dd& v1, const Point3Dd& v2);
protected:
    //static unsigned int _reservedBufferSize;
    static char* _reservedBuffer;
}; // Util class

/**************************************************************************************************
* Splits a string and puts resultant tokens in to a vector of StringClass.
*
* @param	str			   	The.
* @param	delim		   	The delimiter.
* @param [in,out]	results	The results.
* @param	empties		   	Whether to add the empty tokens.
*
* @return	The number of tokens found.
**************************************************************************************************/

template< typename StringClass >
int splitString(const char *str, const char *delim,
                std::vector<StringClass> &results, bool empties = true)
{
    char *pstr = const_cast<char *>(str);
    char *r = nullptr;
    r = strstr(pstr, delim);
    int dlen = strlen(delim);
    while( r != nullptr ) {
        char *cp = new char[(r-pstr)+1];
        memcpy(cp, pstr, (r-pstr));
        cp[(r-pstr)] = '\0';
        if( strlen(cp) > 0 || empties ) {
            StringClass s(cp);
            results.push_back(s);
        }
        delete[] cp;
        pstr = r + dlen;
        r = strstr(pstr, delim);
    }
    if( strlen(pstr) > 0 || empties ) {
        results.push_back(StringClass(pstr));
    }
    return results.size();
}
}

namespace baselib = BaseType;

#endif