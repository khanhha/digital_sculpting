#ifndef SCULPT_BEZIER_CURVE_H
#define SCULPT_BEZIER_CURVE_H
#include <iostream>
#include <fstream>
#include "BaseLib/Point3Dd.h"
#include "VbsQt/VbsDef.h"

class BezierCurve
{
public:
	struct CurvePoint
	{
		double x;
		double y;
	};

	BezierCurve(VbsDef::CURVE type = VbsDef::CURVE_SMOOTH);
	~BezierCurve();

	double evaluate(const double par);
	void render();
	void reset(VbsDef::CURVE type);
	void dumpCurve();

private:
	bool setControlPoints(int type);
	void makeTable(int resolution);

private:
	int _resolution;
	int _totalPoint;
	float _step;
	VbsDef::CURVE  _type;
	CurvePoint		*_cpoints;
	CurvePoint		*_table;
};
#endif