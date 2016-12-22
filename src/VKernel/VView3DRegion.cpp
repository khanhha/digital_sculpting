#include <Graphics/GteMeshFactory.h>
#include <tbb/mutex.h>
#include <Graphics/GL4/GteGL4DrawTarget.h>
#include "VView3DRegion.h"
#include "VSceneRenderer.h"
#include "VContext.h"

using namespace gte;

#define DEBUG_DRAW 1
#ifdef DEBUG_DRAW
std::vector<std::pair<Vector3f, Vector3f>> g_debug_points; /*point, color*/
std::vector<std::tuple<Vector3f, Vector3f, Vector3f>> g_debug_segments; /*point_1, point_2, color*/
#endif

VK_BEGIN_NAMESPACE

VView3DRegion::VView3DRegion()
	:
	mInitialized(false),
	mBoxView(nullptr)
{
	/*note: camera is a VNode, but not a VScene's child*/
	mCamera = new VCamera();
	float aspectRatio = 1920.0f / 1001.0f;
	mCamera->setPerspectiveProjection(45.0f, aspectRatio, .1f, 1000.0f);

	Vector3 camPosition(-15.0f, -15.0f, 20.0f);
	Vector3 viewCenter(0.0f, 0.0f, 0.0f);
	Vector3 viewdir = (viewCenter - camPosition).normalized();
	Vector3 camRight = (viewdir.cross(Vector3f(0.0, 0.0f, 1.0f))).normalized();
	Vector3 camUp = camRight.cross(viewdir);
	mCamera->setFrame(camPosition, camUp.normalized(), viewCenter);

	mBoxView = new VView3DEditBox();
}

VView3DRegion::~VView3DRegion()
{
}

void VView3DRegion::setScene(VScene *scene)
{
	mScene = scene;
}

bool VView3DRegion::initialized()
{
	return mInitialized;
}

void VView3DRegion::initialize()
{
	if (!mInitialized){
		mInitialized = true;
		createRenderStates();
	}
}

void VView3DRegion::createRenderStates()
{
	mBlendState = std::make_shared<BlendState>();
	mBlendState->target[0].enable = true;
	mBlendState->target[0].srcColor = BlendState::BM_SRC_ALPHA;
	mBlendState->target[0].dstColor = BlendState::BM_INV_SRC_ALPHA;
	mBlendState->target[0].srcAlpha = BlendState::BM_SRC_ALPHA;
	mBlendState->target[0].dstAlpha = BlendState::BM_INV_SRC_ALPHA;

	mNoDepthStencilState = std::make_shared<DepthStencilState>();
	mNoDepthStencilState->depthEnable = false;
	mNoDepthStencilState->stencilEnable = false;

	mWireState = std::make_shared<RasterizerState>();
	mWireState->fillMode = RasterizerState::FILL_WIREFRAME;
}

/*called from render thread while GUI thread is blocked*/
void VView3DRegion::syncUpdate(VContext *context)
{
	VScene			*scene		= context->scene();
	Eigen::Matrix4f  pvw		= projViewMatrix() * scene->worldMatrix();
	QRect  vp	= viewport();

	/*calculate the changed bounding box from all mesh objects*/
	Eigen::AlignedBox3f changedBounds = scene->gpuChangedBound();

	if (!changedBounds.isEmpty()){
		Vector2f rect_lower, rect_upper;
		calculateScissorRectangle(pvw, vp, changedBounds, rect_lower, rect_upper);

		calculateScissorBoxTubeFrustum(pvw, vp, rect_lower, rect_upper, mCullingPlanes);

		mScissorRect = QRect(
			rect_lower.x(), 
			rect_lower.y(), 
			rect_upper.x() - rect_lower.x(), 
			rect_upper.y() - rect_lower.y());

		/*restrict scissor rectangle to region*/
		if (mScissorRect.intersects(viewport())){
			mScissorRect = mScissorRect & viewport();
		}

	}
	else{
		mScissorRect = QRect(0, 0, 0, 0); /*null rectangle*/
	}
}

void VView3DRegion::onRender(VContext *context)
{
	/*render first since glBlitFramebuffer overwrite the default Framebuffer, 
	which make anything renderer earlier cleared*/

	GL::engine()->SetViewport(m_viewportSize.x(), m_viewportSize.y(), m_viewportSize.width(), m_viewportSize.height());
	GL::engine()->SetDefaultDepthStencilState();
	GL::engine()->SetDefaultRasterizerState();

	if (!mScissorRect.isNull()){
		GL::function().glScissor(mScissorRect.x(), mScissorRect.y(), mScissorRect.width(), mScissorRect.height());
	}
	else{
		GL::function().glScissor(m_viewportSize.x(), m_viewportSize.y(), m_viewportSize.width(), m_viewportSize.height());
	}
	
	GL::function().glEnable(GL_SCISSOR_TEST);
	GL::engine()->ClearBuffers();

	/*special handling mesh objects*/
	if (!mScissorRect.isNull()){
		VSceneRender::instance()->render(this, &mCullingPlanes);
	}
	else{
		VSceneRender::instance()->render(this, nullptr);
	}

	mScene->onRender(this);
	mBoxView->onRender(this);

	//{
	//	float len = 2.0f;
	//	Vector3f org = Vector3f(1,1,1);
	//	Vector3f x = org + Vector3f(len, 0, 0);
	//	Vector3f y = org + Vector3f(0, len, 0);
	//	Vector3f z = org + Vector3f(0, 0.0, len);
	//	g_debug_segments.clear();
	//	g_debug_segments.push_back(std::make_tuple(org, x, Vector3f(1.0f, 0.0f, 0.0f)));
	//	g_debug_segments.push_back(std::make_tuple(org, y, Vector3f(0.0f, 1.0f, 0.0f)));
	//	g_debug_segments.push_back(std::make_tuple(org, z, Vector3f(0.0f, 0.0f, 1.0f)));
	//}
	renderDebugPoints();

	GL::function().glDisable(GL_SCISSOR_TEST);
}

void VView3DRegion::renderDebugPoints()
{
	if (g_debug_points.size() || g_debug_segments.size() > 0){

		struct Vertex 
		{
			Vector3 pos;
			Vector4 color;
		};
		// Create a vertex buffer for a single triangle.
		VertexFormat vformat;
		vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
		vformat.Bind(VA_COLOR, DF_R32G32B32A32_FLOAT, 0);
		std::shared_ptr<VertexBuffer> point_buffer, line_buffer;
		if (g_debug_points.size() > 0){
			point_buffer = std::make_shared<VertexBuffer>(vformat, g_debug_points.size());
			Vertex *vdata = point_buffer->Get<Vertex>();
			for (size_t i = 0; i < g_debug_points.size(); ++i){
				vdata[i].pos = g_debug_points[i].first;
				vdata[i].color = Vector4(g_debug_points[i].second[0], g_debug_points[i].second[1], g_debug_points[i].second[2], 1.0f);
			}
		}

		if (g_debug_segments.size() > 0){
			line_buffer = std::make_shared<VertexBuffer>(vformat, 2 * g_debug_segments.size());
			Vertex *vdata = line_buffer->Get<Vertex>();
			for (size_t i = 0; i < g_debug_segments.size(); ++i){
				Vector3f p0 = std::get<0>(g_debug_segments[i]);
				Vector3f p1 = std::get<1>(g_debug_segments[i]);
				Vector3f color = std::get<2>(g_debug_segments[i]);
				vdata[2 * i].pos = p0;
				vdata[2 * i].color = Vector4(color[0], color[1], color[2], 1.0f);
				vdata[2 * i + 1].pos = p1;
				vdata[2 * i + 1].color = Vector4(color[0], color[1], color[2], 1.0f);
			}
		}
	

		gte::GLSLProgramFactory factory;
		// Create an effect for the vertex and pixel shaders.
		Vector4 color(1.0f, 0.0f, 0.0f, 0.0f);
		std::shared_ptr<VertexColorEffect> effect = std::make_shared<VertexColorEffect>(factory);

		EMatrix4x4 pvw = this->projViewMatrix() * mScene->worldMatrix();

		effect->GetPVWMatrixConstant()->SetMember("pvwMatrix", pvw);
		GL::engine()->Update(effect->GetPVWMatrixConstant());

		GL::engine()->Enable(effect);
		/*TODO: move disable depth test disable into engine*/
		glDisable(GL_DEPTH_TEST);
		if (g_debug_points.size() > 0)
			GL::engine()->DrawVertexBuffer(effect, point_buffer, GL_POINTS);
		if (g_debug_segments.size() > 0)
			GL::engine()->DrawVertexBuffer(effect, line_buffer, GL_LINES);
		glEnable(GL_DEPTH_TEST);
		GL::engine()->Disable(effect);
	}
}

const BufferUpdater& VView3DRegion::bufferUpdater()
{
	return mUpdater;
}

void VView3DRegion::initializePartialRenderTarget()
{
	mPartialDrawTarget = std::make_shared<DrawTarget>(
		1, DF_R32G32B32A32_FLOAT, m_viewportSize.width(), m_viewportSize.height(), 
		false, false, DF_D24_UNORM_S8_UINT, false);
}


void VView3DRegion::calculateScissorRectangle(
	const Eigen::Matrix4f &pvm, const QRect &viewport, const Eigen::AlignedBox3f &bb,
	Vector2f &rect_lower, Vector2f &rect_upper)
{
	Eigen::Vector3f bbsize = bb.sizes();

	Eigen::Vector2f smin(10000.0f, 10000.0f);
	Eigen::Vector2f smax(-10000.0f, -10000.0f);

	for (size_t ix = 0; ix < 2; ix++)
	{
		for (size_t iy = 0; iy < 2; iy++)
		{
			for (size_t iz = 0; iz < 2; iz++)
			{
				Eigen::Vector3f point = bb.min();

				if (ix) point.x() += bbsize.x();
				if (iy) point.y() += bbsize.y();
				if (iz) point.z() += bbsize.z();

				Eigen::Vector3f proj = MathUtil::project(pvm, viewport, point);
		
				smin.x() = std::min<float>(smin.x(), proj.x());
				smin.y() = std::min<float>(smin.y(), proj.y());
				smax.x() = std::max<float>(smax.x(), proj.x());
				smax.y() = std::max<float>(smax.y(), proj.y());
			}
		}
	}
	smin.x() -= 2.0f;
	smin.y() -= 2.0f;
	smax.x() += 2.0f;
	smax.y() += 2.0f;

	rect_lower = smin;
	rect_upper = smax;
}

void VView3DRegion::calculateScissorBoxTubeFrustum(const Eigen::Matrix4f &pvm, const QRect &viewport, const Eigen::Vector2f &lower, const Eigen::Vector2f &upper, Vector4f planes[4])
{
	VScene *scene = VContext::instance()->scene();
	Matrix4f inv_pvm = pvm.inverse();

	float xlen = upper.x() - lower.x();
	float ylen = upper.y() - lower.y();
	Eigen::Vector3f box[8];

	Eigen::Vector2f corner[4];
	corner[0] = Eigen::Vector2f(lower);
	corner[1] = Eigen::Vector2f(lower.x() + xlen, lower.y());
	corner[2] = Eigen::Vector2f(lower.x() + xlen, lower.y() + ylen);
	corner[3] = Eigen::Vector2f(lower.x(), lower.y() + ylen);

	for (size_t i = 0; i < 4; ++i){
		Vector3f win_near(corner[i].x(), corner[i].y(), 0.2f);
		box[i] = MathUtil::unproject(inv_pvm, viewport, win_near);

		Vector3f win_far(corner[i].x(), corner[i].y(), 0.99f);
		box[i + 4] = MathUtil::unproject(inv_pvm, viewport, win_far);

	}

	Vector3f norm;
	for (size_t i = 0; i < 4; ++i){
		MathGeom::normal_tri_v3(norm, box[i], box[i == 3 ? 0 : i + 1], box[i + 4]);
		MathGeom::plane_from_point_normal_v3(planes[i], box[i], -norm);
	}
}

void VView3DRegion::setViewport(const QRect &rect)
{
	m_viewportSize = rect;
}

QRect VView3DRegion::viewport() const
{
	return m_viewportSize;
}


Vector4i VView3DRegion::viewport_1() const
{
	return Vector4i(
		static_cast<int>(m_viewportSize.x()), static_cast<int>(m_viewportSize.y()),
		static_cast<int>(m_viewportSize.width()), static_cast<int>(m_viewportSize.height()));
}

float VView3DRegion::aspect() const
{
	return m_viewportSize.width() / m_viewportSize.height();
}

Matrix4x4 VView3DRegion::projMatrix()
{
	return mCamera->projectionMatrix();
}

Matrix4x4 VView3DRegion::viewMatrix()
{
	return mCamera->viewMatrix();
}

Matrix4x4 VView3DRegion::projViewMatrix()
{
	return mCamera->projectionMatrix() * mCamera->viewMatrix();
}

float VView3DRegion::x()
{
	return m_viewportSize.x();
}

float VView3DRegion::y()
{	
	return m_viewportSize.y();
}

float VView3DRegion::width()
{
	return m_viewportSize.width();
}

float VView3DRegion::height()
{
	return m_viewportSize.height();
}


VCamera* VView3DRegion::camera()
{
	return mCamera;
}

Vector3 VView3DRegion::cameraPosition()
{
	return mCamera->position();
}

VView3DEditBox* VView3DRegion::viewEditBox()
{
	return mBoxView;
}

Vector2f VView3DRegion::localRectPos(Vector2f pos)
{
	return Vector2f(pos.x() - x(), pos.y() - y());
}

VK_END_NAMESPACE


