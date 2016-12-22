#ifndef SCULPT_VOPERATOR_TEMPLATE_H
#define SCULPT_VOPERATOR_TEMPLATE_H

#include "Operator/VOperator.h"
#include <QObject>

class VOpTemplate : public QObject{
public:
	Q_OBJECT
public:
	enum OP_TYPE
	{
		TYPE_ANCHOR_EDIT,

		TYPE_BOOL,
		TYPE_DROP_MESH,

		TYPE_BMESH_MAKE,
		TYPE_BMESH_NODE_EDIT,
		TYPE_BMESH_NODE_ADD,
		TYPE_BMESH_TO_MESH,
		TYPE_BMESH_MERGE,

		TYPE_OBJECT_DELETE,
		TYPE_OBJECT_SELECT,
		TYPE_OBJECT_TRANSFORM,
		TYPE_PROFILE_UPDATE,

		TYPE_POSE_CLEAR,
		TYPE_POSE_DEFORM,
		TYPE_POSE_SELECT,

		TYPE_SCULPT_BRUSH,

		TYPE_SKETCH_EDIT,
		TYPE_SKETCH_MAKE,
		TYPE_SKETCH_TO_BASE_MESH,

		TYPE_VIEW_MOVE,
		TYPE_VIEW_ZOOM,
		TYPE_VIEW_ROTATE,
		TYPE_VIEW3D_BOX_EDIT,
		OP_MESH_SMOOTH,
		OP_MESH_DECIMATE,
		OP_MESH_REMESH,
		OP_MESH_SELF_ISECT,
		OP_MESH_BOOLEAN,
		OP_SCULPT_CONFIG,
		OP_MANIPULATOR,
		OP_COMMON_CONFIG,
		TYPE_TOTAL
	};
	Q_ENUMS(OP_TYPE)

	/*metadata for each operator*/
	struct VOperatorType
	{
		typedef std::function<bool()> PollFunc;
		typedef std::function<VOperator*()> CreatorFunc;

		VOperatorType() : pollFunc(nullptr), creatorFunc(nullptr){};

		VOpTemplate::OP_TYPE	type;
		PollFunc				pollFunc;
		CreatorFunc				creatorFunc;

		std::vector<OP_TYPE>	opChain; /*this operator is made by a chain of other operators*/
	};

	static void register_operators();
	static VOperatorType& op_template(OP_TYPE idx);
private:
	static std::vector<VOperatorType*>	op_templates;
};

#endif