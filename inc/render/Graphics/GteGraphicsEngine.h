// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <LowLevel/GteThreadSafeMap.h>
#include <Graphics/GteBlendState.h>
#include <Graphics/GteDepthStencilState.h>
#include <Graphics/GteGEDrawTarget.h>
#include <Graphics/GteGEInputLayoutManager.h>
#include <Graphics/GteGEObject.h>
#include <Graphics/GteOverlayEffect.h>
#include <Graphics/GteRasterizerState.h>
#include <Graphics/GteFontArialW400H18.h>

// Allow names to be assigned to graphics API-specific objects for debugging.
#define GTE_GRAPHICS_USE_NAMED_OBJECTS

namespace gte
{

class GTE_IMPEXP GraphicsEngine
{
public:
    // Abstract base class.
    virtual ~GraphicsEngine();

    // Viewport management.  The measurements are in window coordinates.  The
    // origin of the window is (x,y), the window width is w, and the window
    // height is h.  The depth range for the view volume is [zmin,zmax].  The
    // DX11 viewport is left-handed with origin the upper-left corner of the
    // window, the x-axis is directed rightward, the y-axis is directed
    // downward, and the depth range is a subset of [0,1].  The OpenGL
    // viewport is right-handed with origin the lower-left corner of the
    // window, the x-axis is directed rightward, the y-axis is directed
    // upward, and the depth range is a subset of [-1,1].
    virtual void SetViewport(int x, int y, int w, int h) = 0;
    virtual void GetViewport(int& x, int& y, int& w, int& h) const = 0;
    virtual void SetDepthRange(float zmin, float zmax) = 0;
    virtual void GetDepthRange(float& zmin, float& zmax) const = 0;

    // Window resizing.
    virtual bool Resize(unsigned int w, unsigned int h) = 0;

    // Support for clearing the color, depth, and stencil back buffers.
    inline void SetClearColor(std::array<float, 4> const& clearColor);
    inline void SetClearDepth(float clearDepth);
    inline void SetClearStencil(unsigned int clearStencil);
    inline std::array<float, 4> const& GetClearColor() const;
    inline float GetClearDepth() const;
    inline unsigned int GetClearStencil() const;
    virtual void ClearColorBuffer() = 0;
    virtual void ClearDepthBuffer() = 0;
    virtual void ClearStencilBuffer() = 0;
    virtual void ClearBuffers() = 0;
    //virtual void DisplayColorBuffer(unsigned int syncInterval) = 0;

    // Global drawing state.  The default states are shown in GteBlendState.h,
    // GteDepthStencil.h, and GteRasterizerState.h.
    virtual void SetBlendState(std::shared_ptr<BlendState> const& state) = 0;
    inline std::shared_ptr<BlendState> const& GetBlendState() const;
    inline void SetDefaultBlendState();
    inline std::shared_ptr<BlendState> const& GetDefaultBlendState() const;

    virtual void SetDepthStencilState(std::shared_ptr<DepthStencilState> const& state) = 0;
    inline std::shared_ptr<DepthStencilState> const& GetDepthStencilState() const;
    inline void SetDefaultDepthStencilState();
    inline std::shared_ptr<DepthStencilState> const& GetDefaultDepthStencilState() const;

    virtual void SetRasterizerState(std::shared_ptr<RasterizerState> const& state) = 0;
    inline std::shared_ptr<RasterizerState> const& GetRasterizerState() const;
    inline void SetDefaultRasterizerState();
    inline std::shared_ptr<RasterizerState> const& GetDefaultRasterizerState() const;

    // Support for bitmapped fonts used in text rendering.  The default font
    // is Arial (height 18, no italics, no bold).
    void SetFont(std::shared_ptr<Font> const& font);
    inline std::shared_ptr<Font> const& GetFont() const;
    inline void SetDefaultFont();
    inline std::shared_ptr<Font> const& GetDefaultFont() const;

    // Support for drawing.  If occlusion queries are enabled, the return
    // values are the number of samples that passed the depth and stencil
    // tests, effectively the number of pixels drawn.  If occlusion queries
    // are disabled, the functions return 0.

#if 0
	// Draw geometric primitives.
	uint64_t Draw(Visual* visual);
	uint64_t Draw(std::vector<Visual*> const& visuals);
	uint64_t Draw(std::shared_ptr<Visual> const& visual);
	uint64_t Draw(std::vector<std::shared_ptr<Visual>> const& visuals);
	uint64_t Draw(std::shared_ptr<OverlayEffect> const& overlay);
	uint64_t Draw(int x, int y, std::array<float, 4> const& color, std::string const& message);
#endif


    // Draw 2D text.

    // Draw a 2D rectangular overlay.  This is useful for adding buttons,
    // controls, thumbnails, and other GUI objects to an application window.

    // Support for occlusion queries.  When enabled, Draw functions return the
    // number of samples that passed the depth and stencil tests, effectively
    // the number of pixels drawn.  The default value is 'false'.
    inline void AllowOcclusionQuery(bool allow);

    // Support for drawing to offscreen memory (i.e. not to the back buffer).
    // The DrawTarget object encapsulates render targets (color information)
    // and depth-stencil target.
    virtual void Enable(std::shared_ptr<DrawTarget> const& target) = 0;
    virtual void Disable(std::shared_ptr<DrawTarget> const& target) = 0;

    // Graphics object management.  The Bind function creates a graphics
    // API-specific object that corresponds to the input GTEngine object.
    // GraphicsEngine manages this bridge mapping internally.  The Unbind
    // function destroys the graphics API-specific object.  These may be
    // called explicitly, but the engine is designed to create on demand
    // and to destroy on device destruction.
    GEObject* Bind(std::shared_ptr<GraphicsObject> const& object);
    GEDrawTarget* Bind(std::shared_ptr<DrawTarget> const& target);
    GEObject* Get(std::shared_ptr<GraphicsObject> const& object) const;
    GEDrawTarget* Get(std::shared_ptr<DrawTarget> const& target) const;
    inline bool Unbind(std::shared_ptr<GraphicsObject> const& object);
    inline bool Unbind(std::shared_ptr<DrawTarget> const& target);
    void GetTotalAllocation(size_t& numBytes, size_t& numObjects) const;

    // Support for copying from CPU to GPU via mapped memory.
    virtual bool Update(std::shared_ptr<Buffer> const& buffer) = 0;
    virtual bool Update(std::shared_ptr<TextureSingle> const& texture) = 0;
    virtual bool Update(std::shared_ptr<TextureSingle> const& texture, unsigned int level) = 0;
    virtual bool Update(std::shared_ptr<TextureArray> const& textureArray) = 0;
    virtual bool Update(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) = 0;

    // Support for copying from CPU to GPU via staging memory.
    virtual bool CopyCpuToGpu(std::shared_ptr<Buffer> const& buffer) = 0;
    virtual bool CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture) = 0;
    virtual bool CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level) = 0;
    virtual bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray) = 0;
    virtual bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) = 0;

    // Support for copying from GPU to CPU via staging memory.
    virtual bool CopyGpuToCpu(std::shared_ptr<Buffer> const& buffer) = 0;
    virtual bool CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture) = 0;
    virtual bool CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level) = 0;
    virtual bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray) = 0;
    virtual bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) = 0;

#ifdef GL_4
    // Counted buffer management.  GetNumActiveElements stores the result in 'buffer'.
    virtual bool GetNumActiveElements(std::shared_ptr<StructuredBuffer> const& buffer) = 0;

    // Execute the compute program.  If you want the CPU to stall to wait for
    // the results, call WaitForFinish() immediately after Execute(...).
    // However, you can synchronize CPU and GPU activity by calling
    // WaitForFinish() at some later time, the goal being not to stall the
    // CPU before obtaining the GPU results.
    virtual void Execute(std::shared_ptr<ComputeProgram> const& program,
        unsigned int numXGroups, unsigned int numYGroups, unsigned int numZGroups) = 0;
#endif

    // Have the CPU wait until the GPU finishes its current command buffer.
    virtual void WaitForFinish() = 0;

    // Set the warning to 'true' if you want the DX11Engine destructor to
    // report that the bridge maps are nonempty.  If they are, the application
    // did not destroy GraphicsObject items before the engine was destroyed.
    // The default values is 'true'.
    inline void WarnOnNonemptyBridges(bool warn);

protected:
    // Abstract base class.  Graphics engine objects may not be copied.
    GraphicsEngine();
    GraphicsEngine(GraphicsEngine const&) = delete;
    GraphicsEngine& operator=(GraphicsEngine const&) = delete;

    // Helpers for construction and destruction.
    void CreateDefaultGlobalState();
    void DestroyDefaultGlobalState();

    // Support for drawing.  If occlusion queries are enabled, the return
    // values are the number of samples that passed the depth and stencil
    // tests, effectively the number of pixels drawn.  If occlusion queries
    // are disabled, the functions return 0.
    virtual uint64_t DrawPrimitive(
        std::shared_ptr<VertexBuffer> const& vbuffer,
        std::shared_ptr<IndexBuffer> const& ibuffer,
        std::shared_ptr<VisualEffect> const& effect) = 0;

    // Support for GOListener::OnDestroy and DTListener::OnDestroy, because
    // they are passed raw pointers from resource destructors.  These are
    // also used by the Unbind calls whose inputs are std::shared_ptr<T>.
    bool Unbind(GraphicsObject const* object);
    bool Unbind(DrawTarget const* target);


    // The window size.
    unsigned int mXSize, mYSize;

    // Clear values.
    std::array<float, 4> mClearColor;
    float mClearDepth;
    unsigned int mClearStencil;

    // Global state.
    std::shared_ptr<BlendState> mDefaultBlendState;
    std::shared_ptr<BlendState> mActiveBlendState;
    std::shared_ptr<DepthStencilState> mDefaultDepthStencilState;
    std::shared_ptr<DepthStencilState> mActiveDepthStencilState;
    std::shared_ptr<RasterizerState> mDefaultRasterizerState;
    std::shared_ptr<RasterizerState> mActiveRasterizerState;

    // Fonts for text rendering.
    std::shared_ptr<Font> mDefaultFont;
    std::shared_ptr<Font> mActiveFont;

    // Bridge pattern to create graphics API-specific objects that correspond
    // to front-end objects.  The Bind, Get, and Unbind operations act on
    // these maps.
	ThreadSafeMap<GraphicsObject const*, GEObject*> mGOMap;
    ThreadSafeMap<DrawTarget const*, GEDrawTarget*> mDTMap;
    std::unique_ptr<GEInputLayoutManager> mILMap;

    // Creation functions for adding objects to the bridges.  The function
    // pointers are assigned during construction.
    typedef GEObject* (*CreateGEObject)(void*, GraphicsObject const*);
    typedef GEDrawTarget* (*CreateGEDrawTarget)(DrawTarget const*, std::vector<GEObject*>&, GEObject*);
    std::array<CreateGEObject, GT_NUM_TYPES> mCreateGEObject;
    CreateGEDrawTarget mCreateGEDrawTarget;
    void* mGEObjectCreator;

    // Track GraphicsObject destruction and delete to-be-destroyed objects
    // from the bridge map.
    class GOListener : public GraphicsObject::ListenerForDestruction
    {
    public:
        virtual ~GOListener();
        GOListener(GraphicsEngine* engine);
        virtual void OnDestroy(GraphicsObject const* object);
    private:
        GraphicsEngine* mEngine;
    };

    std::shared_ptr<GOListener> mGOListener;

    // Track DrawTarget destruction and delete to-be-destroyed objects from
    // the draw target map.
    class DTListener : public DrawTarget::ListenerForDestruction
    {
    public:
        virtual ~DTListener();
        DTListener(GraphicsEngine* engine);
        virtual void OnDestroy(DrawTarget const* target);
    private:
        GraphicsEngine* mEngine;
    };

    std::shared_ptr<DTListener> mDTListener;

    bool mAllowOcclusionQuery;
    bool mWarnOnNonemptyBridges;
};

inline void GraphicsEngine::SetClearColor(std::array<float, 4> const& clearColor)
{
    mClearColor = clearColor;
}

inline void GraphicsEngine::SetClearDepth(float clearDepth)
{
    mClearDepth = clearDepth;
}

inline void GraphicsEngine::SetClearStencil(unsigned int clearStencil)
{
    mClearStencil = clearStencil;
}

inline std::array<float, 4> const& GraphicsEngine::GetClearColor() const
{
    return mClearColor;
}

inline float GraphicsEngine::GetClearDepth() const
{
    return mClearDepth;
}

inline unsigned int GraphicsEngine::GetClearStencil() const
{
    return mClearStencil;
}

inline std::shared_ptr<BlendState> const& GraphicsEngine::GetBlendState() const
{
    return mActiveBlendState;
}

inline void GraphicsEngine::SetDefaultBlendState()
{
    SetBlendState(mDefaultBlendState);
}

inline std::shared_ptr<BlendState> const& GraphicsEngine::GetDefaultBlendState() const
{
    return mDefaultBlendState;
}

inline std::shared_ptr<DepthStencilState> const& GraphicsEngine::GetDepthStencilState() const
{
    return mActiveDepthStencilState;
}

inline void GraphicsEngine::SetDefaultDepthStencilState()
{
    SetDepthStencilState(mDefaultDepthStencilState);
}

inline std::shared_ptr<DepthStencilState> const& GraphicsEngine::GetDefaultDepthStencilState() const
{
    return mDefaultDepthStencilState;
}

inline std::shared_ptr<RasterizerState> const& GraphicsEngine::GetRasterizerState() const
{
    return mActiveRasterizerState;
}

inline void GraphicsEngine::SetDefaultRasterizerState()
{
    SetRasterizerState(mDefaultRasterizerState);
}

inline std::shared_ptr<RasterizerState> const& GraphicsEngine::GetDefaultRasterizerState() const
{
    return mDefaultRasterizerState;
}

inline std::shared_ptr<Font> const& GraphicsEngine::GetFont() const
{
    return mActiveFont;
}

inline void GraphicsEngine::SetDefaultFont()
{
    SetFont(mDefaultFont);
}

inline std::shared_ptr<Font> const& GraphicsEngine::GetDefaultFont() const
{
    return mDefaultFont;
}

inline bool GraphicsEngine::Unbind(std::shared_ptr<GraphicsObject> const& object)
{
    return Unbind(object.get());
}

inline bool GraphicsEngine::Unbind(std::shared_ptr<DrawTarget> const& target)
{
    return Unbind(target.get());
}

inline void GraphicsEngine::AllowOcclusionQuery(bool allow)
{
    mAllowOcclusionQuery = allow;
}

inline void GraphicsEngine::WarnOnNonemptyBridges(bool warn)
{
    mWarnOnNonemptyBridges = warn;
}

}
