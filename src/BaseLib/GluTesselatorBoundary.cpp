
#include "GluTesselatorBoundary.h"
#include <stdexcept>

#ifndef CALLBACK
#define CALLBACK
#endif

#define PRINT_DEBUG
#define MAX_INTERSECTIONS 64

namespace Tesselator
{
	GLdouble vertices[MAX_INTERSECTIONS][3];               // array to store newly created vertices (x,y,z,r,g,b) by combine callback
	int vertexIndex = 0;                    // array index for above array incremented inside combine callback

	PolygonList polygons;
	Polygon3d newvertices;

// CALLBACK functions for GLU_TESS ////////////////////////////////////////////
// NOTE: must be declared with CALLBACK directive
void CALLBACK tessBeginCB(GLenum which);
void CALLBACK tessEndCB();
void CALLBACK tessErrorCB(GLenum errorCode);
void CALLBACK tessVertexCB(const GLvoid *data);
void CALLBACK tessCombineCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4],
							const GLfloat neighborWeight[4], GLdouble **outData);

// create a tessellation object and compile a quad into a display list
///////////////////////////////////////////////////////////////////////////////
bool boundTesselatePolygon(PolygonList const & inpolygons, PolygonList & boundaries, int windingRule /*= GLU_TESS_WINDING_POSITIVE*/)
{
	polygons.clear();
	boundaries.clear();
	newvertices.clear();

	GLUtesselator *tess = gluNewTess(); // create a tessellator
	
	if(!tess) 
		return false;  // failed to create tessellation object, return false

	gluTessProperty(tess, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);
	gluTessProperty(tess, GLU_TESS_WINDING_RULE, windingRule);

	gluTessCallback(tess, GLU_TESS_BEGIN,	(void (__stdcall*)(void))tessBeginCB);
	gluTessCallback(tess, GLU_TESS_END,		(void (__stdcall*)(void))tessEndCB);
	gluTessCallback(tess, GLU_TESS_ERROR,	(void (__stdcall*)(void))tessErrorCB);
	gluTessCallback(tess, GLU_TESS_VERTEX,	(void (__stdcall*)(void))tessVertexCB);
	gluTessCallback(tess, GLU_TESS_COMBINE,	(void (__stdcall*)(void))tessCombineCB);

	gluTessBeginPolygon(tess, 0);
	for (unsigned i = 0; i < inpolygons.size(); ++i)
	{
		gluTessBeginContour(tess);
		Polygon3d const & poly = inpolygons[i];
		for (unsigned j = 0; j < poly.size(); j++)
			gluTessVertex(tess, const_cast<GLdouble*>(poly[j].data()), (void *)poly[j].data());

		gluTessEndContour(tess);
	}
	gluTessEndPolygon(tess);

	gluDeleteTess(tess);        // delete after tessellation

	boundaries = polygons;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// convert enum of OpenGL primitive type to a string(char*)
// OpenGL supports only 10 primitive types.
///////////////////////////////////////////////////////////////////////////////
const char* getPrimitiveType(GLenum type)
{
	switch(type)
	{
	case 0x0000:
		return "GL_POINTS";
		break;
	case 0x0001:
		return "GL_LINES";
		break;
	case 0x0002:
		return "GL_LINE_LOOP";
		break;
	case 0x0003:
		return "GL_LINE_STRIP";
		break;
	case 0x0004:
		return "GL_TRIANGLES";
		break;
	case 0x0005:
		return "GL_TRIANGLE_STRIP";
		break;
	case 0x0006:
		return "GL_TRIANGLE_FAN";
		break;
	case 0x0007:
		return "GL_QUADS";
		break;
	case 0x0008:
		return "GL_QUAD_STRIP";
		break;
	case 0x0009:
		return "GL_POLYGON";
		break;
	}

	return "";
}

///////////////////////////////////////////////////////////////////////////////
// GLU_TESS CALLBACKS
///////////////////////////////////////////////////////////////////////////////
void CALLBACK tessBeginCB(GLenum which)
{
	//glBegin(which);

	polygons.push_back(Polygon3d());
}

void CALLBACK tessEndCB()
{
	//glEnd();
}

void CALLBACK tessVertexCB(const GLvoid *data)
{
	// cast back to double type
	const GLdouble *ptr = (const GLdouble*)data;
	Point3Dd p;
	p[0] = *ptr;
	p[1] = *(ptr+1);
	p[2] = *(ptr+2);

	polygons.back().push_back(p);
	//glVertex3dv(ptr);
}

///////////////////////////////////////////////////////////////////////////////
// Combine callback is used to create a new vertex where edges intersect.
// In this function, copy the vertex data into local array and compute the
// color of the vertex. And send it back to tessellator, so tessellator pass it
// to vertex callback function.
//
// newVertex: the intersect point which tessellator creates for us
// neighborVertex[4]: 4 neighbor vertices to cause intersection (given from 3rd param of gluTessVertex()
// neighborWeight[4]: 4 interpolation weights of 4 neighbor vertices
// outData: the vertex data to return to tessellator
///////////////////////////////////////////////////////////////////////////////
void CALLBACK tessCombineCB(const GLdouble newVertex[3], const GLdouble *neighborVertex[4],
							const GLfloat neighborWeight[4], GLdouble **outData)
{
	//Point3d p;
	//p[0] = newVertex[0];
	//p[1] = newVertex[1];
	//p[2] = newVertex[2];
	//newvertices.push_back(p);
	// copy new intersect vertex to local array
	// Because newVertex is temporal and cannot be hold by tessellator until next
	// vertex callback called, it must be copied to the safe place in the app.
	// Once gluTessEndPolygon() called, then you can safly deallocate the array.
	

	if (vertexIndex >= MAX_INTERSECTIONS)
	{
		assert(vertexIndex < MAX_INTERSECTIONS);

		throw std::invalid_argument("vertexIndex >= MAX_INTERSECTIONS");
	}

	vertices[vertexIndex][0] = newVertex[0];
	vertices[vertexIndex][1] = newVertex[1];
	vertices[vertexIndex][2] = newVertex[2];

	// return output data (vertex coords and others)
	*outData = vertices[vertexIndex];   // assign the address of new intersect vertex

	++vertexIndex;  // increase index for next vertex
	//GLdouble & first = newvertices.back()[0];
	//*outData = &first;
}

void CALLBACK tessErrorCB(GLenum errorCode)
{
	const GLubyte *errorStr;

	errorStr = gluErrorString(errorCode);
	throw std::runtime_error((char const *)errorStr);
}

}
