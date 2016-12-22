#include <Windows.h>
#include <vector>
#include <array>
#include <gl/GLU.h>
#include "Point3Dd.h"

namespace Tesselator
{

//typedef std::array<GLdouble, 3> Point3d;

typedef Point3Dd Point3d;
typedef std::vector<Point3Dd> Polygon3d;
typedef std::vector<Polygon3d> PolygonList;

bool boundTesselatePolygon(PolygonList const & polygons, PolygonList & boundaries, int windingRule);

}
