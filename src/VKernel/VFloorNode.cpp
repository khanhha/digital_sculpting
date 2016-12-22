#include "VFloorNode.h"
#include "VView3DRegion.h"
#include "VContext.h"
#include <Eigen/Dense>
#include <memory>

using namespace  gte;
VK_BEGIN_NAMESPACE

VFloorNode::VFloorNode(VScene *scene)
	:_scene(scene)
{
	/*build floor grid*/
	int		grid_side_num	= 10;
	float   grid_side_step	= 1.0f;
	int		tot_verts		= (grid_side_num * 2 + 1) * 4; /*four sides*/

	_axisXColor = EVector4(1.0f, 0.0f, 0.0f, 1.0f);
	_axisYColor = EVector4(0.0f, 1.0f, 0.0f, 1.0f);
	_gridColor = EVector4(96.0f / 256.0f, 98.0f / 256.0f, 97.0f / 256.0f, 1.0f);

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DFType::DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_COLOR, DFType::DF_R32G32B32A32_FLOAT, 0);

	{
		_vBuffer = std::make_shared<VertexBuffer>(vformat, tot_verts, true);
		_iBuffer = std::make_shared<IndexBuffer>(IP_POLYSEGMENT_DISJOINT, tot_verts* 2, sizeof(short));

		struct Vertex
		{
			EVector3 position; 
			EVector4 color;
		};

		Vertex *vdata = _vBuffer->Get<Vertex>();
		short  *idata = _iBuffer->Get<short>();

		int		  vcnt = 0;
		EVector3  p[2];

		/*generate lines parallel to x axis*/
		p[0] = EVector3(-grid_side_num * grid_side_step, 0.0f, 0.0f);
		p[1] = EVector3(+grid_side_num * grid_side_step, 0.0f, 0.0f);
		for (int i = -grid_side_num; i <= grid_side_num; ++i){
			p[0].y() = p[1].y() = i * grid_side_step;
			for (size_t j = 0; j < 2; ++j){
				vdata[vcnt].position = p[j];
				vdata[vcnt].color = i  == 0 ? _axisXColor : _gridColor;
				idata[vcnt] = vcnt;
				vcnt++;
			}
		}
		/*generate lines parallel to y axis*/
		p[0] = EVector3(0.0f, -grid_side_num * grid_side_step, 0.0f);
		p[1] = EVector3(0.0f, +grid_side_num * grid_side_step, 0.0f);
		for (int i = -grid_side_num; i <= grid_side_num; ++i){
			p[0].x() = p[1].x() = i * grid_side_step;
			for (size_t j = 0; j < 2; ++j){
				vdata[vcnt].position = p[j];
				vdata[vcnt].color = i == 0 ? _axisYColor : _gridColor;
				idata[vcnt] = vcnt;
				vcnt++;
			}
		}
	}
}

VFloorNode::~VFloorNode()
{
}

void VFloorNode::onRender(VView3DRegion *region)
{
	ensureEffect();

	EMatrix4x4 pvw = region->projViewMatrix() * worldTransform().matrix();
	_effect->SetPVWMatrix(pvw);
	GL::engine()->Update(_effect->GetPVWMatrixConstant());

	GL::engine()->DrawPrimitive(_vBuffer, _iBuffer, _effect);
}

void VFloorNode::ensureEffect()
{
	if (!_effect){
		/*create effect*/
		GLSLProgramFactory factory;
		_effect = std::make_shared<VertexColorEffect>(factory);
	}
}

VK_END_NAMESPACE
