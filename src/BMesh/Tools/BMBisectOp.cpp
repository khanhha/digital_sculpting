#include "Tools/BMBisectOp.h"

VM_BEGIN_NAMESPACE


BMBisectOp::BMBisectOp()
{
}

BMBisectOp::~BMBisectOp()
{
}

void BMBisectOp::execute()
{
	const Vector3f &plane_co = *in_slot_data_get<Vector3f>("plane_co");
	const Vector3f &plane_no = *in_slot_data_get<Vector3f>("plane_no");
}

VM_END_NAMESPACE