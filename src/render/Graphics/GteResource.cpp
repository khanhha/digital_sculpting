// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

//#include <GTEnginePCH.h>
//#include <LowLevel/GteLogger.h>
#include <Graphics/GteResource.h>
using namespace gte;


Resource::~Resource()
{
    DestroyStorage();
}

Resource::Resource(unsigned int numElements, size_t elementSize,
    bool createStorage)
    :
    mNumElements(numElements),
    mElementSize(static_cast<unsigned int>(elementSize)),
    mUsage(IMMUTABLE),
    mCopyType(COPY_NONE),
    mOffset(0),
    mData(nullptr)
{

    mType = GT_RESOURCE;

    if (mNumElements > 0 && mElementSize > 0)
    {
        mNumBytes = mNumElements*mElementSize;
        mNumActiveElements = mNumElements;
        if (createStorage)
        {
            CreateStorage();
        }
    }
    else
    {
        // No assertion may occur here.  The VertexBuffer constructor with
        // a VertexFormat of zero attributes (used for SV_VertexID-based
        // drawing) and the IndexBuffer constructor for which no indices are
        // provided will lead to this path.
        mNumBytes = 0;
        mElementSize = 0;
        mNumActiveElements = 0;
    }
}

void Resource::ResetStorage(unsigned int numElements)
{
	mNumElements = numElements;

	if (mNumElements > 0 && mElementSize > 0)
	{
		mNumBytes = mNumElements*mElementSize;
		mNumActiveElements = mNumElements;
		mStorage.resize(mNumBytes);
		mData = &mStorage[0];
	}
	else
	{
		// No assertion may occur here.  The VertexBuffer constructor with
		// a VertexFormat of zero attributes (used for SV_VertexID-based
		// drawing) and the IndexBuffer constructor for which no indices are
		// provided will lead to this path.
		mNumBytes = 0;
		mElementSize = 0;
		mNumActiveElements = 0;
	}
}

void Resource::CreateStorage()
{
    if (mStorage.size() == 0)
    {
        mStorage.resize(mNumBytes);
        if (!mData)
        {
            mData = &mStorage[0];
        }
    }
}

void Resource::DestroyStorage()
{
    if (mStorage.size() > 0 && mData == &mStorage[0])
    {
        mData = nullptr;
        mStorage.clear();
		mStorage.shrink_to_fit();
    }
}

void Resource::SetOffset(unsigned int offset)
{
    if (offset < mNumElements)
    {
        mOffset = offset;
    }
    else
    {
        //LogWarning("Invalid offset.");
        mOffset = 0;
    }
}

void Resource::SetNumActiveElements(unsigned int numActiveElements)
{
    if (numActiveElements + mOffset <= mNumElements)
    {
        mNumActiveElements = numActiveElements;
    }
    else
    {
        //LogWarning("Invalid number of active elements.");
        mNumActiveElements =
            static_cast<unsigned int>(mNumElements - mOffset);
    }
}

