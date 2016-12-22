#pragma once

#include <math.h>
#include <stdlib.h>
#include "misc/TriangleTriangleIntersection.h"

#ifndef NOMINMAX // Workaround for M$ use of min/max macro
#define NOMINMAX
#endif

using std::max;
#include <ppl/ppl_extras.h>

#ifdef NOMINMAX
#undef  NOMINMAX
#endif

namespace BaseType
{
	template<typename T>
	inline bool is_equal(const T & val1, const T & val2, const T & epsilon)
	{
		T diff = val1 - val2;
		assert(((-epsilon <= diff) && (diff <= T(epsilon))) == (abs(diff) <= T(epsilon)));

		return ((-T(epsilon) <= diff) && (diff <= T(epsilon)));
	}

	template<typename T>
	inline bool less_than_or_equal(const T & val1, const T & val2, const T & epsilon)
	{
		return (val1 < val2) || is_equal(val1,val2, epsilon);
	}

	template<typename T>
	inline bool greater_than_or_equal(const T & val1, const T & val2, const T & epsilon)
	{
		return (val1 > val2) || is_equal(val1,val2,epsilon);
	}

	template<typename Point>
	inline Plane<Point> make_plane(const double& x1, const double& y1, const double& z1,
		const double& x2, const double& y2, const double& z2,
		const double& x3, const double& y3, const double& z3)
	{
		Plane<Point> plane_;
		Point v1 = Point(x2 - x1, y2 - y1, z2 - z1);
		Point v2 = Point(x3 - x1, y3 - y1, z3 - z1);
		plane_.normal   = normalize(v1 * v2);
		plane_.constant = -dot_product(plane_.normal,make_vector(x1,y1,z1));
		return plane_;
	}

	template<typename Point>
	inline Plane<Point> make_plane(const Point& point1,
		const Point& point2,
		const Point& point3)
	{
		return make_plane(point1.x,point1.y,point1.z,
			point2.x,point2.y,point2.z,
			point3.x,point3.y,point3.z);
	}

	template<typename Point>
	inline Plane<Point> make_plane(const Triangle<Point>& triangle)
	{
		return make_plane(triangle[0],triangle[1],triangle[2]);
	}

	template<typename Point>
	inline bool intersect(const Segment<Point> & segment, const Box<Point> & box)
	{
		double cx = (box[0].x + box[1].x) * double(0.5);
		double cy = (box[0].y + box[1].y) * double(0.5);
		double cz = (box[0].z + box[1].z) * double(0.5);

		double ex = box[1].x - cx;
		double ey = box[1].y - cy;
		double ez = box[1].z - cz;

		double mx = (segment[0].x + segment[1].x) * double(0.5) - cx;
		double my = (segment[0].y + segment[1].y) * double(0.5) - cy;
		double mz = (segment[0].z + segment[1].z) * double(0.5) - cz;

		double dx = segment[1].x - mx;
		double dy = segment[1].y - my;
		double dz = segment[1].z - mz;

		double adx = abs(dx);
		if (abs(mx) > ex + adx) return false;

		double ady = abs(dy);
		if (abs(my) > ey + ady) return false;

		double adz = abs(dz);
		if (abs(mz) > ez + adz) return false;

		adx += double(Epsilon);
		ady += double(Epsilon);
		adz += double(Epsilon);

		if (abs(my * dz - mz * dy) > (ey * adz + ez * ady)) return false;
		if (abs(mz * dx - mx * dz) > (ex * adz + ez * adx)) return false;
		if (abs(mx * dy - my * dx) > (ex * ady + ey * adx)) return false;

		return true;
	}

	template<typename Point>
	bool intersect(const Triangle<Point> & tri1, const Triangle<Point> & tri2,
		bool & isCoplanar, Point & point1, Point & point2)
	{
		int inter =  tri_tri_intersection_test_3d(
			tri1[0].data(), tri1[1].data(), tri1[2].data(),
			tri2[0].data(), tri2[1].data(), tri2[2].data(),
			isCoplanar,
			point1.data(), point2.data());

		return inter != 0;
	}

	/**************************************************************************************************
	* Check whether a line segment intersect with a triangle.
	*
	* @param   r0              The first point of the segment.
	* @param   r1              The second point of the segment.
	* @param   t0              The first point of the triangle.
	* @param   t1              The second point of the triangle.
	* @param   t2              The third point of the triangle.
	* @param [out]  i          The intersection point.
	* @param   eps             (optional) The epsilon for float comparison.
	*
	* @return  Returns 1 if truly intersects, 0 if no intersections,
	* 			-1 if triangle degenerates, 2 if segment on triangle's plane.
	**************************************************************************************************/

	template<typename Point>
	int intersect(Point const & r0, Point const & r1,
		Point const & t0, Point const & t1, Point const & t2,
		Point & i, double const & eps)
	{
		Point    u, v, n;             // triangle vectors
		Point    dir, w0, w;          // ray vectors
		double     r, a, b;             // params to calc ray-plane intersect

		// get triangle edge vectors and plane normal
		u = t1 - t0;
		v = t2 - t0;
		n = u * v;             // cross product
		if (n.length() < eps)            // triangle is degenerate
			return -1;                 // do not deal with this case

		dir = r1 - r0;             // ray direction vector
		w0 = r0 - t0;
		a = -dot(n,w0);
		b = dot(n,dir);
		if (fabs(b) < eps) {     // ray is parallel to triangle plane
			if (a == 0)                // ray lies in triangle plane
				return 2;
			else return 0;             // ray disjoint from plane
		}

		// get intersect point of ray with triangle plane
		r = a / b;
		if (r < 0.0)                   // ray goes away from triangle
			return 0;                  // => no intersect
		// for a segment, also test if (r > 1.0) => no intersect

		i = r0 + r * dir;           // intersect point of ray and plane

		// is I inside T?
		float    uu, uv, vv, wu, wv, D;
		uu = dot(u,u);
		uv = dot(u,v);
		vv = dot(v,v);
		w = i - t0;
		wu = dot(w,u);
		wv = dot(w,v);
		D = uv * uv - uu * vv;

		// get and test parametric coords
		float s, t;
		s = (uv * wv - vv * wu) / D;
		if (s < 0.0 || s > 1.0)        // I is outside T
			return 0;
		t = (uv * wu - uu * wv) / D;
		if (t < 0.0 || (s + t) > 1.0)  // I is outside T
			return 0;

		return 1;                      // I is in T
	}

	template<typename Point>
	int intersect(Point const & r0, Point const & r1, Triangle<Point> const & t,
		Point & i, double const & eps = Epsilon_Low)
	{
		return intersect(r0, r1, t[0], t[1], t[2], i, eps);
	}

	template<typename Point>
	bool intersection_point(const Point& point1, const Point& point2,
		const Point& t1, const Point& t2, const Point& t3)
	{
		normal = (t3-t1).cross(t2-t1)
			Point seg_vec = point2 - point1;
		double denom = seg_vec.dot(plane.normal);
		point3d<T> ipoint = degenerate_point3d<T>();
		if (not_equal(denom,T(0.0)))
		{
			T t = -distance(segment[0],plane) / denom;
			if ((t > T(0.0)) && (t < T(1.0)))
			{
				ipoint = segment[0] + t * (segment[1] - segment[0]);
			}
		}
		return ipoint;
	}

	template<typename T>
	inline int orientation(const T& x1, const T& y1,
		const T& x2, const T& y2,
		const T& px, const T& py)
	{
		T orin = (x2 - x1) * (py - y1) - (px - x1) * (y2 - y1);

		if (orin > T(0.0))      return LeftHandSide;         /* Orientation is to the left-hand side  */
		else if (orin < T(0.0)) return RightHandSide;        /* Orientation is to the right-hand side */
		else                    return CollinearOrientation; /* Orientation is neutral aka collinear  */
	}

	template<typename T>
	inline int orientation(const T& x1, const T& y1, const T& z1,
		const T& x2, const T& y2, const T& z2,
		const T& x3, const T& y3, const T& z3,
		const T& px, const T& py, const T& pz)
	{
		T px1 = x1 - px;
		T px2 = x2 - px;
		T px3 = x3 - px;

		T py1 = y1 - py;
		T py2 = y2 - py;
		T py3 = y3 - py;

		T pz1 = z1 - pz;
		T pz2 = z2 - pz;
		T pz3 = z3 - pz;

		T orin = px1 * (py2 * pz3 - pz2 * py3) +
			px2 * (py3 * pz1 - pz3 * py1) +
			px3 * (py1 * pz2 - pz1 * py2);

		if (orin < T(0.0))      return BelowOrientation;    /* Orientaion is below plane                      */
		else if (orin > T(0.0)) return AboveOrientation;    /* Orientaion is above plane                      */
		else                    return CoplanarOrientation; /* Orientaion is coplanar to plane if Result is 0 */
	}

	template<typename Point>
	inline int orientation(const Point& point1,
		const Point& point2,
		const double&           px,
		const double&           py)
	{
		return orientation(point1.x,point1.y,point2.x,point2.y,px,py);
	}

	template<typename Point>
	inline int orientation(const Point& point1,
		const Point& point2,
		const Point& point3)
	{
		return orientation(point1.x,point1.y,point2.x,point2.y,point3.x,point3.y);
	}

	//template<typename PointType>
	//inline bool convex_vertex(const size_t& index, const Polygon<PointType>& polygon, const int& polygon_orientation)
	//{
	//    if (0 == index)
	//    {
	//        return (orientation(polygon.back(),polygon.front(),polygon[1]) == polygon_orientation);
	//    }
	//    else if (index == (polygon.size() - 1))
	//    {
	//        return (orientation(polygon[polygon.size() - 2],polygon.back(),polygon.front()) == polygon_orientation);
	//    }
	//    else
	//    {
	//        return (orientation(polygon[index - 1],polygon[index],polygon[index + 1]) == polygon_orientation);
	//    }
	//}

	template<typename Point>
	inline Triangle<Point> make_triangle(const Point& point1, const Point& point2, Point const& point3)
	{
		return Triangle<Point>(point1, point2, point3);
	}

	template<typename Point>
	inline bool vertex_is_ear(const size_t& index, const Polygon<Point>& polygon)
	{
		size_t pred_index;
		size_t succ_index;
		if (0 == index)
		{
			pred_index = polygon.size() - 1;
			succ_index = 1;
		}
		else if (index == polygon.size() - 1)
		{
			pred_index = polygon.size() - 2;
			succ_index = 0;
		}
		else
		{
			pred_index = index - 1;
			succ_index = index + 1;
		}

		Triangle<Point> triangle = Triangle<Point>(polygon[pred_index],polygon[index],polygon[succ_index]);

		if (robust_collinear(triangle[0],triangle[1],triangle[2]))
		{
			return false;
		}

		for(size_t i = 0; i < polygon.size(); ++i)
		{
			if ((i != pred_index) && (i != succ_index) && (i != index))
			{
				if (point_in_triangle(polygon[i],triangle))
				{
					return false;
				}
			}
		}
		return true;
	}

	template<typename Point>
	inline Triangle<Point> vertex_triangle(const size_t& index, const Polygon<Point> polygon)
	{
		if (0 == index)
		{
			return Triangle<Point>(polygon.back(),polygon.front(),polygon[1]);
		}
		else if (index == (polygon.size() - 1))
		{
			return Triangle<Point>(polygon[polygon.size() - 2],polygon.back(),polygon.front());
		}
		else
		{
			return Triangle<Point>(polygon[index - 1],polygon[index],polygon[index + 1]);
		}
	}

	template <typename Point>
	inline int polygon_orientation(const Polygon<Point>& polygon)
	{
		if (polygon.size() < 3)
		{
			return 0;
		}
		double area = 0.0;
		size_t prev_index = polygon.size() - 1;

		for(size_t index = 0; index < polygon.size(); ++index)
		{
			area += (polygon[prev_index].x * polygon[index].y - polygon[index].x * polygon[prev_index].y);
			prev_index = index;
		}

		return ((greater_than_or_equal(area,double(0.0)))? CounterClockwise : Clockwise);

		/*
		size_t anchor = 0;
		for(size_t i = 1; i < polygon.size(); ++i)
		{
		if (polygon[i].x > polygon[anchor].x)
		anchor = i;
		else if ((polygon[i].x == polygon[anchor].x) && (polygon[i].y  < polygon[anchor].y))
		anchor = i;
		}

		if (0 == anchor)
		{
		return orientation(*(polygon.end() - 1),*polygon.end(),*polygon.begin());
		}
		else if (anchor == (polygon.size() - 1))
		{
		return orientation(*(polygon.end() - 2),*(polygon.end() - 1),*polygon.end());
		}
		else
		{
		return orientation(polygon[anchor - 2],polygon[anchor - 1],polygon[anchor]);
		}
		*/
	}

	const int Clockwise            = -1;
	const int CounterClockwise     = +1;

	template<typename Point>
	struct polygon_triangulate
	{
	public:
		template<typename OutputIterator>
		polygon_triangulate(const Polygon3d& polygon, OutputIterator out)
		{
			Polygon3d internal_polygon;
			internal_polygon.reserve(polygon.size());
			copy(polygon.begin(),polygon.end(),back_inserter(internal_polygon));
			if (polygon_orientation(internal_polygon) != Clockwise)
			{
				internal_polygon.reverse();
			}

			while(internal_polygon.size() > 3)
			{
				for(size_t i = 0; i < internal_polygon.size(); ++i)
				{
					if (convex_vertex(i,internal_polygon,Clockwise) && vertex_is_ear(i,internal_polygon))
					{
						(*out++) = vertex_triangle(i,internal_polygon);
						internal_polygon.erase(i);
						break;
					}
				}
			}
			(*out++) = vertex_triangle(1,internal_polygon);
		}
	};

	template<typename Point>

	/**************************************************************************************************
	* Tests if point line in cone.
	*
	* @param   center      The center.
	* @param   normal      The normal.
	* @param   baseRadius  The base radius.
	* @param   topRadius   The top radius.
	* @param   height      The height.
	* @param   x           The Point const &amp; to process.
	* @param   eps         The EPS.
	*
	* @return  true if it succeeds, false if it fails.
	**************************************************************************************************/

	bool point_in_cone(Point const & vertex, Point const & axis,
		double const & angle, double const & height, Point const & x, double eps)
	{
		Point xv = x-vertex;
		bool positive = dot(axis, xv) > length(xv) * cos(angle) + eps;

		return positive;
	}

	template<typename Point>
	bool point_in_cone(Cone<Point> const & cone, Point const & x, double eps)
	{
		return point_in_cone(cone.vertex, cone.axis, cone.angle, cone.height, x, eps);
	}

	// Let the vertex be V, the unit-length direction vector be A,
	// and the angle measured from the cone axis to the cone wall be Theta,
	// and define g = cos(Theta).  A point X is on the cone wall whenever
	// Dot(A,(X-V))/|X-V| = g. A point inside the cone is tested against by
	// replacing '=' with '>='. To avoid calculating of square root, one
	// can write Dot(A,(X-V))^2 = g^2 * |X-V|
	template<typename Point>
	bool point_on_cone(Point const & vertex, Point const & axis,
		double const & angle, double const & height, Point const & x, double eps)
	{
		Point xv = x-vertex;
		bool positive = is_equal(
			dot(axis, xv), length(xv) * cos(angle),
			eps);

		return positive;
	}

	template<typename Point>
	bool point_on_cone(Cone<Point> const & cone, Point const & x, double eps)
	{
		Point center;
		return point_on_cone(cone.vertex, cone.axis, cone.angle, cone.height, x, eps);
	}

	template<typename Point>
	bool point_in_cylinder(Point const & center, Point const & axis,
		double const & radius, double const & height, Point const & P, double eps)
	{
		Point CP1 = center;

		Point CN1 = axis; //CP2 - CP1;
		//CN1.Normalize();

		double fDistanceToPlane = dot( P-CP1, CN1 );

		if (less_than_or_equal(fDistanceToPlane, 0.0 , eps))	
			return false;

		Point CP2 = center + axis * height;
		Point CN2 = -CN1;
		if (less_than_or_equal(dot( P-CP2, CN2 ), 0.0, eps)) 
			return false;

		Point TempP =  P - (CN1 * fDistanceToPlane);
		double fDistanceFromCenter = length( TempP - CP1);

		if (greater_than_or_equal(fDistanceFromCenter, radius, eps)) 
			return false;

		return true; // All tests passed, point is in cylinder
	}

	template<typename Point>
	bool point_in_cylinder(Cylinder<Point> const & cylinder, Point const & P, double eps)
	{
		return point_in_cylinder(cylinder.center, cylinder.axis, cylinder.radius, cylinder.height, P, eps);
	}

	template<typename Point>
	bool point_in_triangle(Point const & px, Point const & p0, Point const & p1, Point const & p2)
	{
		// If point px is same as each vertex
		PointEqual3d pe;
		if (pe(p0, px) || pe(p1, px) || pe(p2, px) )
			return false;

		// Compute vectors
		Point v0 = p2 - p0;
		Point v1 = p1 - p0;
		Point v2 = px - p0;
		Point norm = v0 * v1;

		if (norm.length2() < Epsilon_High) // Too thin triangle
			return false;

		if (norm.dot(v2) > Epsilon_Low) // Point not in the same plane of the triangle
			return false;

		// Compute dot products
		double
			dot00 = v0.dot(v0),
			dot01 = v0.dot(v1),
			dot02 = v0.dot(v2),
			dot11 = v1.dot(v1),
			dot12 = v1.dot(v2);

		// Compute barycentric coordinates
		double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
		double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

		// Check if point is in triangle
		return (u >= 0) && (v >= 0) && (u + v < 1);
	}

	template<typename Point>
	bool point_in_triangle(Point const & px, Triangle<Point> const & tri)
	{
		return point_in_triangle(px, tri[0], tri[1], tri[2]);
	}

	template<typename Point>
	Point triangle_normal(Point const & p0, Point const & p1, Point const & p2)
	{
		return (p1 - p0)*(p2 - p1);
	}

	template<typename Point>
	Point triangle_normal(Triangle<Point> const & tri)
	{
		return (tri[1] - tri[0])*(tri[2] - tri[1]);
	}

	template<typename Container, typename Less, typename Equal>
	void remove_duplicates(Container & cont, Less less, Equal equal)
	{
		sort_parallel(cont.begin(), cont.end(), less);
		auto it = std::unique(cont.begin(), cont.end(), equal);
		cont.resize(it - cont.begin());

		cont.shrink_to_fit();
	}

	template<typename Container>
	void remove_duplicates(Container & cont)
	{
		std::sort(cont.begin(), cont.end());
		auto it = std::unique(cont.begin(), cont.end());
		cont.resize(it - cont.begin());

		cont.shrink_to_fit();
	}

	/**************************************************************************************************
	 * Removes the duplicates using radix sort (used for integer items).
	 *
	 * @param [in,out]	cont	The container.
	 **************************************************************************************************/

	template<typename Container>
	void remove_duplicates_radix(Container & cont)
	{
		Concurrency::samples::parallel_radixsort(cont.begin(), cont.end());
		auto it = std::unique(cont.begin(), cont.end());
		cont.resize(it - cont.begin());

		cont.shrink_to_fit();
	}

	template<typename Point>
	void make_plane(Point const & p1, Point const & p2, Point const & p3, Plane<Point> & plane)
	{
		make_plane(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z,
			plane.normal, plane.constant);
	}

	template<typename Point>
	Segment3d make_segment(Point const & p1, Point const & p2)
	{
		return Segment3d(p1, p2);
	}

	/**************************************************************************************************
	 * Calculate distance from a point to a line.
	 *
	 * @param	p 	The Point const &amp; to process.
	 * @param	sp	The start point.
	 * @param	ep	The end point.
	 *
	 * @return	The distance.
	 **************************************************************************************************/

	template<typename Point>
	double dist_point_to_line( Point const & p, Point const & sp, Point const & ep)
	{
		Point v = ep - sp;
		Point w = p - sp;

		double c1 = dot(w,v);
		if ( c1 <= 0 )
			return length(p-s[0]);

		double c2 = length2(v);
		if ( c2 <= c1 )
			return length(p-s[1]);

		double b = c1 / c2;
		Point pb = s[0] + b * v;
		return length(p-pb);	
	}

	/**************************************************************************************************
  	 * Calculate distance from a point to a line.
	 *
	 * @param	p	The start point.
	 * @param	s	The end point.
	 *
	 * @return	The distance.
	 **************************************************************************************************/

	template<typename Point, typename Segment>
	double dist_point_to_line( Point const & p, Segment s)
	{
		return dist_point_to_line(p, s[0], s[1]);
	}

	/**************************************************************************************************
	 * Closest point on plane from point.
	 *
	 * @param	plane	  	The plane.
	 * @param	px		  	The input point.
	 * @param [out]	po	The output point.
	 *
	 **************************************************************************************************/

	template<typename Point>
	void closest_point_on_plane_from_point(const Plane<Point>& plane,
		const Point& px, Point & po)
	{
		double mu = 
			plane.normal.x * px.x +
			plane.normal.y * px.y +
			plane.normal.z * px.z  - plane.constant;

		if (is_equal(mu,double(0.0)))
		{
			po = px;
		}
		else
		{
			po.x = point.x - mu * plane.normal.x;
			po.y = point.y - mu * plane.normal.y;
			po.x = point.z - mu * plane.normal.z;
		}
	}

	/**************************************************************************************************
	* Find the closest point on line from a point.
	 *
	 * @param	x1		  	The x value of the first end point.
	 * @param	y1		  	The y value of the first end point.
	 * @param	z1		  	The z value of the first end point.
	 * @param	x2		  	The x value of the second end point.
	 * @param	y2		  	The y value of the second end point.
	 * @param	z2		  	The z value of the second end point.
	 * @param	px		  	The x value of the input point.
	 * @param	py		  	The y value of the input point.
	 * @param	pz		  	The z value of the input point.
	 * @param [in,out]	nx	The x value of the output point.
	 * @param [in,out]	ny	The y value of the output point.
	 * @param [in,out]	nz	The z value of the output point.
	 **************************************************************************************************/

	template<typename T>
	inline void closest_point_on_line_from_point(const T& x1, const T& y1, const T& z1,
		const T& x2, const T& y2, const T& z2,
		const T& px, const T& py, const T& pz,
		T& nx,       T& ny,       T& nz)
	{
		T vx = x2 - x1;
		T vy = y2 - y1;
		T vz = z2 - z1;
		T wx = px - x1;
		T wy = py - y1;
		T wz = pz - z1;

		T c1 = vx * wx + vy * wy + vz * wz;
		T c2 = vx * vx + vy * vy + vz * vz;

		T ratio = c1 / c2;

		nx = x1 + ratio * vx;
		ny = y1 + ratio * vy;
		nz = z1 + ratio * vz;
	}

	/**************************************************************************************************
	 * Find the closest point on line from a point.
	 *
	 * @param	p1		  	The first end point of the line.
	 * @param	p2		  	The second end point of the line.
	 * @param	px		  	The input point.
	 * @param [in,out]	po	The output point.
	 **************************************************************************************************/

	template<typename Point>
	inline void closest_point_on_line_from_point(const Point& p1, const Point& p2,
		const Point& px, Point& po)
	{
		closest_point_on_line_from_point(
			p1.x, p1.y, p1.z,
			p2.x, p2.y, p2.z,
			px.x, px.y, px.z,
			po.x, po.y, po.z);
	}


	/**************************************************************************************************
	 * Find the line-plane intersection point.
	 *
	 * @param   p1          The first point on line.
	 * @param   p2          The second point on line.
	 * @param   p3          A point on the plane.
	 * @param   n           The plane normal vector.
	 * @param [out]  p   The output point.
	 *
	 * @return  true if it succeeds, false if it fails.
	 **************************************************************************************************/
	template<typename Point>
	inline bool line_plane_intersect(const Point& p1, const Point& p2,
		const Point& p3, const Point& n, Point & p)
	{
		Point3Dd
			p31 = p3 - p1,
			p21 = p2 - p1;

		double denom = dot(n, p21);

		if (is_equal(denom,double(0.0)))
			return false;
		else
		{
			double u = dot(n, p31) / denom;

			p = p1 + u * p21;

			return true;
		}
	}

	//void make_plane(const double& x1, const double& y1, const double& z1,
	//    const double& x2, const double& y2, const double& z2,
	//    const double& x3, const double& y3, const double& z3, Point3Dd & normal, double & constant)
	//{
	//    Point3Dd v1(x2 - x1, y2 - y1, z2 - z1);
	//    Point3Dd v2(x3 - x1, y3 - y1, z3 - z1);
	//    normal   = (v1 * v2);

	//    assert(normal.length2() > 1e-10);

	//    normal.normalize();
	//    constant = -normal.dot(Point3Dd(x1,y1,z1));
	//}

	template<typename T>
	inline T lay_distance_segment_to_segment(const T& x1, const T& y1, const T& z1,
		const T& x2, const T& y2, const T& z2,
		const T& x3, const T& y3, const T& z3,
		const T& x4, const T& y4, const T& z4)
	{
		T ux = x2 - x1;
		T uy = y2 - y1;
		T uz = z2 - z1;

		T vx = x4 - x3;
		T vy = y4 - y3;
		T vz = z4 - z3;

		T wx = x1 - x3;
		T wy = y1 - y3;
		T wz = z1 - z3;

		T a  = (ux * ux + uy * uy + uz * uz);
		T b  = (ux * vx + uy * vy + uz * vz);
		T c  = (vx * vx + vy * vy + vz * vz);
		T d  = (ux * wx + uy * wy + uz * wz);
		T e  = (vx * wx + vy * wy + vz * wz);
		T dt = a * c - b * b;

		T sd = dt;
		T td = dt;

		T sn = T(0.0);
		T tn = T(0.0);

		if (is_equal(dt,T(0.0)))
		{
			sn = T(0.0);
			sd = T(1.00);
			tn = e;
			td = c;
		}
		else
		{
			sn = (b * e - c * d);
			tn = (a * e - b * d);
			if (sn < T(0.0))
			{
				sn = T(0.0);
				tn = e;
				td = c;
			}
			else if (sn > sd)
			{
				sn = sd;
				tn = e + b;
				td = c;
			}
		}

		if (tn < T(0.0))
		{
			tn = T(0.0);
			if (-d < T(0.0))
				sn = T(0.0);
			else if (-d > a)
				sn = sd;
			else
			{
				sn = -d;
				sd = a;
			}
		}
		else if (tn > td)
		{
			tn = td;
			if ((-d + b) < T(0.0))
				sn = T(0.0);
			else if ((-d + b) > a)
				sn = sd;
			else
			{
				sn = (-d + b);
				sd = a;
			}
		}

		T sc = T(0.0);
		T tc = T(0.0);

		if (is_equal(sn,T(0.0)))
			sc = T(0.0);
		else
			sc = sn / sd;

		if (is_equal(tn,T(0.0)))
			tc = T(0.0);
		else
			tc = tn / td;

		T dx = wx + (sc * ux) - (tc * vx);
		T dy = wy + (sc * uy) - (tc * vy);
		T dz = wz + (sc * uz) - (tc * vz);
		return dx * dx + dy * dy + dz * dz;
	}

	template<typename T>
	inline bool intersect(const T& x1, const T& y1, const T& z1,
		const T& x2, const T& y2, const T& z2,
		const T& x3, const T& y3, const T& z3,
		const T& x4, const T& y4, const T& z4,
		const T& fuzzy)
	{
		return (less_than_or_equal(lay_distance_segment_to_segment(x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4),fuzzy));
	}

	template<typename Point>
	inline bool intersect(const Point& point1,
		const Point& point2,
		const Point& point3,
		const Point& point4,
		const double& fuzzy)
	{
		return intersect(point1.x, point1.y, point1.z,
			point2.x, point2.y, point2.z,
			point3.x, point3.y, point3.z,
			point4.x, point4.y, point4.z,fuzzy);
	}

	// a point X is between two other points A and B, if the distance AX
	// added to the distance BX is equal to the distance AB.
	template<typename Point>
	bool point_on_segment(Point const & a, Point const & b, Point const & p, double const & eps)
	{
		return is_equal(length(a-p) + length(b-p), length(b-a), eps);
	}

	template<typename Point>
	bool point_on_segment(Segment<Point> const & segment, Point const & p, double eps)
	{
		return point_on_segment(segment[0], segment[1], p, eps);
	}

	template<typename Point>
	bool make_cone(Point const & center, // User clicked point in the model
		Point const & normal, // Normal vector of the clicked triangle
		double baseRadius, // Base radius of the hole
		double topRadius, // Top radius of the hole
		double thickness, // Estimated thickness of the wall
		Cone<Point> & cone)
	{
		assert(greater_than_or_equal(baseRadius, 0.0) &&
			greater_than_or_equal(topRadius, 0.0) &&
			greater_than_or_equal(thickness, 0.0));

		if (is_equal(baseRadius, topRadius, 1e-2))
			return false;

		double baseR, topR;
		Point norm, cen;
		if (baseRadius > topRadius)
		{
			baseR = baseRadius;
			topR = topRadius;
			norm = normal;
			cen = center;
		}
		else
		{
			baseR = topRadius;
			topR = baseRadius;
			cen = center + thickness*normal;
			norm = -normal;
		}

		double diffR = baseR - topR;
		//double topHeight = topR * topR / diffR;

		cone.trheight = thickness;
		cone.height = (baseR * cone.trheight) / diffR;
		cone.angle = atan2(baseR, cone.height);
		cone.vertex = cen + cone.height * norm;
		cone.axis = -norm;

		return true;
	}

	/**************************************************************************************************
	 * Generate approximate segments for a cone.
	 *
	 * @param	cone				The cone.
	 * @param [in,out]	segments	The segments.
	 * @param	nvertices			The nvertices.
	 *
	 * @return	.
	 **************************************************************************************************/

	template<typename Point>
	int cone_segments(Cone<Point> const & cone, std::vector<Segment<Point>> & segments, int nvertices)
	{
		assert(nvertices);

		Point xaxis(1, 0, 0); // Initially use X axis to calculate normal
		Point perp = cross(cone.axis, xaxis);

		Point caxis = cone.axis;
		// Get the perpendicular vector to normal vector
		if (is_equal(perp.length(), 0.0, double(Epsilon_Low))) // cone.axis is parallel to xaxis
		{
			Point yaxis(0, 1, 0); // Use Y axis instead
			perp = cross(caxis, yaxis);
		}
		perp.normalize();

		Point baseCenter = cone.vertex + cone.height * caxis,
			topCenter = cone.vertex + (cone.height - cone.trheight) * caxis;

		double tange = tan(cone.angle);
		double baseRadius = cone.height * tange,
			topRadius = (cone.height - cone.trheight) * tange;

		Point baseVec = perp * baseRadius,
			topVec = perp * topRadius;

		segments.reserve(nvertices);
		Segment<Point> seg;
		double inc = 2*PI / nvertices;

		for (int i = 0; i <= nvertices; i++)
		{
			seg.set(baseCenter + baseVec, topCenter + topVec);
			segments.push_back(seg);
			baseVec.rotAroundAVector(caxis, inc);
			topVec.rotAroundAVector(caxis, inc);
		}

		return segments.size();
	}

	/**************************************************************************************************
	 * Generate approximate points for a cone.
	 *
	 * @param	cone		  	The cone.
	 * @param [in,out]	points	The points.
	 * @param	nvertices	  	The nvertices.
	 *
	 * @return	.
	 **************************************************************************************************/

	template<typename Point>
	int cone_points(Cone<Point> const & cone, std::vector<Point> & points, int nvertices)
	{
		assert(nvertices);

		Point xaxis(1, 0, 0); // Initially use X axis to calculate normal
		Point perp = cross(cone.axis, xaxis);

		Point caxis = cone.axis;
		// Get the perpendicular vector to normal vector
		if (is_equal(perp.length(), 0.0, double(Epsilon_Low))) // cone.axis is parallel to xaxis
		{
			Point yaxis(0, 1, 0); // Use Y axis instead
			perp = cross(caxis, yaxis);
		}
		perp.normalize();

		Point baseCenter = cone.vertex + cone.height * caxis,
			topCenter = cone.vertex + (cone.height - cone.trheight) * caxis;

		double tange = tan(cone.angle);
		double baseRadius = cone.height * tange,
			topRadius = (cone.height - cone.trheight) * tange;

		Point baseVec = perp * baseRadius,
			topVec = perp * topRadius;

		points.reserve(nvertices*2);
		double inc = 2*PI / nvertices;

		for (int i = 0; i < nvertices; i++)
		{
			points.push_back(baseCenter + baseVec);
			points.push_back(topCenter + topVec);

			baseVec.rotAroundAVector(caxis, inc);
			topVec.rotAroundAVector(caxis, inc);
		}

		return points.size();
	}

	/**************************************************************************************************
	 * Generate approximate points for a cylinder.
	 *
	 * @param	cylinder	  	The cylinder.
	 * @param [in,out]	points	The points.
	 * @param	nvertices	  	The nvertices.
	 *
	 * @return	.
	 **************************************************************************************************/

	template<typename Point>
	int cylinder_points(Cylinder<Point> const & cylinder, std::vector<Point> & points, int nvertices)
	{
		assert(nvertices);

		Point xaxis(1, 0, 0); // Initially use X axis to calculate normal
		Point perp = cross(cylinder.axis, xaxis);

		Point caxis = cylinder.axis;
		// Get the perpendicular vector to normal vector
		if (is_equal(perp.length(), 0.0, double(Epsilon_Low))) // cone.axis is parallel to xaxis
		{
			Point yaxis(0, 1, 0); // Use Y axis instead
			perp = cross(caxis, yaxis);
		}
		perp.normalize();

		Point baseCenter = cylinder.center,
			topCenter = baseCenter + cylinder.axis*cylinder.height;

		Point baseVec = perp * cylinder.radius,
			topVec = perp * cylinder.radius;

		points.reserve(nvertices*2);
		double inc = 2*PI / nvertices;

		for (int i = 0; i < nvertices; i++)
		{
			points.push_back(baseCenter + baseVec);
			points.push_back(topCenter + topVec);

			baseVec.rotAroundAVector(caxis, inc);
			topVec.rotAroundAVector(caxis, inc);
		}

		return points.size();
	}

	/**************************************************************************************************
	 * Generate approximate segments for a cylinder.
	 *
	 * @param	cylinder			The cylinder.
	 * @param [in,out]	segments	The segments.
	 * @param	nvertices			The nvertices.
	 *
	 * @return	.
	 **************************************************************************************************/

	template<typename Point>
	int cylinder_segments(Cylinder<Point> const & cylinder, std::vector<Segment<Point>> & segments, int nvertices)
	{
		assert(nvertices);

		Point xaxis(1, 0, 0); // Initially use X axis to calculate normal
		Point perp = cross(cylinder.axis, xaxis);

		// Get the perpendicular vector to normal vector
		if (is_equal(perp.length(), 0.0, double(Epsilon_Low))) // cone.axis is parallel to xaxis
		{
			Point yaxis(0, 1, 0); // Use Y axis instead
			perp = cross(cylinder.axis, yaxis);
		}
		perp.normalize();

		Point baseCenter = cylinder.center,
			topCenter = cylinder.center + cylinder.height * cylinder.axis;

		Point baseVec = perp * cylinder.radius,
			topVec = perp * cylinder.radius;

		segments.reserve(nvertices);
		Segment<Point> seg;
		double inc = 2*PI / nvertices;

		for (int i = 0; i <= nvertices; i++)
		{
			seg.set(topCenter + topVec, baseCenter + baseVec);
			segments.push_back(seg);
			baseVec.rotAroundAVector(cylinder.axis, inc);
			topVec.rotAroundAVector(cylinder.axis, inc);
		}

		return segments.size();
	}

	/**************************************************************************************************
	* Half-space test for a point: Which side of a plane a point lies.
	*
	* @param   plane   The plane.
	* @param   point   The point.
	*
	* @return  0 if points lies on plane, 1 if above, and -1 if behind.
	**************************************************************************************************/

	template<typename Point>
	int hasfspace_test(Plane<Point> const & plane, Point const & point, double const & eps)
	{
		Point v = point - plane.point;
		double res = dot(v, plane.normal);

		if (is_equal(res, 0.0, eps))
			return 0;
		else if (res > 0.0)
			return 1;
		else
			return -1;
	}

	template<typename T>
	inline bool box_to_box_intersect(const T& x1, const T& y1, const T& z1,
		const T& x2, const T& y2, const T& z2,
		const T& x3, const T& y3, const T& z3,
		const T& x4, const T& y4, const T& z4)
	{
		return ((x1 <= x4) && (x2 >= x3) && (y1 <= y4) && (y2 >= y3) && (z1 <= z4) && (z2 >= z3));
	}

	template<typename Point>
	inline bool intersect(const Box<Point>& box1, const Box<Point>& box2)
	{
		return box_to_box_intersect(box1[0].x,box1[0].y,box1[0].z,
			box1[1].x,box1[1].y,box1[1].z,
			box2[0].x,box2[0].y,box2[0].z,
			box2[1].x,box2[1].y,box2[1].z);
	}

	template<typename Point, typename PointList>
	inline void make_box(const PointList & polygon, Box<Point>& box)
	{
		assert(!polygon.empty());

		box = Box<Point>(polygon.front(), polygon.front());

		for (size_t i = 0; i < polygon.size(); i++)
		{
			box[0].x = std::min(box[0].x, polygon[i].x); box[1].x = std::max(box[1].x, polygon[i].x);
			box[0].y = std::min(box[0].y, polygon[i].y); box[1].y = std::max(box[1].y, polygon[i].y);
			box[0].z = std::min(box[0].z, polygon[i].z); box[1].z = std::max(box[1].z, polygon[i].z);
		}
	}

	/**************************************************************************************************
	 * Calculates the normal of plane from 3 points.
	 *
	 * @param	p0			  	The first point.
	 * @param	p1			  	The second.
	 * @param	p2			  	The third point.
	 * @param [in,out]	normal	The normal.
	 *
	 * @return	The calculated normal.
	 **************************************************************************************************/

	template<typename Point>
	Point3Dd calc_normal(Point const & p0, Point const & p1, Point const & p2, Point3Dd & normal )
	{
		normal = (p1 - p0).cross(p2 - p0);

		normalize(normal);

		return normal;
	}

	/**************************************************************************************************
	 * Calculates the normal using the first 3 vertices.
	 *
	 * @param	poly		  	The polygon.
	 * @param [in,out]	normal	The normal.
	 *
	 * @return	The calculated normal.
	 **************************************************************************************************/

	template<typename Polygon>
	Point3Dd calc_normal(Polygon const & poly, Point3Dd & normal)
	{
		assert(poly.size() > 2);
		normal = cross(poly[1] - poly[0], poly[2] - poly[0]);

		normalize(normal);

		return normal;
	}

	/**************************************************************************************************
	 * Reverse the point list that makes the polygon.
	 *
	 * @param [in,out]	polygon	The polygon.
	 **************************************************************************************************/
	template<typename PointList>
	void reverse_polygon(PointList & polygon)
	{
		assert(!polygon.empty());
		std::reverse(polygon.begin(), polygon.end());
	}

	/**************************************************************************************************
	 * Reverse triangle by swapping two first vertices.
	 *
	 * @param [in,out]	tri	The triangle.
	 **************************************************************************************************/
	template<typename Point>
	void reverse_triangle(BaseType::Triangle<Point> & tri)
	{
		assert(tri.size()== 3);

		std::swap(tri[1], tri[2]);
	}

	/**************************************************************************************************
	 * Check whether point on lies on any edges.
	 *
	 * @param	polygon	The polygon.
	 * @param	point  	The point.
	 * @param	eps	   	(optional) the EPS.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/

	template<typename PointType, typename PointList>
	bool point_on_edges(PointList const & polygon, PointType const & point, double const & eps /*= Epsilon_Low*/)
	{
		assert(polygon.size() > 1);
		for (size_t j = 0; j < polygon.size()-1; j++)
		{
			if (BaseType::point_on_segment(polygon[j], polygon[j+1], point, eps))
				return true;
		}

		return false;
	}

	/**************************************************************************************************
	 * Calculate center point.
	 *
	 * @param	poly		  	The polygon.
	 * @param [in,out]	center	The center.
	 **************************************************************************************************/
	template<typename PointList>
	void calc_center_point(PointList const & poly, Point3Dd & center)
	{
		assert(poly.size() > 1);

		int size = poly.size();

		for (int i = 0; i < size; i++)
		{
			center.x += poly[i].x;
			center.y += poly[i].y;
			center.z += poly[i].z;
		}

		center.x /= size;
		center.y /= size;
		center.z /= size;
	}

	/**************************************************************************************************
	 * Checks whether three points are on the same line.
	 *
	 * @param	[in] p0		  	The first point.
	 * @param	[in] p1		  	The second point.
	 * @param	[in] p2		  	The thirst point.
	 * 
	 * @return	true if it yes, false if no.
	 **************************************************************************************************/
	template<typename Point>
	bool is_collinear(Point const & p0, Point const & p1, Point const & p2, float eps /*= Epsilon_Low*/)
	{
		return point_on_segment(p0, p2, p1, eps);
	}

	/**************************************************************************************************
	 * Sort a sequence in parallel manner using Microsoft's PPL library.
	 *
	 * @param	begin			 	The begin.
	 * @param	end				 	The end.
	 * @param	lessCmp			 	The sort less than predicate.
	 * @param	fallbackThreshold	(optional) the fallback threshold: Threshold below which normal
	 * 								sort is used.
	 **************************************************************************************************/

	template<class Iterator, class Compare>
	void sort_parallel(Iterator begin, Iterator end, Compare lessCmp, int fallbackThreshold /*= 10024*/)
	{
		using std::max;
		if (end - begin > fallbackThreshold)
			Concurrency::samples::parallel_buffered_sort(begin, end, lessCmp);
		else
			std::sort(begin, end, lessCmp);
	}

	/**************************************************************************************************
	 * Removes the co-linear points of polygon edges.
	 *
	 * @param [in,out]	poly	The polygon.
	 * @param	eps				(optional) the EPS.
	 *
	 * @return	true if it actually removes vertices, false if it fails.
	 **************************************************************************************************/

	template<typename PointList>
	bool remove_colinear(PointList & poly, double eps /*= BaseType::Epsilon_Low*/)
	{
		int size = poly.size();

		if (size < 3)
			return false;

		//KLOG_DEBUG() << "Number of vertices:" << poly.size();

		std::vector<bool> keeping;
		keeping.resize(poly.size(), true);

		for (int i = 0; i < size-2; i++)
		{
			if (point_on_segment(poly[i+0], poly[i+2], poly[i+1]))
			{
				keeping[i+1] = false;
				//KLOG_DEBUG() << "Vertex #" << i << "removed";
			}
		}

		unsigned count = 0;
		for (int i = 0; i < poly.size(); i++)
			if (keeping[i])
				poly[count++] = poly[i];

		poly.resize(count);

		// The case when the start point locates on an edge
		if (point_on_segment(poly[poly.size()-2], poly[1], poly[0]))
		{
			poly.erase(poly.begin());
			poly.erase(poly.begin() + poly.size()- 1); // Remove last
			poly.push_back(poly.front()); // Close polygon

			//KLOG_DEBUG() << "Vertex #" << 0 << "removed";
		}

		//KLOG_DEBUG() << "Number of vertices REMAINED:" << count;

		return true;
	}

    /**************************************************************************************************
    * Hard coding algorithm to sort three numbers for best performance.
    *
    * @param [in,out]  p   The T &amp; to process.
    * @param [in,out]  q   The T &amp; to process.
    * @param [in,out]  r   The T &amp; to process.
    * @param   c           The C to process.
    **************************************************************************************************/

    template<typename T, typename C>
    void sort3(T & p, T & q, T & r, C c)
    {
        T t;
        if (c(q , p))
        {
            t = p;
            if (c(r , q))
            {
                p = r;
                r = t;
            }
            else
            {
                if (c(r , t))
                {
                    p = q;
                    q = r;
                    r = t;
                }
                else
                {
                    p = q;
                    q = t;
                }
            }
        }
        else if (c(r , q))
        {
            t = r; r = q;
            if (c(t , p))
            {
                q = p;
                p = t;
            }
            else
                q = t;
        }
    }

}
