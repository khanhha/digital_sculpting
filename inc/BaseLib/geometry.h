#pragma once

#include <assert.h>
#include <vector>

#include "util.h"

class Point3Dd;

namespace BaseType
{
	static const float Epsilon_High      = 1.0E-16f;
	static const float Epsilon_Low       = 1.0E-07f;
	static const float Epsilon           = 1.0E-07f;

    template<typename Point>
    class PointLess
    {
    public:
        PointLess(double eps = Epsilon_Low): _epsilon(eps) {}
        bool operator() (const Point & a, const Point & b) const
        {
            if (a.x == b.x)
            {
                if (a.y == b.y)
                    return a.z < b.z;
                else
                    return a.y < b.y;
            }
            else
                return a.x < b.x;
        }

        double _epsilon;
    };

	template<typename Point>
	class PointLess2
	{
	public:
		PointLess2(double eps = Epsilon_Low): _epsilon(eps) {}
		bool operator() (const Point & a, const Point & b) const
		{
			if (a.x == b.x)
				return a.y < b.y;
			else
				return a.x < b.x;
		}

		double _epsilon;
	};

	template<typename Point>
	class PointEqual2
	{
	public:
		PointEqual2(double eps = Epsilon_Low): _eps(eps) {}
		bool operator() (const Point & a, const Point & b) const
		{
			return
				is_equal(a.x, b.x, _eps) &&
				is_equal(a.y, b.y, _eps);
		}

		double _eps;
	};

    template<typename Point>
    class PointEqual
    {
    public:
        PointEqual(double eps = Epsilon_Low): _eps(eps) {}
    	bool operator() (const Point & a, const Point & b) const
    	{
    		return
                is_equal(a.x, b.x, _eps) &&
                is_equal(a.y, b.y, _eps) &&
                is_equal(a.z, b.z, _eps);
    	}
        double _eps;
    };

	template<typename Point>
	class Box
	{
	public:
		const static size_t PointCount = 2;

		Box(){}
		Box(Point const & p0, Point const & p1){_data[0] = p0; _data[1] = p1;}
		~Box(){}
		void set(Point const & p0, Point const & p1){_data[0] = p0; _data[1] = p1;}

		typedef const Point& const_reference;
		typedef       Point& reference;

	private:
		Point _data[PointCount];

	public:
		inline reference       operator [](const std::size_t& index)       { return _data[index]; }
		inline const_reference operator [](const std::size_t& index) const { return _data[index]; }
        const Point * data(void) const	{ return(_data);};
        inline std::size_t  size () const { return PointCount;   }
	};

	template<typename Point>
	class Segment
	{
	public:
		const static std::size_t PointCount = 2;

		Segment(){}
		Segment(Point const & p0, Point const & p1){_data[0] = p0; _data[1] = p1;}
		~Segment(){}
		void set(Point const & p0, Point const & p1){_data[0] = p0; _data[1] = p1;}

		typedef const Point& const_reference;
		typedef       Point& reference;

	private:
		Point _data[PointCount];

	public:
		inline reference       operator [](const std::size_t& index)       { return _data[index]; }
		inline const_reference operator [](const std::size_t& index) const { return _data[index]; }
        const Point * data(void) const	{ return(_data);};
        inline std::size_t  size () const { return PointCount;   }
	};

    template<typename Point>
    class Plane
    {
    public:
        Plane(){}
        Plane(Point const & point_, Point const & normal_)
        {
            point = point_;
            normal = normal_;
        }

        ~Plane(){}

        Point point;
        Point normal;
    };

	template<typename Point>
	class Triangle
	{
	public:
		const static std::size_t PointCount = 3;

		Triangle(){}
		Triangle(Point const & p0, Point const & p1, Point const & p2){_data[0] = p0; _data[1] = p1;_data[2] = p2;}
		~Triangle(){}
		void set(Point const & p0, Point const & p1, Point const & p2){_data[0] = p0; _data[1] = p1;_data[2] = p2;}

		typedef const Point& const_reference;
		typedef       Point& reference;
        typedef       Point value_type;

	private:
		Point _data[PointCount];

	public:
		inline reference       operator [](const std::size_t& index)       { return _data[index]; }
		inline const_reference operator [](const std::size_t& index) const { return _data[index]; }
        const Point * data(void) const	{ return(_data);};
        inline size_t  size () const { return PointCount;   }
	};

    template<typename Point>
    class Polygon: public std::vector<Point>
    {
    public:
        typedef std::vector<Point> Parent;

        Polygon(): Parent() {};
        Polygon(size_t sz): Parent(sz) {};
    };

    template<typename Point>
    class Cone
    {
    public:
        // An acute cone is Dot(A,X-V) = |X-V| cos(t) where V is the vertex, A
        // is the unit-length direction of the axis of the cone, and double is the
        // cone angle with 0 < t < pi/2.  The cone interior is defined by the
        // inequality Dot(A,X-V) >= |X-V| cos(t).  Since cos(t) > 0, we can avoid
        // computing square roots.  The solid cone is defined by the inequality
        // Dot(A,X-V)^2 >= Dot(X-V,X-V) cos(t)^2.  This is an infinite,
        // single-sided cone.
        //
        // The cone may be truncated by a Plane perpendicular to its axis at a
        // height h from the vertex (distance from the vertex to the intersection
        // of the Plane and the axis).  The infinite cone has h = infinity.  The
        // finite cone has a disk of intersection between the Plane and infinite
        // cone.  The radius r of the disk is r = h*tan(t).

        Cone ()
        {
            //_vertex = vertex;
            //_axis = axis;
            angle = 0.0;
            height = 0.0;
            trheight = 0.0;
        }

        ~Cone ()
        {
        }

        // The axis must be unit-length and the angle must be in (0,pi/2).  For
        // an infinite cone, set 'height' to Math<Real>::MAX_FLOAT.
        Cone (const Point& vertex_, Point& axis_, const double & angle_, const double & height_, const double & trheight_)
        {
            vertex = vertex_;
            axis = axis_;
            angle = angle_;
            height = height_;
            trheight = trheight_;
        }

        double topRadius() const
        {
            //assert(height > trheight);
            return (height - trheight) * tan(angle);
        }

        double baseRadius() const
        {
            return height * tan(angle);
        }
        
        Point baseCenter() const
        {
            return vertex + height * axis;
        }

        Point topCenter() const
        {
            assert(height > trheight);
            return vertex + (height - trheight) * axis;
        }

        Point vertex;
        Point axis;
        double angle;
        double height;
        double trheight; // Truncated cone height
    };

    template<typename Point>
    class Cylinder
    {
    public:
        // Construction and destruction.  The cylinder axis is a line.  The origin
        // of the cylinder is chosen to be the line origin.  The cylinder wall is
        // at a distance R units from the axis.  An infinite cylinder has infinite
        // height.  A finite cylinder has center C at the line origin and has a
        // finite height H.  The segment for the finite cylinder has endpoints
        // C-(H/2)*D and C+(H/2)*D where D is a unit-length direction of the line.
        Cylinder ()  // uninitialized
        {
            //axis = axis_;
            //center = center_;
            radius = 0.0;
            height = 0.0;
        }

        ~Cylinder () {}

        Cylinder (const Point& axis_, const Point& center_, double const & radius_, double const & height_)
        {
            axis = axis_;			
            center = center_;
            radius = radius_;
            height = height_;
        }

        Point center;
        Point axis;
        double radius;
        double height;
    };

    /**************************************************************************************************
     * Predefined geometric data type.
     *
     **************************************************************************************************/
    typedef Segment<Point3Dd> Segment3d;
    typedef Box<Point3Dd> Box3d;
    typedef Triangle<Point3Dd> Triangle3d;
    typedef Polygon<Point3Dd> Polygon3d;
    typedef PointLess<Point3Dd> PointLess3d;
	typedef PointLess2<Point2Dd> PointLess2d;
    typedef PointEqual<Point3Dd> PointEqual3d;
	typedef PointEqual2<Point2Dd> PointEqual2d;
    typedef Cone<Point3Dd> Cone3d;
    typedef Cylinder<Point3Dd> Cylinder3d;
    typedef Plane<Point3Dd> Plane3d;


    /**************************************************************************************************
     * Geometric algorithms.
     *
     **************************************************************************************************/

    template<typename T>
    inline bool is_equal(const T & val1, const T & val2, const T & epsilon = Epsilon_Low);

    //template<typename T>
    //inline bool is_equal(const T & val1, const T & val2);

    template<typename T>
    inline bool less_than_or_equal(const T & val1, const T & val2, const T & epsilon = Epsilon_Low);

    template<typename T>
    inline bool greater_than_or_equal(const T & val1, const T & val2, const T & epsilon = Epsilon_Low);

	template<typename Point>
	inline bool intersect(const Segment<Point> & segment, const Box<Point> & box);

    template<typename Point>
    bool intersect(const Triangle<Point> & tri1, const Triangle<Point> & tri2,
        bool & isCoplanar, Point & point1, Point & point2);

	template<typename Point>
    bool intersect(const Segment<Point> & segment, const Box<Point> & box);

	template<typename Point>
	int intersect(Point const & r0, Point const & r1,
		Point const & t0, Point const & t1, Point const & t2, 
		Point & i, double const & eps = Epsilon_Low);

    template<typename Point>
    bool point_on_cone(Point const & center, Point const & normal, double baseRadius, double topRadius,
        double height, Point const & px, double eps = Epsilon_Low);

    template<typename Point>
    bool point_on_cone(Cone<Point> const & cone, Point const & x, double eps = Epsilon_Low);

    template<typename Point>
    bool point_in_cone(Point const & center, Point const & normal, double baseRadius, double topRadius,
        double height, Point const & px, double eps = Epsilon_Low);

    template<typename Point>
    bool point_in_cone(Cone<Point> const & cone, Point const & x, double eps = Epsilon_Low);

	template<typename Point>
	bool point_in_cylinder(Point const & center, Point const & axis,
		double const & radius, double const & height, Point const & x, double eps);

	template<typename Point>
	bool point_in_cylinder(Cylinder<Point> const & cylinder, Point const & x, double eps);	

    template<typename Point>
    inline Plane<Point> make_plane(const double& x1, const double& y1, const double& z1,
        const double& x2, const double& y2, const double& z2,
        const double& x3, const double& y3, const double& z3);

    template<typename Point>
    inline Plane<Point> make_plane(const Point& point1,
        const Point& point2,
        const Point& point3);

    template<typename Point>
    inline Point make_plane(const Triangle<Point>& triangle);

    template<typename Point>
    bool point_in_triangle(Point const & px, Point const & p0, Point const & p1, Point const & p2);

    template<typename Point>
    bool point_in_triangle(Point const & px, Triangle<Point> const & tri);

    template<typename Point>
    bool point_in_triangle(Point const & px, Triangle<Point> const & tri);

    // a point X is between two other points A and B, if the distance AX
    // added to the distance BX is equal to the distance AB.
    template<typename Point>
    bool point_on_segment(Point const & a, Point const & b, Point const & p, double const & eps = Epsilon_Low);

    template<typename Point>
    bool point_on_segment(Segment<Point> const & segment, Point const & p, double const & eps = Epsilon_Low);

    template<typename Point>
    bool intersect(const Point& point1, const Point& point2, const Point& point3, const Point& point4, const double& eps);

    template<typename Point>
    Point triangle_normal(Point const & p0, Point const & p1, Point const & p2);

	template<typename Container, typename Less, typename Equal>
	void remove_duplicates(Container & cont, Less less, Equal equal);

    template<typename Container>
    void remove_duplicates(Container & cont);

    template<typename Point>
    void make_plane(Point const & p1, Point const & p2, Point const & p3, Plane<Point> & plane);

    template<typename Point>
    Segment3d make_segment(Point const & p1, Point const & p2);

    template<typename Point>
    bool make_cone(Point const & center, // User clicked point in the model
        Point const & normal, // Normal vector of the clicked triangle
        double baseRadius, // Base radius of the hole
        double topRadius, // Top radius of the hole
        double thickness, // Estimated thickness of the wall
        Cone<Point> & cone);
    
	template<typename Point>
	int cone_points(Cone<Point> const & cone, std::vector<Point> & points, int nvertices);

    template<typename Point>
    int cone_segments(Cone<Point> const & cone, std::vector<Segment<Point>> & segments, int nvertices);

	template<typename Point>
	int cylinder_segments(Cylinder<Point> const & cone, std::vector<Segment<Point>> & segments, int nvertices);

    template<typename Point>
    int cylinder_segments(Cylinder<Point> const & cone, std::vector<Segment<Point>> & segments, int nvertices);

    template<typename Point>
    int hasfspace_test(Plane<Point> const & plane, Point const & point, double const & eps = Epsilon_Low);
    
    template<typename Point>
    inline bool intersect(const Box<Point>& box1, const Box<Point>& box2);

    template<typename Point>
    inline bool intersect(const Box<Point>& box1, const Box<Point>& box2);

    template<typename Point, typename PointList> 
    inline void make_box(const PointList & polygon, Box<Point>& box);

	template<typename Point>
	void closest_point_on_plane_from_point(const Plane<Point>& plane,
		const Point& px, Point & po);

	template<typename T>
	inline void closest_point_on_line_from_point(const T& x1, const T& y1, const T& z1,
		const T& x2, const T& y2, const T& z2,
		const T& px, const T& py, const T& pz,
		T& nx,       T& ny,       T& nz);

	template<typename Point>
	inline void closest_point_on_line_from_point(const Point& p1, const Point& p2,
		const Point& px, Point& po);

	template<typename Polygon>
	Point3Dd calc_normal(Polygon const & poly, Point3Dd & normal);

	template<typename Point>
	Point3Dd calc_normal(Point const & p0, Point const & p1, Point const & p2, Point3Dd & normal );

	template<typename PointType, typename PointList>
	bool point_on_edges(PointList const & polygon, PointType const & point, double const & eps = Epsilon_Low);

	template<typename Point>
	void reverse_triangle(BaseType::Triangle<Point> & tri);

	template<typename PointList>
	void reverse_polygon(PointList & polygon);

	template<typename PointList>
	void calc_center_point(PointList const & poly, Point3Dd & center);

	template<typename Point>
	double dist_point_to_line( Point const & p, Point const & sp, Point const & ep);

	template<typename Point, typename Segment>
	double dist_point_to_line( Point const & p, Segment s);

	template<typename Point>
	inline bool line_plane_intersect(const Point& p1, const Point& p2,
		const Point& p3, const Point& n, Point & p);

	template<typename Point>
	bool is_collinear(Point const & p0, Point const & p1, Point const & p2, float eps = Epsilon_Low);

	template<class Iterator, class Compare>
	void sort_parallel(Iterator begin, Iterator end, Compare lessCmp, int fallbackThreshold = 10024);

	template<typename PointList>
	bool remove_colinear(PointList & poly, double eps = BaseType::Epsilon_Low);

    template<typename T, typename C>
    void sort3(T & p, T & q, T & r, C c);

} // BaseType


#include "geometry.inl"
