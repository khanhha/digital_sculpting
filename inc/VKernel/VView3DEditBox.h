#ifndef VKERNEL_VBOX_VIEW_NODE_H
#define VKERNEL_VBOX_VIEW_NODE_H
#include "VKernel/VKernelCommon.h"
#include "VKernel/VNode.h"
#include "VKernel/VScene.h"
#include <Eigen/Dense>
VK_BEGIN_NAMESPACE
class VView3DEditBox
{
	struct BoxSelect
	{
		enum State{
			SEL_CORNER,
			SEL_SIDE,
			SEL_EDGE,
			SEL_NONE
		};

		State	 stt;
		/*
		corner: pos = (1,1,1), (1,-1,1)... total corner = 8
		side: pos = (1,0,0): x side, (-1, 0, 0): -x side, (0, 1, 0) : y side... total sides = 6
		edge: pos = (2, 1, -1) pos[0] = 2 ==> edge parallel to axis 2. pos[0] = 1 ==> edge parallel to axis 1
		*/
		Eigen::Vector3f dir;

	};

	struct VertexData
	{
		gte::EVector3 position;
		gte::EVector2 texture;
		gte::EVector4 color;
	};

	struct VertexInfor
	{
		VertexData *vdata;
		Eigen::Vector3f pos; /*corner/edge/side*/
		BoxSelect::State stt; /*current hightligth*/
	};

	struct BoxMesh
	{
		std::vector<VertexInfor>			 verts;
		bool								 updateGL;
		std::shared_ptr<gte::VertexBuffer>   vbuffer;
		std::shared_ptr<gte::IndexBuffer>	 ibuffer;

			
		/*current select highlight status*/
		BoxSelect::State stt; 
		Eigen::Vector3f dir;

		/*current view highlight status*/
		BoxSelect::State vstt;
		Eigen::Vector3f vdir;
	};

public:
	VView3DEditBox();
	~VView3DEditBox();
	void  onRender(VView3DRegion *region);
	bool  select(Eigen::Vector2f pos);
	bool  viewdir(Eigen::Vector3f &d);
	QRect rect();
	void  resetSelect();
private:
	void updateHighlightColor();
	void ensureEffect();
	void createBoxGeometry();
	void resetHightlight();
	void classifyVertsRegion();
	bool classifyPointRegion(const Eigen::Vector3f &coord, BoxSelect::State &stt, Eigen::Vector3f &pos) const;
	bool neareastRegion(Eigen::Vector3f dir, BoxSelect::State &stt, Eigen::Vector3f &pos);
private:
	float	_pixel_extent;
	QRect	_rect;
	BoxSelect	_select;
	float		_region_size;
	BoxMesh		_box;
	std::shared_ptr<VCamera>					_camera;
	std::shared_ptr<gte::Texture2>				_texture;
	std::shared_ptr<gte::Texture2ColorEffect>	_effect;
};

VK_END_NAMESPACE
#endif