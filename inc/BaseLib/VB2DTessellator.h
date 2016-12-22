#ifndef BASELIB_TESSELLATOR_H
#define BASELIB_TESSELLATOR_H
#include "KLinkList.h"
#include <vector>
#include <deque>
#include "Point3Dd.h"
#include "Point2Df.h"
#include "VBGrid2D.h"
namespace BaseType{

    typedef KLinkListEx<Point2Df> Point2DfLinkList;
    typedef VBGrid2D<Point2DfLinkList> WPGridLinkPoint2Df;

    class VB2DTessellator
    {
    private:
        unsigned _numberInitContour;
        WPGridLinkPoint2Df _grid;
        std::vector < Point2DfLinkList*> _contours;
        std::deque<unsigned> _indexTrigs;
        std::vector<Point2Df*> _pointCoord;
    public:
        VB2DTessellator();
        ~VB2DTessellator();
        void setOrigin(float origin[2]);
        void setSize(float size[2]);
        void addAContour(const std::vector<Point2Df*> &contour);
        void dumpToCad(Point2DfLinkList *contour, int color = 0);
        void dumpToCad(int color = 0);
        Point2DfLinkList* findOuterContour();
        void calcAngleOfContour(Point2DfLinkList *contour);
        void calcAngleOfAllContours();
        Point2DfLinkList* findVertexWithSmallestAngle(Point2DfLinkList *contour);
        void generateTriangles();
        void getData(float **coords, unsigned &coordSize, unsigned **index, unsigned &indexSize);
        void getTrianglesIndex(unsigned **index, unsigned &nts);
        
        Point2DfLinkList*  createTriangle(Point2DfLinkList *vmin);
        void setPolygonID(Point2DfLinkList *contour, const int id){ contour->setIDForAllLinkList(id); };
    };
}// namespace
#endif
