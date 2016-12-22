// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4InputLayoutManager.h>
using namespace gte;

GL4InputLayoutManager::~GL4InputLayoutManager()
{
    if (mMap.HasElements())
    {
        qWarning("Input layout map is not empty on destruction.");
        UnbindAll();
    }
}

GL4InputLayoutManager::GL4InputLayoutManager()
{
}

GL4InputLayout* GL4InputLayoutManager::Bind(GLuint programHandle,
    GLuint vbufferHandle, VertexBuffer const* vbuffer)
{
    GL4InputLayout* layout = nullptr;
    if (programHandle)
    {
        if (vbuffer)
        {
            if (!mMap.Get(std::make_pair(vbuffer, programHandle), layout))
            {
                layout = new GL4InputLayout(programHandle, vbufferHandle,
                    vbuffer);
                mMap.Insert(std::make_pair(vbuffer, programHandle), layout);
            }
        }
        // else: A null vertex buffer is passed when an effect wants to
        // bypass the input assembler.
    }
    else
    {
        qCritical("Program must exist.");
    }
    return layout;
}

bool GL4InputLayoutManager::Unbind(VertexBuffer const* vbuffer)
{
    if (vbuffer)
    {
        std::vector<VBPPair> matches;
        mMap.GatherMatch(vbuffer, matches);
        for (auto match : matches)
        {
            GL4InputLayout* layout = nullptr;
            if (mMap.Remove(match, layout))
            {
                delete layout;
            }
        }
        return true;
    }
    else
    {
		qCritical("Vertex buffer must be nonnull.");
        return false;
    }
}

bool GL4InputLayoutManager::Unbind(Shader const*)
{
    return true;
}

void GL4InputLayoutManager::UnbindAll()
{
    std::vector<GL4InputLayout*> layouts;
    mMap.GatherAll(layouts);
    for (auto layout : layouts)
    {
        delete layout;
    }
    mMap.RemoveAll();
}

bool GL4InputLayoutManager::HasElements() const
{
    return mMap.HasElements();
}

GL4InputLayoutManager::LayoutMap::~LayoutMap()
{
}

GL4InputLayoutManager::LayoutMap::LayoutMap()
{
}

void GL4InputLayoutManager::LayoutMap::GatherMatch(
    VertexBuffer const* vbuffer, std::vector<VBPPair>& matches)
{
    mMutex.lock();
    {
        for (auto vbp : mMap)
        {
            if (vbuffer == vbp.first.first)
            {
                matches.push_back(vbp.first);
            }
        }
    }
    mMutex.unlock();
}
