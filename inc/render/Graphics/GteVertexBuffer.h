// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteBuffer.h>
#include <Graphics/GteVertexFormat.h>

namespace gte
{

class GTE_IMPEXP VertexBuffer : public Buffer
{
public:
    // Construction.  The first constructor is for typical usage whereby you
    // specified the attributes of a vertex format.  The second constructor
    // is used for SV_VertexID-based drawing where the vertices are read from
    // a non-vertex-buffer resource in the vertex shader.
    VertexBuffer(VertexFormat const& vformat, unsigned int numVertices,
        bool createStorage = true);
    VertexBuffer(unsigned int numVertices);
	~VertexBuffer();


    // Member access.
    VertexFormat const& GetFormat() const;
    bool IsFormatted() const;

    // Get pointers to attribute data if it exists for the specified semantic
    // and unit.  Also, you request that the attribute be one of a list of
    // required types (OR-ed bit flags).  If you do not care about the type,
    // pass DF_UNKNOWN for the required input.  If the request fails, a null
    // pointer is returned.
    char* GetChannel(VASemantic semantic, unsigned int unit,
        std::set<DFType> const& requiredTypes);

protected:
    VertexFormat mVFormat;

	int debug;
};

}
