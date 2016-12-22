// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GteGraphicsEngine.h>
using namespace gte;

GraphicsEngine::~GraphicsEngine()
{
}

GraphicsEngine::GraphicsEngine()
    :
    mXSize(0),
    mYSize(0),
    mClearDepth(1.0f),
    mClearStencil(0),
    mCreateGEDrawTarget(nullptr),
    mGEObjectCreator(nullptr),
    mAllowOcclusionQuery(false),
    mWarnOnNonemptyBridges(true)
{
    mClearColor.fill(0.0f);
    mCreateGEObject.fill(nullptr);

    mGOListener = std::make_shared<GOListener>(this);
    GraphicsObject::SubscribeForDestruction(mGOListener);

    mDTListener = std::make_shared<DTListener>(this);
    DrawTarget::SubscribeForDestruction(mDTListener);
}

void GraphicsEngine::SetFont(std::shared_ptr<Font> const& font)
{
    if (font)
    {
        if (font != mActiveFont)
        {
            // Destroy font resources in GPU memory.  The mActiveFont should
            // be null once, only when the mDefaultFont is created.
            if (mActiveFont)
            {
                Unbind(mActiveFont->GetVertexBuffer());
                Unbind(mActiveFont->GetIndexBuffer());
                Unbind(mActiveFont->GetTextEffect()->GetTranslate());
                Unbind(mActiveFont->GetTextEffect()->GetColor());
                Unbind(mActiveFont->GetTextEffect()->GetVertexShader());
                Unbind(mActiveFont->GetTextEffect()->GetPixelShader());
            }

            mActiveFont = font;

            // Create font resources in GPU memory.
            Bind(mActiveFont->GetVertexBuffer());
            Bind(mActiveFont->GetIndexBuffer());
            Bind(mActiveFont->GetTextEffect()->GetTranslate());
            Bind(mActiveFont->GetTextEffect()->GetColor());
            Bind(mActiveFont->GetTextEffect()->GetVertexShader());
            Bind(mActiveFont->GetTextEffect()->GetPixelShader());
        }
    }
    else
    {
        qCritical("Input font is null.");
    }
}

#if 0
uint64_t GraphicsEngine::Draw(Visual* visual)
{
	if (visual)
	{
		auto const& vbuffer = visual->GetVertexBuffer();
		auto const& ibuffer = visual->GetIndexBuffer();
		auto const& effect = visual->GetEffect();
		if (vbuffer && ibuffer && effect)
		{
			return DrawPrimitive(vbuffer, ibuffer, effect);
		}
	}

	qCritical("Null input to Draw.");
	return 0;
}

uint64_t GraphicsEngine::Draw(std::vector<Visual*> const& visuals)
{
	uint64_t numPixelsDrawn = 0;
	for (auto const& visual : visuals)
	{
		numPixelsDrawn += Draw(visual);
	}
	return numPixelsDrawn;
}


uint64_t GraphicsEngine::Draw(std::shared_ptr<Visual> const& visual)
{
	return Draw(visual.get());
}


uint64_t GraphicsEngine::Draw(std::vector<std::shared_ptr<Visual>> const& visuals)
{
	uint64_t numPixelsDrawn = 0;
	for (auto const& visual : visuals)
	{
		numPixelsDrawn += Draw(visual);
	}
	return numPixelsDrawn;
}


uint64_t GraphicsEngine::Draw(int x, int y, std::array<float, 4> const& color, std::string const& message)
{
	uint64_t numPixelsDrawn;

	if (message.length() > 0)
	{
		int vx, vy, vw, vh;
		GetViewport(vx, vy, vw, vh);
		mActiveFont->Typeset(vw, vh, x, y, color, message);

		Update(mActiveFont->GetTextEffect()->GetTranslate());
		Update(mActiveFont->GetTextEffect()->GetColor());
		Update(mActiveFont->GetVertexBuffer());

		// We need to restore default state for text drawing.  Remember the
		// current state so that we can reset it after drawing.
		std::shared_ptr<BlendState> bState = GetBlendState();
		std::shared_ptr<DepthStencilState> dState = GetDepthStencilState();
		std::shared_ptr<RasterizerState> rState = GetRasterizerState();
		SetDefaultBlendState();
		SetDefaultDepthStencilState();
		SetDefaultRasterizerState();

		numPixelsDrawn = DrawPrimitive(mActiveFont->GetVertexBuffer(),
			mActiveFont->GetIndexBuffer(), mActiveFont->GetTextEffect());

		SetBlendState(bState);
		SetDepthStencilState(dState);
		SetRasterizerState(rState);
	}
	else
	{
		numPixelsDrawn = 0;
	}

	return numPixelsDrawn;
}

uint64_t GraphicsEngine::Draw(std::shared_ptr<OverlayEffect> const& overlay)
{
	if (overlay)
	{
		auto const& vbuffer = overlay->GetVertexBuffer();
		auto const& ibuffer = overlay->GetIndexBuffer();
		auto const& effect = overlay->GetEffect();
		if (vbuffer && ibuffer && effect)
		{
			return DrawPrimitive(vbuffer, ibuffer, effect);
		}
	}

	qCritical("Null input to Draw.");
	return 0;
}
#endif

GEObject* GraphicsEngine::Bind(std::shared_ptr<GraphicsObject> const& object)
{
    if (!object)
    {
		qCritical("Attempt to bind a null object.");
        return nullptr;
    }

    GraphicsObject const* gtObject = object.get();
    GEObject* geObject = nullptr;
    if (!mGOMap.Get(gtObject, geObject))
    {
        // The 'create' function is not null with the current engine design.
        // If the assertion is triggered, someone changed the hierarchy of
        // GraphicsObjectType but did not change msCreateFunctions[] to match.
        CreateGEObject create = mCreateGEObject[object->GetType()];
        if (!create)
        {
            // No logger message is generated here because GL4 does not
            // have shader creation functions.
            return nullptr;
        }

        geObject = create(mGEObjectCreator, gtObject);
		if (!geObject){
			qCritical("Null object.  Out of memory?");
		}

        mGOMap.Insert(gtObject, geObject);
#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
        geObject->SetName(object->GetName());
#endif
    }
    return geObject;
}

GEDrawTarget* GraphicsEngine::Bind(std::shared_ptr<DrawTarget> const& target)
{
    DrawTarget const* gtTarget = target.get();
    GEDrawTarget* geTarget = nullptr;
    if (!mDTMap.Get(gtTarget, geTarget))
    {
        unsigned int const numTargets = target->GetNumTargets();
        std::vector<GEObject*> rtTextures(numTargets);
        for (unsigned int i = 0; i < numTargets; ++i)
        {
            rtTextures[i] = static_cast<GEObject*>(Bind(target->GetRTTexture(i)));
        }

        std::shared_ptr<TextureDS> object = target->GetDSTexture();
        GEObject* dsTexture;
        if (object)
        {
            dsTexture = static_cast<GEObject*>(Bind(object));
        }
        else
        {
            dsTexture = nullptr;
        }

        geTarget = mCreateGEDrawTarget(gtTarget, rtTextures, dsTexture);
        mDTMap.Insert(gtTarget, geTarget);
    }
    return geTarget;
}

GEObject* GraphicsEngine::Get(std::shared_ptr<GraphicsObject> const& object) const
{
    GraphicsObject const* gtObject = object.get();
    GEObject* geObject = nullptr;
    if (mGOMap.Get(gtObject, geObject))
    {
        return geObject;
    }
    return nullptr;
}

GEDrawTarget* GraphicsEngine::Get(std::shared_ptr<DrawTarget> const& target) const
{
    DrawTarget const* gtTarget = target.get();
    GEDrawTarget* geTarget = nullptr;
    if (!mDTMap.Get(gtTarget, geTarget))
    {
		qCritical("Cannot find draw target.");
    }
    return geTarget;
}

void GraphicsEngine::GetTotalAllocation(size_t& numBytes, size_t& numObjects) const
{
    numBytes = 0;
    numObjects = 0;
    std::vector<GEObject*> objects;
    mGOMap.GatherAll(objects);
    for (auto object : objects)
    {
        if (object)
        {
            auto resource = dynamic_cast<Resource*>(object->GetGraphicsObject());
            if (resource)
            {
                ++numObjects;
                numBytes += resource->GetNumBytes();
            }
        }
    }
}

void GraphicsEngine::CreateDefaultGlobalState()
{
    mDefaultBlendState = std::make_shared<BlendState>();
    mDefaultDepthStencilState = std::make_shared<DepthStencilState>();
    mDefaultRasterizerState = std::make_shared<RasterizerState>();
	mClearColor[0] = 175.f / 255.0f;
	mClearColor[1] = 176.f / 255.0f;
	mClearColor[2] = 176.f / 255.0f;

#if defined(GTE_GRAPHICS_USE_NAMED_OBJECTS)
    mDefaultBlendState->SetName("GraphicsEngine::mDefaultBlendState");
    mDefaultDepthStencilState->SetName("GraphicsEngine::mDefaultDepthStencilState");
    mDefaultRasterizerState->SetName("GraphicsEngine::mDefaultRasterizerState");
#endif

    SetDefaultBlendState();
    SetDefaultDepthStencilState();
    SetDefaultRasterizerState();
}

void GraphicsEngine::DestroyDefaultGlobalState()
{
    if (mDefaultBlendState)
    {
        Unbind(mDefaultBlendState);
        mDefaultBlendState = nullptr;
        mActiveBlendState = nullptr;
    }

    if (mDefaultDepthStencilState)
    {
        Unbind(mDefaultDepthStencilState);
        mDefaultDepthStencilState = nullptr;
        mActiveDepthStencilState = nullptr;
    }

    if (mDefaultRasterizerState)
    {
        Unbind(mDefaultRasterizerState);
        mDefaultRasterizerState = nullptr;
        mActiveRasterizerState = nullptr;
    }
}

bool GraphicsEngine::Unbind(GraphicsObject const* object)
{
    GEObject* dxObject = nullptr;
    if (mGOMap.Get(object, dxObject))
    {
        GraphicsObjectType type = object->GetType();
        if (type == GT_VERTEX_BUFFER)
        {
            mILMap->Unbind(static_cast<VertexBuffer const*>(object));
        }
        else if (type == GT_VERTEX_SHADER)
        {
            mILMap->Unbind(static_cast<Shader const*>(object));
        }

        if (mGOMap.Remove(object, dxObject))
        {
            delete dxObject;
        }

        return true;
    }
    return false;
}

bool GraphicsEngine::Unbind(DrawTarget const* target)
{
    GEDrawTarget* dxTarget = nullptr;
    if (mDTMap.Remove(target, dxTarget))
    {
        delete dxTarget;
        return true;
    }

    return false;
}

GraphicsEngine::GOListener::~GOListener()
{
}

GraphicsEngine::GOListener::GOListener(GraphicsEngine* engine)
    :
    mEngine(engine)
{
}

void GraphicsEngine::GOListener::OnDestroy(GraphicsObject const* object)
{
    if (mEngine)
    {
        mEngine->Unbind(object);
    }
}

GraphicsEngine::DTListener::~DTListener()
{
}

GraphicsEngine::DTListener::DTListener(GraphicsEngine* engine)
    :
    mEngine(engine)
{
}

void GraphicsEngine::DTListener::OnDestroy(DrawTarget const* target)
{
    if (mEngine)
    {
        mEngine->Unbind(target);
    }
}
