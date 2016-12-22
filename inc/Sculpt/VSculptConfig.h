#ifndef SCULPT_SCULPT_CONFIG_H
#define SCULPT_SCULPT_CONFIG_H
#include "BezierCurve.h"
#include "commonDefine.h"
#include "BaseLib/UtilMacro.h"
#include "VbsQt/VbsDef.h"

class VSculptConfig
{
	struct BrushConfig{
		BrushConfig() :
			strength(0.5f),
			spacing(10.0f),
			pixel_size(50),
			detail_size(5.0),
			dynamic_topo(false),
			falloff(VbsDef::CURVE_SMOOTH)
		{}

		float strength;
		float spacing;
		int	  pixel_size;
		float detail_size;
		bool  dynamic_topo;
		VbsDef::CURVE falloff;
	};

public:
	VSculptConfig();
	~VSculptConfig();
	void setBrush(VbsDef::BRUSH);
	void setBrushStrength(float strength);
	void setBrushSpacing(float spacing);
	void setBrushPixelSize(int size);
	void setBrushDynamicTopology(bool topo);
	void setFalloffCurve(VbsDef::CURVE type);
	void setBrushDetailsize(float detail);
	
	VbsDef::BRUSH  brush();
	float brushStrength();
	float brushSpacing();
	float brushPixelSize();
	float brushDynamicTopology();
	float brushDetailsize();
	VbsDef::CURVE brushFalloffCurve();

private:
	std::vector<BrushConfig> _brush_confs;
	VbsDef::BRUSH	_brush; /*current brush*/
};
#endif