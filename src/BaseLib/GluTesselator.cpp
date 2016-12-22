#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "GluTesselator.h"

/**************************************************************************************************
 * Default constructor.
 **************************************************************************************************/

GluTesselator::GluTesselator(int windingRule): _windingRule(windingRule)
{
}

/**************************************************************************************************
 * Constructor from a single contour.
 *
 * \param	contour	   	The contour.
 * \param	windingRule	The winding rule.
 **************************************************************************************************/

GluTesselator::GluTesselator(const Contour & contour, int windingRule): _windingRule(windingRule)
{
	_contours.push_back(contour);
}

/**************************************************************************************************
 * Constructor from a list of single contours.
 *
 * \param	contours   	The contours.
 * \param	windingRule	The winding rule.
 **************************************************************************************************/

GluTesselator::GluTesselator(const ContourVector& contours, int windingRule): _windingRule(windingRule)
{
	_contours = contours;
}

void GluTesselator::process()
{
	GLUtesselator* tess = 0;

	try
	{
		// create tesselation object
		tess = gluNewTess();
		if (tess == NULL)
		{
			throw std::runtime_error("Creation of tesselation object failed.");
		}

		gluTessProperty(tess, GLU_TESS_WINDING_RULE, _windingRule);

		// register tesselation callbacks
		gluTessCallback(tess,
			GLU_TESS_BEGIN_DATA,
			reinterpret_cast<GluTessCallbackType>(BeginCB)
			);
		gluTessCallback(tess,
			GLU_TESS_EDGE_FLAG_DATA,
			reinterpret_cast<GluTessCallbackType>(EdgeFlagCB)
			);
		gluTessCallback(tess,
			GLU_TESS_VERTEX_DATA,
			reinterpret_cast<GluTessCallbackType>(VertexCB)
			);
		gluTessCallback(tess,
			GLU_TESS_END_DATA,
			reinterpret_cast<GluTessCallbackType>(EndCB)
			);
		gluTessCallback(tess,
			GLU_TESS_COMBINE_DATA,
			reinterpret_cast<GluTessCallbackType>(CombineCB)
			);
		gluTessCallback(tess,
			GLU_TESS_ERROR_DATA,
			reinterpret_cast<GluTessCallbackType>(ErrorCB)
			);

		// begin polygon
		gluTessBeginPolygon(tess, reinterpret_cast<void*>(this));

		// copy the points in the path polygon to the vertex vector

		// go through the contours
		for (unsigned i = 0; i < _contours.size(); i++)
		{
			// store reference to contour
			const Contour& c = _contours[i];

			// go through the points in the contour
			for (unsigned p = 0; p < c.size(); p++)
			{
				// Add this point of the contour to the list of vertices.
				// Flip the vertices because the _contours has a different
				// coordinate space than a normal Direct3D 8 application.

				_vertices.push_back(c[p]);
			}
		}

		unsigned int vertex_num = 0;

		// go through the contours
		unsigned i;
		for (i = 0; i < _contours.size(); i++)
		{
			gluTessBeginContour(tess);

			// store reference to contour
			const Contour& c = _contours[i];

			// go through the points in the contour
			for (unsigned p = 0; p < c.size(); p++)
			{
				// pass the corresponding vertex to the tesselator object
				gluTessVertex(tess,
					reinterpret_cast<double*>(&_vertices[vertex_num]),
					reinterpret_cast<void*>(vertex_num));

				vertex_num++;
			}

			gluTessEndContour(tess);
		}

		// end polygon
		gluTessEndPolygon(tess);

		// destroy the tesselation object
		gluDeleteTess(tess);
		tess = 0;
	}
	catch (...)
	{
		// destroy the tesselation object
		if (tess)
		{
			gluDeleteTess(tess);
		}

		//MainWindow::getInstance()->updateProgressDialog(CommonBase::CLOSE);
		// re-throw exception
		throw;
	}
}

GluTesselator::~GluTesselator()
{
}

void __stdcall GluTesselator::BeginCB(GLenum type,
	GluTesselator* caller)
{
	if (type != GL_TRIANGLES)
	{
	}
}

void __stdcall GluTesselator::EdgeFlagCB(GLboolean flag,
	GluTesselator* caller)
{
}

void __stdcall GluTesselator::VertexCB(unsigned int vertexIndex,
	GluTesselator* caller)
{
	caller->_indices.push_back(vertexIndex);
}

void __stdcall GluTesselator::EndCB(GluTesselator* caller)
{
}

void __stdcall GluTesselator::CombineCB(GLdouble coords[3],
	unsigned int vertexData[4],
	GLfloat weight[4],
	unsigned int* outData,
	GluTesselator* caller)
{
	// create a new vertex with the given coordinates
	// add the vertex to the calling object's vertex vector
	caller->_vertices.push_back(Point3Dd(coords));

	// pass back the index of the new vertex; it will be passed
	// as the vertexIndex parameter to VertexCB in turn
	*outData = caller->_vertices.size() - 1;
}

void __stdcall GluTesselator::ErrorCB(GLenum errno, GluTesselator* caller)
{
}

/**************************************************************************************************
 * Adds a contour.
 *
 * \param	contours	The contours.
 **************************************************************************************************/

void GluTesselator::addContour(const Contour&contour)
{
	_contours.push_back(contour);
}

/**************************************************************************************************
 * Clears this object to its blank/initial state.
 **************************************************************************************************/

void GluTesselator::clear(void)
{
	_contours.clear();
	_vertices.clear();
	_indices.clear();
}

/**************************************************************************************************
 * Sets the contours.
 *
 * \param	contours	The contours.
 **************************************************************************************************/

void GluTesselator::setContours(const ContourVector&contours)
{
	clear();
	_contours = contours;
}
