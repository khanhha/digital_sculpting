#ifndef VSCULPT_VVIEW3D_EDIT_BOX_H
#define VSCULPT_VVIEW3D_EDIT_BOX_H
#include "VOperator.h"
class VView3DEditBoxOp : public VOperator
{
public:
	VView3DEditBoxOp();
	virtual ~VView3DEditBoxOp();
	static bool poll();
	static VOperator* create();

	virtual int  invoke(QEvent *ev);
	virtual int  modal(QEvent *ev);
private:
	void init();
	void apply();
	Eigen::Vector3f _rotaxis_view;
	float	 _rotangle;
	float	 _rotanglestep;
	bool	 _done;
	int		 _timerid;
	VView3DRegion		*_region;
	Eigen::Vector2f		 _pos;
};


#endif