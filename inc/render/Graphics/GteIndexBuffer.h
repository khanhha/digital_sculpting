// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteBuffer.h>
#include <Graphics/GteIndexFormat.h>

// Test for mismatches of primitive type in {Set,Get}Point, {Set,Get}Segment,
// and {Set,Get}Triangle.
//#define GTE_VERIFY_PRIMITIVE_TYPE

namespace gte
{

class GTE_IMPEXP IndexBuffer : public Buffer
{
public:
    // Construction.  The first constructor leads to a DrawIndexed call in the
    // graphics engine.
    IndexBuffer(IPType type, unsigned int numPrimitives,
        size_t indexSize, bool createStorage = true);

    // Use this constructor when you want the indexing to be implied by the
    // ordering of vertices in the vertex buffer.  This leads to a Draw call
    // in the graphics engine.  You must ensure that numPrimitives and
    // numVertices (in the VertexBuffer) are consistent.  The usage flag is
    // not applicable, because there is no system-memory resource data.
    IndexBuffer(IPType type, unsigned int numPrimitives);

    // Member access.
    inline IPType GetPrimitiveType() const;
    inline unsigned int GetNumPrimitives() const;
    inline bool IsIndexed() const;

    // Specify how many primitives are to be drawn (if you do not want them
    // all drawn).  The default value is mNumPrimitives.  The function
    // SetNumActivePrimitives ensures that the input satisfies
    // numActive <= mNumPrimitives.
    void SetNumActivePrimitives(unsigned int numActive);
    inline unsigned int GetNumActivePrimitives() const;
    unsigned int GetNumActiveIndices() const;

    // Specify the index of the first primitive to be drawn.  The default
    // value is zero.  If you plan to modify both the number of active
    // primitives and the first primitive to be drawn, set the number of
    // of active primitives first.  SetFirstPrimitive ensures that
    // first < mNumPrimitives and first + numActive <= mNumPrimitives.
    void SetFirstPrimitive(unsigned int first);
    inline unsigned int GetFirstPrimitive() const;
    unsigned int GetFirstIndex() const;

    // Support for set/get of primitive indices.  The functions return 'true'
    // iff the index i is within range for the primitive.  The caller is
    // responsible for using the correct functions for the primitive type.
    // If you want to trap mismatches, enable GTE_VERIFY_PRIMITIVE_TYPE.
    // These functions have per-primitive overhead, namely, various range
    // checks and typecasting, so consider these a convenience.  For optimum
    // speed, you can use
    //   IndexBuffer* ibuffer = new IndexBuffer(...);
    //   type* indices = (type*)ibuffer->GetRawData();
    //   <set or get indices[i]>;
    // where 'type' is 'int' or 'short' depending on how you constructed the
    // index buffer.
    bool SetPoint(unsigned int i, unsigned int v);
    bool GetPoint(unsigned int i, unsigned int& v) const;
    bool SetSegment(unsigned int i, unsigned int v0, unsigned int v1);
    bool GetSegment(unsigned int i, unsigned int& v0, unsigned int& v1) const;
    bool SetTriangle(unsigned int i, unsigned int v0, unsigned int v1,
        unsigned int v2);
    bool GetTriangle(unsigned int i, unsigned int& v0, unsigned int& v1,
        unsigned int& v2) const;

protected:
    IPType mPrimitiveType;
    unsigned int mNumPrimitives;
    unsigned int mNumActivePrimitives;
    unsigned int mFirstPrimitive;

    inline bool ValidPrimitiveType(IPType type) const;

    typedef unsigned int(*ICFunction)(unsigned int);
    static ICFunction msIndexCounter[IP_NUM_TYPES];

    static unsigned int GetPolypointIndexCount(
        unsigned int numPrimitives);

    static unsigned int GetPolysegmentDisjointIndexCount(
        unsigned int numPrimitives);

    static unsigned int GetPolysegmentContiguousIndexCount(
        unsigned int numPrimitives);

    static unsigned int GetTrimeshIndexCount(
        unsigned int numPrimitives);

    static unsigned int GetTristripIndexCount(
        unsigned int numPrimitives);
};


inline IPType IndexBuffer::GetPrimitiveType() const
{
    return mPrimitiveType;
}

inline unsigned int IndexBuffer::GetNumPrimitives() const
{
    return mNumPrimitives;
}

inline bool IndexBuffer::IsIndexed() const
{
    return mData != nullptr;
}

inline unsigned int IndexBuffer::GetNumActivePrimitives() const
{
    return mNumActivePrimitives;
}

inline unsigned int IndexBuffer::GetFirstPrimitive() const
{
    return mFirstPrimitive;
}

inline bool IndexBuffer::ValidPrimitiveType(IPType type) const
{
    if ((mPrimitiveType & type) != 0)
    {
        return true;
    }
    else
    {
#if defined(GTE_VERIFY_PRIMITIVE_TYPE)
        LogError("Invalid primitive type used in Set<Primitive> call.");
#endif
        return false;
    }
}


}
