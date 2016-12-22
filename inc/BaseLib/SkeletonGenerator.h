#ifndef BASELIB_SKELETONGENERATOR_H
#define BASELIB_SKELETONGENERATOR_H
#include "DlnPoint.h"
#include "DlnEdge.h"
#include "DlnTriangle.h"
#include <deque>
#include <vector>
#include <stack>
namespace BaseType{

    class SkeletonGenerator
    {
	public:
		struct SkeLink;

		struct SkePoint
		{
			SkePoint(){};
			SkePoint(const SkePoint& o)
				:_coord(o._coord), _cordal0(o._cordal0), _cordal1(o._cordal1), _radius(o._radius)
			{};
			SkePoint(const Point2Dd& coord, const Point2Dd& cordal0, const Point2Dd& cordal1)
				:_coord(coord), _cordal0(cordal0), _cordal1(cordal1)
			{};

			Point2Dd _coord;
			Point2Dd _cordal0;
			Point2Dd _cordal1;
			double _radius;
			/*helper data for simplify link*/
			bool deleted;
		};

		struct SkeNode 
		{
			SkePoint* _point;
			DlnTriangle* _trig;
			std::vector<SkeLink*> _adjLinks;
		};

		struct SkeLink
		{
			SkeLink()
				:_node0(nullptr), _node1(nullptr), _avgChordalLength(0)
			{}
			std::vector<SkePoint*> _points;
			SkeNode* _node0;
			SkeNode* _node1;
			double _avgChordalLength;
			double _length;
		};

		/*helper class for simplify skeleton link*/
		struct SubPoly
		{
			SubPoly(const SubPoly& o)
				:_first(o._first), _last(o._last), _index(o._index)
			{};

			SubPoly(size_t first, size_t last)
				:_first(first), _last(last), _index(-1)
			{};

			size_t _first;
			size_t _last;
			int _index;
			double _maxDst;
		};
		struct Trapezoid
		{
			Point2Dd _negativeSegment[2];
			Point2Dd _origin;
			Point2Dd _positiveSegment[2];
			Point2Dd _axis;
		};

	private:
        std::deque<DlnPoint*> _skePoint;
        std::deque<DlnEdge*> _skeEdges;
        std::deque<DlnTriangle*> _skeTriangles;
        std::vector<SkePointLink*> _skeContours;
        std::vector<std::vector<Point2Dd*>*> _centerCurves;
		
		std::vector<SkeNode*> _skeNodes;
		std::vector<SkeLink*> _skeLinks;
		double _contourLength;
		double _skeletonLength;
    public:
        SkeletonGenerator();
        ~SkeletonGenerator();
        int addContour(const std::vector<Point2Dd*> &contour);
        int createASkeTriangle(DlnPoint* kp0, DlnPoint* kp1, DlnPoint* kp2);
        int triangulateSkeContour(SkePointLink* kp0, std::deque<SkePointLink*> &removed);
        SkePointLink* findMinAngleSkePoint(SkePointLink* kp);
        int calcBoundaryAngle(SkePointLink* kp);
        int createSkeTriangles();
        int filterShortEdge(SkePointLink* kp0);
        int calcBoundaryAngleAll(SkePointLink* kp0);
        int filterSkeTriangle();
        int generateSkeCurveFromSkeEdges();
        void generateSkeleton(const std::vector<Point2Dd*> &contour, std::vector<std::vector<Point2Dd*>*>& centerCurves);
		
		/*KhanhHH*/
		void makeDelaunayTriangulation();
		void addEdgesToStack(std::stack<DlnEdge*>& edges, DlnTriangle* trig);
		void flipEdge(DlnEdge* edge);
		bool collectQuadFromEdge(DlnEdge* e, DlnPoint* verts[4]);
		bool isectSegSeg(const Point2Dd& v1, const Point2Dd& v2, const Point2Dd& v3, const Point2Dd& v4);
		bool isConvexQuad(Point2Dd& a, Point2Dd& b, Point2Dd& c, Point2Dd& d);
		bool isInCircle(Point2Dd& pa, Point2Dd& pb, Point2Dd& pc, Point2Dd& pd);
		bool isInCircle1(Point2Dd& pa, Point2Dd& pb, Point2Dd& pc, Point2Dd& pd);

		void calcCircumCircleTriangle(const Point2Dd& p0, const Point2Dd& p1, const Point2Dd& p2, Point2Dd& pc, double& r);

		/*khanhHH*/
		float calcculateCircumcircleRadius(DlnTriangle* trig);
		void calculateTrapezoid(SkePoint* p0, SkePoint* p1, Trapezoid& trape);
		void findTrapezoidBestSplit(const std::vector<SkePoint*>& points, SubPoly& poly);
		void calculateExtremeNodeCordal(SkeNode* node, const Point2Dd& dir, SkePoint* p);
		bool isExtremeTriangle(DlnTriangle* tirg);

		void calcSkeNodePoint(SkeNode* node);
		void calcNodeRadius(SkeNode* node);

		void createSkeNodes();
		void createSkeLinks();
		void correctSkePointChordalAndRadius();
		void calcSkeLinkLengthInfor();
		
		void updateExtremeNodeChordal(SkeLink* link);
		void filterKeyPointsLinks();
		void filterKeyPointsLink(SkeLink* link);
		void simplifyLinks();
		bool isCircleCenterInsideCircle(const Point2Dd& c0, double r0, const Point2Dd& c1, double r1);
		bool isCircleCenterInsideCirclePercent(const Point2Dd& c0, double r0, const Point2Dd& c1, double r1, double percent);
		bool isCircleCenterIntersectCircle(const Point2Dd& c0, double r0, const Point2Dd& c1, double r1);

		void filterBranchNodes();

		void simplifyLink(SkeLink* link);
		void filterSphere();
		void filterSphere(SkeLink* link);
		void mergeShortBranchLink();
		void removeSmallLinkTips();
		void debugOutputSkeleton();

		void generateSkeleton(const std::vector<Point2Dd*> &contour);
		void removeRedundant2TypeTriangle();
		void recalcTriangleType();
		double calculateContourLength(const std::vector<Point2Dd*> &contour);
		double calculateSkeletonLength();

		std::vector<SkeNode*>& getSkeletonNodes(){ return _skeNodes; };
		std::vector<SkeLink*>& getSkeletonLinks(){ return _skeLinks; };

		void debugOutputLinkChordals();
		void debugOutputLink();
		void debugOutputContour(const std::vector<Point2Dd*>& contour);
		void debugOutputTriangle();
    };
}
#endif