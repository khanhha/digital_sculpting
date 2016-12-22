#ifndef BMESH_BMESH_SYMMETRIZE_H
#define BMESH_BMESH_SYMMETRIZE_H
#include "BMesh/BMesh.h"
VM_BEGIN_NAMESPACE
class BMeshSymmetrize
{
	enum
	{
		ELEM_CENTER = 1 << 0
	};
public:
	BMeshSymmetrize(BMesh *bm, Vector3f plane_co, Vector3f plane_no, bool side);
	~BMeshSymmetrize();
	void execute();
private:
	void clear_flag(BMesh *mesh);
	void mirror_clone();
private:
	BMesh	*_bm;
	Vector3f _plane_co;
	Vector3f _plane_no;
	Vector4f _plane;
	bool	 _side;
};
VM_END_NAMESPACE
#endif