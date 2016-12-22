#ifndef SCULPT_ANCHORED_STROKE_H
#define SCULPT_ANCHORED_STROKE_H
#include "sculpt/IStroke.h"
class AnchoredStroke : public IStroke
{
public:
    AnchoredStroke();
    ~AnchoredStroke();
    void startStroke(const double& mouseX, const double& mouseY);
    void doStroke(const double& mouseX, const double& mouseY);
    void endStroke(const double& mouseX, const double& mouseY);
    void processStroke();
    void saveData();
    void restoreData(); 
private:
    size_t _oldFlag;
    std::vector<BVH4::Node*> _lastNodes;
    Point2Dd _pixelCenter;
    Point3Dd _initialHitPoint;
    double _oldPixelRadius;
    bool _firstTime;

    StlVertex** _lastVertArr;
    size_t _sizeLastVertArr;
    size_t _numLastVerts;
};
#endif