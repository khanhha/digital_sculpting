// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteResource.h>
#include <Graphics/GL4/GteGL4GraphicsObject.h>

namespace gte
{

class GTE_IMPEXP GL4Resource : public GL4GraphicsObject
{
protected:
    // Abstract base class.
    GL4Resource(Resource const* gtResource);

public:
    // Member access.
    inline Resource* GetResource() const;

    // TODO: TENTATIVE INTERFACE (modify as needed).  Make these pure
    // virtual latter (if relevant).
	void* MapForWrite(GLenum target, const GLintptr offset, const GLsizeiptr len);
    void  Unmap(GLenum target);
    virtual bool Update() { return false;  }
    virtual bool CopyCpuToGpu() { return false; }
    virtual bool CopyGpuToCpu() { return false; }

protected:
    // Support for copying between CPU and GPU.
    bool PreparedForCopy(GLenum access) const;
};

inline Resource* GL4Resource::GetResource() const
{
    return static_cast<Resource*>(mGTObject);
}

}
