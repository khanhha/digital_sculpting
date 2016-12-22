#ifndef VKERNEL_VRENDER_H
#define VKERNEL_VRENDER_H
#include <memory>
#include <GTGraphics.h>
#include <QtGui/QOpenGLFunctions>
#include <QtQuick/QQuickWindow>
#include "VKernelCommon.h"
#include "VScene.h"
#include "VNode.h"
#include "VCamera.h"
#include "VMeshObject.h"
#include "VPVWUpdater.h"
#include "VView3DEditBox.h"
#include <VCamera.h>
#include <QObject>

VK_BEGIN_NAMESPACE
class VContext;
class VView3DRegion : public QObject
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
	Q_OBJECT
public:
	VView3DRegion();
	~VView3DRegion();

	bool initialized();
	void initialize();
	void release();
	void setScene(VScene* scene);
	void syncUpdate(VContext *context);
	void onRender(VContext *context);
	
	void		setViewport(const QRect &rect);
	QRect		viewport() const;
	Vector4i	viewport_1() const;
	float		aspect() const;

	Matrix4x4	projMatrix();
	Matrix4x4	viewMatrix();
	Matrix4x4	projViewMatrix();

	VCamera*	camera();
	Vector3		cameraPosition();

	VView3DEditBox* viewEditBox();
	float		x();
	float		y();
	float		width();
	float		height();
	Vector2f	localRectPos(Vector2f pos);
	const gte::BufferUpdater& bufferUpdater();
private:
	void createRenderStates();
	void renderDebugPoints();
	void initializePartialRenderTarget();
	void calculateScissorRectangle(const Eigen::Matrix4f &pvm, const QRect &viewport, const Eigen::AlignedBox3f &bb, Vector2f &rect_lower, Vector2f &rect_upper);
	void calculateScissorBoxTubeFrustum(const Eigen::Matrix4f &pvm, const QRect &viewport, const Eigen::Vector2f &lower, const Eigen::Vector2f &upper, Vector4f planes[4]);

private:
	bool					 mInitialized;
	VScene					*mScene;
	VCamera					*mCamera;
	VView3DEditBox			*mBoxView;
	gte::BufferUpdater					mUpdater;
	QRect								m_viewportSize;
	std::shared_ptr<gte::DrawTarget>	mPartialDrawTarget;
	QRect								mScissorRect;
	Eigen::AlignedBox3f					mLastChangedBB;
	Eigen::Vector4f						mCullingPlanes[4];

	/*draw data*/
	std::shared_ptr<gte::BlendState>		mBlendState;
	std::shared_ptr<gte::RasterizerState>	mWireState;
	std::shared_ptr<gte::DepthStencilState> mNoDepthStencilState;
	std::shared_ptr<gte::OverlayEffect>		mOverlay;

};
VK_END_NAMESPACE
#endif