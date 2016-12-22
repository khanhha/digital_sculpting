#include "Operator/VOpTemplate.h"
#include "VViewEditOp.h"
#include "VDropMeshOp.h"
#include "VSculptBrushOp.h"
#include "VView3DEditBoxOp.h"
#include "VSculptConfigOp.h"
#include "VCommonConfigOp.h"
#include "VMeshSmoothOp.h"
#include "VMeshDecimateOp.h"
#include "VMeshRemeshOp.h"
#include "VManipulatorOp.h"
#include "VMeshSelfIsectOp.h"
#include "VMeshBooleanOp.h"


std::vector<VOpTemplate::VOperatorType*>	VOpTemplate::op_templates;

void VOpTemplate::register_operators()
{
	op_templates.resize(VOpTemplate::TYPE_TOTAL);

	VOperatorType *optype;

	optype = new VOperatorType();
	optype->pollFunc = &VViewMoveOp::poll;
	optype->creatorFunc = &VViewMoveOp::create;
	optype->type = VOpTemplate::TYPE_VIEW_MOVE;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VViewZoomOp::poll;
	optype->creatorFunc = &VViewZoomOp::create;
	optype->type = VOpTemplate::TYPE_VIEW_ZOOM;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VViewRotateOp::poll;
	optype->creatorFunc = &VViewRotateOp::create;
	optype->type = VOpTemplate::TYPE_VIEW_ROTATE;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VDropMeshOp::poll;
	optype->creatorFunc = &VDropMeshOp::create;
	optype->type = VOpTemplate::TYPE_DROP_MESH;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VSculptBrushOp::poll;
	optype->creatorFunc = &VSculptBrushOp::create;
	optype->type = VOpTemplate::TYPE_SCULPT_BRUSH;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VView3DEditBoxOp::poll;
	optype->creatorFunc = &VView3DEditBoxOp::create;
	optype->type = VOpTemplate::TYPE_VIEW3D_BOX_EDIT;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VSculptConfigOp::poll;
	optype->creatorFunc = &VSculptConfigOp::create;
	optype->type = VOpTemplate::OP_SCULPT_CONFIG;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VMeshSmoothOp::poll;
	optype->creatorFunc = &VMeshSmoothOp::create;
	optype->type = VOpTemplate::OP_MESH_SMOOTH;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VMeshDecimateOp::poll;
	optype->creatorFunc = &VMeshDecimateOp::create;
	optype->type = VOpTemplate::OP_MESH_DECIMATE;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VMeshRemeshOp::poll;
	optype->creatorFunc = &VMeshRemeshOp::create;
	optype->type = VOpTemplate::OP_MESH_REMESH;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VManipulatorOp::poll;
	optype->creatorFunc = &VManipulatorOp::create;
	optype->type = VOpTemplate::OP_MANIPULATOR;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VCommonConfigOp::poll;
	optype->creatorFunc = &VCommonConfigOp::create;
	optype->type = VOpTemplate::OP_COMMON_CONFIG;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VMeshSelfIsectOp::poll;
	optype->creatorFunc = &VMeshSelfIsectOp::create;
	optype->type = VOpTemplate::OP_MESH_SELF_ISECT;
	op_templates[optype->type] = optype;

	optype = new VOperatorType();
	optype->pollFunc = &VMeshBooleanOp::poll;
	optype->creatorFunc = &VMeshBooleanOp::create;
	optype->type = VOpTemplate::OP_MESH_BOOLEAN;
	op_templates[optype->type] = optype;
}

VOpTemplate::VOperatorType& VOpTemplate::op_template(OP_TYPE idx)
{
	return *op_templates[idx];
}
