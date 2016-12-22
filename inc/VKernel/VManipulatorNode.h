#ifndef VKERNEL_MANIPULATOR_H
#define VKERNEL_MANIPULATOR_H

#include "VKernel/VKernelCommon.h"
#include "VKernel/VNode.h"
#include "VKernel/VScene.h"
#include "VbsQt/VbsDef.h"

VK_BEGIN_NAMESPACE
class VManipulatorNode : public VNode
{
private:
	struct VertexData
	{
		gte::EVector3 co;
		gte::EVector4 color;
	};

public:

public:
	VManipulatorNode(VScene *scene);
	virtual ~VManipulatorNode();
	virtual void onRender(VView3DRegion *region);

	void update(const Eigen::Vector3f &org);
	void drawCube(const Eigen::Vector3f &center, double radius, double color[3]);
	void drawCone(const Eigen::Vector3f &center, const Eigen::Vector3f &axis, double radius, double color[3]);

	bool selectState(VView3DRegion *region, Eigen::Vector2f glMouse);
	bool poll(VView3DRegion *region, Eigen::Vector2f glMouse) const;

	void viewDir(VView3DRegion *region, Eigen::Vector2f glmouse, Eigen::Vector3f &org, Eigen::Vector3f &dir) const;
	char axis();
	char mode();
	void setAxis(char axis);
	void setMode(char mode);

	gte::EVector4 axisColor(int i) const;
	void beginManipulator();
	void endManipulator();

	/*receive signal from scene*/
	void	onActiveObjectChanged(VObject *obj);
	float	zoomFactor(VView3DRegion *region, Eigen::Vector3f p) const;
private:
	void ensureEffect();
	void createGPUBuffer();
	void createTransGPUBuffer();
	void createScaleGPUBuffer();
	void createRotateGPUBuffer();
	void updateRotateCircleColor(const Eigen::Vector3f &cameraPosition);

	bool isectManipulatorBB(float scale, const Eigen::Vector3f &org, const Eigen::Vector3f &dir) const;
	bool isectTransCenter(float scale, const Eigen::Vector3f &org, const Eigen::Vector3f &dir) const;
	int  isectTransAxis(float scale, const Eigen::Vector3f &org, const Eigen::Vector3f &dir) const;
	int  isectScaleAxis(float scale, const Eigen::Vector3f &org, const Eigen::Vector3f &dir) const;
	int  isectRotateAxis(float scale, const Eigen::Vector3f &org, const Eigen::Vector3f &dir) const;
private:
	VScene *_scene;
	typedef std::pair<std::shared_ptr<gte::VertexBuffer>, std::shared_ptr<gte::IndexBuffer>> GPUBuffer;
	std::shared_ptr<gte::ConstantColorEffect>	_effect;
	std::shared_ptr<gte::VertexColorEffect>		_vcolorEffect;

	std::shared_ptr<gte::DepthStencilState>		_disableDepthState;
	const float									_pixel_size;

	std::array<GPUBuffer, 3> _scaleAxisGPUBuffer;
	std::array<GPUBuffer, 3> _transAxisGPUBuffer;
	std::array<GPUBuffer, 3> _axisGPUBuffer;
	std::array<GPUBuffer, 3> _rotateGPUBuffer;

	GPUBuffer _transCenterBuffer;

	Eigen::Vector3f _org;
	Eigen::Vector3f _basis[3];
	float	_radius;
	float   _selectSize;
	float	_centerSize;
	char	_transAxis;
	char	_transMode;
	bool	_isManipulating;
};

VK_END_NAMESPACE
#endif