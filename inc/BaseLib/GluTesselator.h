/**************************************************************************************************
 * 
 * This is a wrapper for OpenGL tesselation facility.
 * See http://www.songho.ca/opengl/gl_tessellation.html for details.
 * 
 * This tool can be used to perform Boolean operations as described in http://www.glprogramming.com/red/chapter11.html.
 * 
 * The output of this class are the list of triangles that make up the "filled" area.
 * Use triangleNum() and triangle() to get them.
 * 
 **************************************************************************************************/

#pragma once

#include "BaseLib/Point3Dd.h"
#include <gl/GLU.h>

class GluTesselator
{
public:
	typedef std::vector<Point3Dd> Contour;
	typedef std::vector<Contour>  ContourVector;
	typedef std::vector<unsigned int> IndexVector;
	typedef Contour VertexVector;

public:
    GluTesselator(int windingRule_ = GLU_TESS_WINDING_ODD);
    
    GluTesselator(const ContourVector& contours, int windingRule_ = GLU_TESS_WINDING_ODD);
    
    GluTesselator(const Contour & contour, int windingRule_ = GLU_TESS_WINDING_ODD);

    virtual ~GluTesselator();

public:
	void clear();

	void addContour(const Contour& contour);
	
	void setContours(const ContourVector& contours);

	void process();

    /**************************************************************************************************
     * Get the number of generated triangles.
     *
     * \return	Number of triangle.
     **************************************************************************************************/

    int triangleNum() const 
	{
        //assert(_vertices.size() > 2 && _indices.size() > 2);
        return _indices.size() / 3;
    };

	// Get the i-th triangle
    void triangle(int i, Point3Dd & v0, Point3Dd & v1, Point3Dd & v2) const 
	{
        assert(_vertices.size() > 2 && _indices.size() > 2);

        v0 = _vertices[_indices[3*i+0]];
        v1 = _vertices[_indices[3*i+1]];
        v2 = _vertices[_indices[3*i+2]];
    };

protected:

    // Access the WindingRule
    int windingRule(void) const			{ return(_windingRule);			};
    void setWindingRule(int windingRule)	{ _windingRule = windingRule;	};

protected:
	// Rule for interior/exterior check
    int			  _windingRule;

	// List of contours
	ContourVector _contours;

public:

	// Access the Vertices
	const VertexVector& vertices(void) const	{ return(_vertices);	};

	// Access the Indices
	const IndexVector& indices(void) const		{ return(_indices);		};

public:
	VertexVector _vertices; // List of vertices

	IndexVector _indices; // List of triple set of indices of triangles into _vertices

private:

    /**
     *  BeginCB
     *  GLU_TESS_BEGIN_DATA callback
     */
    static void __stdcall BeginCB(GLenum type,
                                  GluTesselator* caller);

    /**
     *  EdgeFlagCB
     *  GLU_TESS_EDGE_FLAG_DATA callback
     */
    static void __stdcall EdgeFlagCB(GLboolean flag,
                                     GluTesselator* caller);

    /**
     *  VertexCB
     *  GLU_TESS_VERTEX_DATA callback
     */
    static void __stdcall VertexCB(unsigned int vertexIndex,
                                   GluTesselator* caller);

    /**
     *  EndCB
     *  GLU_TESS_END_DATA callback
     */
    static void __stdcall EndCB(GluTesselator* caller);

    /**
     *  CombineCB
     *  GLU_TESS_COMBINE_DATA callback
     */
    static void __stdcall CombineCB(GLdouble coords[3],
                                    unsigned int vertexData[4],
                                    GLfloat weight[4],
                                    unsigned int* outData,
                                    GluTesselator* caller);

    /**
     *  ErrorCB
     *  GLU_TESS_ERROR_DATA callback
     */
    static void __stdcall ErrorCB(GLenum errno,
                                  GluTesselator* caller);
};

typedef void (__stdcall *GluTessCallbackType)();
