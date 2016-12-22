// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <Mathematics/GteBitHacks.h>
#include <Graphics/GteIndexBuffer.h>
#include <QtGlobal>
using namespace gte;


IndexBuffer::IndexBuffer(IPType type, unsigned int numPrimitives,
    size_t indexSize, bool createStorage)
    :
    Buffer(msIndexCounter[Log2OfPowerOfTwo(type)](numPrimitives),
        indexSize, createStorage),
    mPrimitiveType(type),
    mNumPrimitives(numPrimitives),
    mNumActivePrimitives(numPrimitives),
    mFirstPrimitive(0)
{
    mType = GT_INDEX_BUFFER;
    //LogAssert(mNumPrimitives > 0, "Invalid number of primitives.");
	if (numPrimitives < 0) qErrnoWarning("Invalid number of primitives.");
}

IndexBuffer::IndexBuffer(IPType type, unsigned int numPrimitives)
    :
    Buffer(msIndexCounter[Log2OfPowerOfTwo(type)](numPrimitives), 0, false),
    mPrimitiveType(type),
    mNumPrimitives(numPrimitives),
    mNumActivePrimitives(numPrimitives),
    mFirstPrimitive(0)
{
    mType = GT_INDEX_BUFFER;
    //qLogg(mNumPrimitives > 0, "Invalid number of primitives.");
	if(numPrimitives <0 ) qErrnoWarning("Invalid number of primitives.");
}

void IndexBuffer::SetNumActivePrimitives(unsigned int numActive)
{
    if (numActive <= mNumPrimitives)
    {
        mNumActivePrimitives = numActive;
    }
    else
    {
        //LogWarning("Invalid number of active primitives.");
        mNumActivePrimitives = mNumPrimitives;
    }
}

unsigned int IndexBuffer::GetNumActiveIndices() const
{
    unsigned int i = Log2OfPowerOfTwo(mPrimitiveType);
    return msIndexCounter[i](mNumActivePrimitives);
}

void IndexBuffer::SetFirstPrimitive(unsigned int first)
{
    if (0 <= first && first < mNumPrimitives
    &&  first + mNumActivePrimitives <= mNumPrimitives)
    {
        mFirstPrimitive = first;
        return;
    }

   //LogError("Invalid first primitive.");
	qErrnoWarning("Invalid first primitive.");
}

unsigned int IndexBuffer::GetFirstIndex() const
{
    if (mFirstPrimitive == 0)
    {
        return 0;
    }

    int i = Log2OfPowerOfTwo(mPrimitiveType);
    return msIndexCounter[i](mFirstPrimitive);
}

bool IndexBuffer::SetPoint(unsigned int i, unsigned int v)
{
    if (ValidPrimitiveType(IP_HAS_POINTS))
    {
        if (mData && i < mNumPrimitives)
        {
            if (mElementSize == sizeof(unsigned int))
            {
                unsigned int* index = i + Get<unsigned int>();
                *index = v;
            }
            else
            {
                unsigned short* index = i + Get<unsigned short>();
                *index = static_cast<unsigned short>(v);
            }
            return true;
        }
    }
    return false;
}

bool IndexBuffer::GetPoint(unsigned int i, unsigned int& v) const
{
    if (ValidPrimitiveType(IP_HAS_POINTS))
    {
        if (mData && i < mNumPrimitives)
        {
            if (mElementSize == sizeof(unsigned int))
            {
                unsigned int const* index = i + Get<unsigned int>();
                v = *index;
            }
            else
            {
                unsigned short const* index = i + Get<unsigned short>();
                v = static_cast<unsigned int>(*index);
            }
            return true;
        }
    }
    return false;
}

bool IndexBuffer::SetSegment(unsigned int i, unsigned int v0, unsigned int v1)
{
    if (ValidPrimitiveType(IP_HAS_SEGMENTS))
    {
        if (mData && i < mNumPrimitives)
        {
            if (mElementSize == sizeof(unsigned int))
            {
                if (mPrimitiveType == IP_POLYSEGMENT_DISJOINT)
                {
                    unsigned int* index = 2 * i + Get<unsigned int>();
                    *index++ = v0;
                    *index = v1;
                }
                else
                {
                    unsigned int* index = i + Get<unsigned int>();
                    *index++ = v0;
                    *index = v1;
                }
            }
            else
            {
                if (mPrimitiveType == IP_POLYSEGMENT_DISJOINT)
                {
                    unsigned short* index = 2 * i + Get<unsigned short>();
                    *index++ = static_cast<unsigned short>(v0);
                    *index = static_cast<unsigned short>(v1);
                }
                else
                {
                    unsigned short* index = i + Get<unsigned short>();
                    *index++ = static_cast<unsigned short>(v0);
                    *index = static_cast<unsigned short>(v1);
                }
            }
            return true;
        }
    }
    return false;
}

bool IndexBuffer::GetSegment(unsigned int i, unsigned int& v0,
    unsigned int& v1) const
{
    if (ValidPrimitiveType(IP_HAS_SEGMENTS))
    {
        if (mData && i < mNumPrimitives)
        {
            if (mElementSize == sizeof(unsigned int))
            {
                if (mPrimitiveType == IP_POLYSEGMENT_DISJOINT)
                {
                    unsigned int const* index = 2 * i + Get<unsigned int>();
                    v0 = *index++;
                    v1 = *index;
                }
                else
                {
                    unsigned int const* index = i + Get<unsigned int>();
                    v0 = *index++;
                    v1 = *index;
                }
            }
            else
            {
                if (mPrimitiveType == IP_POLYSEGMENT_DISJOINT)
                {
                    unsigned short const* index =
                        2 * i + Get<unsigned short>();
                    v0 = static_cast<unsigned int>(*index++);
                    v1 = static_cast<unsigned int>(*index);
                }
                else
                {
                    unsigned short const* index = i + Get<unsigned short>();
                    v0 = static_cast<unsigned int>(*index++);
                    v1 = static_cast<unsigned int>(*index);
                }
            }
            return true;
        }
    }
    return false;
}

bool IndexBuffer::SetTriangle(unsigned int i, unsigned int v0,
    unsigned int v1, unsigned int v2)
{
    if (ValidPrimitiveType(IP_HAS_TRIANGLES))
    {
        if (mData && i < mNumPrimitives)
        {
            if (mElementSize == sizeof(unsigned int))
            {
                if (mPrimitiveType == IP_TRIMESH)
                {
                    unsigned int* index = 3 * i + Get<unsigned int>();
                    *index++ = v0;
                    *index++ = v1;
                    *index = v2;
                }
                else
                {
                    unsigned int* index = i + Get<unsigned int>();
                    index[0] = v0;
                    if (i & 1)
                    {
                        index[2] = v1;
                        index[1] = v2;
                    }
                    else
                    {
                        index[1] = v1;
                        index[2] = v2;
                    }
                }
            }
            else
            {
                if (mPrimitiveType == IP_TRIMESH)
                {
                    unsigned short* index = 3 * i + Get<unsigned short>();
                    *index++ = static_cast<unsigned short>(v0);
                    *index++ = static_cast<unsigned short>(v1);
                    *index = static_cast<unsigned short>(v2);
                }
                else
                {
                    unsigned short* index = i + Get<unsigned short>();
                    index[0] = static_cast<unsigned short>(v0);
                    if (i & 1)
                    {
                        index[2] = static_cast<unsigned short>(v1);
                        index[1] = static_cast<unsigned short>(v2);
                    }
                    else
                    {
                        index[1] = static_cast<unsigned short>(v1);
                        index[2] = static_cast<unsigned short>(v2);
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool IndexBuffer::GetTriangle(unsigned int i, unsigned int& v0,
    unsigned int& v1, unsigned int& v2) const
{
    if (ValidPrimitiveType(IP_HAS_TRIANGLES))
    {
        if (mData && i < mNumPrimitives)
        {
            if (mElementSize == sizeof(unsigned int))
            {
                if (mPrimitiveType == IP_TRIMESH)
                {
                    unsigned int const* index = 3 * i + Get<unsigned int>();
                    v0 = *index++;
                    v1 = *index++;
                    v2 = *index;
                }
                else
                {
                    unsigned int const* index = i + Get<unsigned int>();
                    unsigned int offset = (i & 1);
                    v0 = index[0];
                    v1 = index[1 + offset];
                    v2 = index[2 - offset];
                }
            }
            else
            {
                if (mPrimitiveType == IP_TRIMESH)
                {
                    unsigned short const* index =
                        3 * i + Get<unsigned short>();
                    v0 = static_cast<unsigned int>(*index++);
                    v1 = static_cast<unsigned int>(*index++);
                    v2 = static_cast<unsigned int>(*index);
                }
                else
                {
                    unsigned short const* index = i + Get<unsigned short>();
                    int offset = (i & 1);
                    v0 = static_cast<unsigned int>(index[0]);
                    v1 = static_cast<unsigned int>(index[1 + offset]);
                    v2 = static_cast<unsigned int>(index[2 - offset]);
                }
            }
            return true;
        }
    }
    return false;
}

unsigned int IndexBuffer::GetPolypointIndexCount(unsigned int numPrimitives)
{
    // Create one point when numPrimitives is invalid.
    return numPrimitives > 0 ? numPrimitives : 1;
}

unsigned int IndexBuffer::GetPolysegmentDisjointIndexCount(
    unsigned int numPrimitives)
{
    // Create one segment when numPrimitives is invalid.
    return numPrimitives > 0 ? 2*numPrimitives : 2;
}

unsigned int IndexBuffer::GetPolysegmentContiguousIndexCount(
    unsigned int numPrimitives)
{
    // Create one segment when numPrimitives is invalid.
    return numPrimitives > 0 ? numPrimitives + 1 : 2;
}

unsigned int IndexBuffer::GetTrimeshIndexCount(unsigned int numPrimitives)
{
    // Create one triangle when numPrimitives is invalid.
    return numPrimitives > 0 ? 3*numPrimitives : 3;
}

unsigned int IndexBuffer::GetTristripIndexCount(unsigned int numPrimitives)
{
    // Create one triangle when numPrimitives is invalid.
    return numPrimitives > 0 ? numPrimitives + 2 : 3;
}


IndexBuffer::ICFunction IndexBuffer::msIndexCounter[IP_NUM_TYPES] =
{
    &IndexBuffer::GetPolypointIndexCount,
    &IndexBuffer::GetPolysegmentDisjointIndexCount,
    &IndexBuffer::GetPolysegmentContiguousIndexCount,
    &IndexBuffer::GetTrimeshIndexCount,
    &IndexBuffer::GetTristripIndexCount
};
