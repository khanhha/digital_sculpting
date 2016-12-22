// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteIndexBuffer.h>
#include <Graphics/GteVertexBuffer.h>

namespace gte
{
// This class is a factory for Visual objects corresponding to common
// geometric primitives.  Triangle mesh primitives are generated.  Each mesh
// is centered at (0,0,0) and has an up-axis of (0,0,1).  The other axes
// forming the coordinate system are (1,0,0) and (0,1,0).
//
// The factory always generates 3-tuple positions.  If normals, tangents, or
// binormals are requested, they are also generated as 3-tuples.  They are
// stored in the vertex buffer as 3-tuples or 4-tuples as requested (w = 1 for
// positions, w = 0 for the others).  The factory also generates 2-tuple
// texture coordinates.  These are stored in the vertex buffer for 2-tuple
// units.  All other attribute types are unassigned by the factory.

class GTE_IMPEXP MeshFactory
{
public:
	typedef std::pair<std::shared_ptr<VertexBuffer>, std::shared_ptr<IndexBuffer>> MeshBuffer;
public:
    // Construction and destruction.
    ~MeshFactory();
    MeshFactory();

    // Specify the vertex format.
    inline void SetVertexFormat(VertexFormat const& format);

    // Specify the usage for the vertex buffer data.  The default is
    // Resource::IMMUTABLE.
    inline void SetVertexBufferUsage(Resource::Usage usage);

    // Specify the type of indices and where the index buffer data should be
    // stored.  For 'unsigned int' indices, set 'use32Bit' to 'true'; for
    // 'unsigned short' indices, set 'use32Bit' to false.  The default is
    // 'unsigned int'.
    inline void SetIndexFormat(bool use32Bit);
    
    // Specify the usage for the index buffer data.  The default is
    // Resource::IMMUTABLE.
    inline void SetIndexBufferUsage(Resource::Usage usage);

    // For the geometric primitives that have an inside and an outside, you
    // may specify where the observer is expected to see the object.  If the
    // observer must see the primitive from the outside, pass 'true' to this
    // function.  If the observer must see the primitive from the inside, pass
    // 'false'.  This Boolean flag simply controls the triangle face order
    // for face culling.  The default is 'true' (observer view object from the
    // outside).
    inline void SetOutside(bool outside);

    // The sphere has center (0,0,0) and the specified radius.  The north pole
    // is at (0,0,radius) and the south pole is at (0,0,-radius).  The mesh has
    // the topology of an open cylinder (which is also the topology of a
    // rectangle with wrap-around for one pair of parallel edges) and is then
    // stitched to the north and south poles.  The triangles are unevenly
    // distributed.  If you want a more even distribution, create an
    // icosahedron and subdivide it.
    MeshBuffer CreateSphere(unsigned int numZSamples,
        unsigned int numRadialSamples, float radius);

    // The torus has center (0,0,0).  If you observe the torus along the
    // line with direction (0,0,1), you will see an annulus.  The circle
    // that is the center of the annulus has radius 'outerRadius'.  The
    // distance from this circle to the boundaries of the annulus is the
    // 'inner radius'.
	MeshBuffer CreateTorus(unsigned int numCircleSamples,
        unsigned int numRadialSamples, float outerRadius, float innerRadius);


	// The box has center (0,0,0); unit-length axes (1,0,0), (0,1,0), and
	// (0,0,1); and extents (half-lengths) xExtent, yExtent, and zExtent.  The
	// mesh has 8 vertices and 12 triangles.  For example, the box corner in
	// the first octant is (xExtent, yExtent, zExtent).
	MeshBuffer CreateBox(float xExtent, float yExtent, float zExtent);
	MeshBuffer CreateLine(gte::EVector3 &p1, gte::EVector3 &p2);
	MeshBuffer CreateCircle(const gte::EVector3 &normal, float radius, unsigned int numCircleSamples);
private:
    // Support for creating vertex and index buffers.
    std::shared_ptr<VertexBuffer> CreateVBuffer(unsigned int numVertices);
    std::shared_ptr<IndexBuffer> CreateIBuffer(unsigned int numTriangles);

    // Support for vertex buffers.
    char* GetGeometricChannel(std::shared_ptr<VertexBuffer> const& vbuffer,
        VASemantic semantic, float w);
    inline EVector3& Position(unsigned int i);
    inline EVector3& Normal(unsigned int i);
    inline EVector3& Tangent(unsigned int i);
    inline EVector3& Binormal(unsigned int i);
    inline EVector2& TCoord(unsigned int unit, unsigned int i);
    void SetPosition(unsigned int i, EVector3 const& pos);
    void SetNormal(unsigned int i, EVector3 const& nor);
    void SetTangent(unsigned int i, EVector3 const& tan);
    void SetBinormal(unsigned int i, EVector3 const& bin);
    void SetTCoord(unsigned int i, EVector2 const& tcd);
    void SetPlatonicTCoord(unsigned int i, EVector3 const& pos);

    // Support for index buffers.
    void ReverseTriangleOrder(IndexBuffer* ibuffer);

	float	ComputeOrthogonalComplement(int numInputs, EVector3* v);
    VertexFormat mVFormat;
    size_t mIndexSize;
    Resource::Usage mVBUsage, mIBUsage;
    bool mOutside;
    bool mAssignTCoords[VA_MAX_TCOORD_UNITS];

    char* mPositions;
    char* mNormals;
    char* mTangents;
    char* mBinormals;
    char* mTCoords[VA_MAX_TCOORD_UNITS];
};


inline void MeshFactory::SetVertexFormat(VertexFormat const& format)
{
    mVFormat = format;
}

inline void MeshFactory::SetVertexBufferUsage(Resource::Usage usage)
{
    mVBUsage = usage;
}

inline void MeshFactory::SetIndexFormat(bool use32Bit)
{
    mIndexSize = (use32Bit ? sizeof(unsigned int) : sizeof(unsigned short));
}

inline void MeshFactory::SetIndexBufferUsage(Resource::Usage usage)
{
    mIBUsage = usage;
}

inline void MeshFactory::SetOutside(bool outside)
{
    mOutside = outside;
}

inline EVector3& MeshFactory::Position(unsigned int i)
{
    return *reinterpret_cast<EVector3*>(
        mPositions + i*mVFormat.GetVertexSize());
}

inline EVector3& MeshFactory::Normal(unsigned int i)
{
    return *reinterpret_cast<EVector3*>(
        mNormals + i*mVFormat.GetVertexSize());
}

inline EVector3& MeshFactory::Tangent(unsigned int i)
{
    return *reinterpret_cast<EVector3*>(
        mTangents + i*mVFormat.GetVertexSize());
}

inline EVector3& MeshFactory::Binormal(unsigned int i)
{
    return *reinterpret_cast<EVector3*>(
        mBinormals + i*mVFormat.GetVertexSize());
}

inline EVector2& MeshFactory::TCoord(unsigned int unit, unsigned int i)
{
    return *reinterpret_cast<EVector2*>(
        mTCoords[unit] + i*mVFormat.GetVertexSize());
}


}
