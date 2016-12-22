#pragma once

#include <vector>
#include <unordered_map>

#include "StlLib/StlTriangle.h"
#include "Diagnotics/Log.h"

// This class indexes the triangles by Z value range, used for exporting slices
class TriangleIndexByZ
{
public:
    typedef std::unordered_map<size_t, StlTriangleSet> Layer2TriangleSet;
    typedef std::unordered_map<size_t, StlTriangleVector> Layer2TriangleVector;
    friend class ObjectEx;

public:
    TriangleIndexByZ()
        : _zMin(0.0)
        , _zMax(0.0)
        , _zStep(0.0)
    {
    }

    ~TriangleIndexByZ()
    {
    }

    StlTriangleVector & getTrianglesByZ(double const & z)
    {
        assert(BaseType::greater_than_or_equal(z, _zMin) && BaseType::less_than_or_equal(z, _zMax));

        size_t pos = (z - _zMin) / _zStep;

        size_t triCount = _triangleSet[pos].size();
        KLOG_DEBUG() << KLOG_VARIABLE(pos);
        KLOG_DEBUG() << KLOG_VARIABLE(triCount);

        std::unordered_map<size_t, StlTriangleSet> t;
        return _triangleList[pos];
    }

    bool const hasZ(double const & z) const
    {
        assert(BaseType::greater_than_or_equal(z, _zMin) && BaseType::less_than_or_equal(z, _zMax));

        size_t pos = (z - _zMin) / _zStep;

        KLOG_DEBUG() << KLOG_VARIABLE(pos);
        size_t triCount = const_cast<Layer2TriangleSet&>(_triangleSet)[pos].size();
        KLOG_DEBUG() << KLOG_VARIABLE(triCount);

        return _triangleSet.count(pos) > 0 && !_triangleSet.find(pos)->second.empty();
    }

    bool const hasIndex() const {return !_triangleList.empty() && !_triangleSet.empty();};

protected:
    // Mapping from the AaBb layer number to list of triangle it contains
    Layer2TriangleSet  _triangleSet; // Use set to remove duplicates

    Layer2TriangleVector _triangleList; // Use vector to sort later by a specific predicate

    // Min Z of Object's triangles
    double _zMin;

    // Max Z of Object's triangles
    double _zMax;

    // Size of layer
    double _zStep;
};

