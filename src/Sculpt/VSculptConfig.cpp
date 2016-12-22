#include "VSculptConfig.h"

VSculptConfig::VSculptConfig()
{
	_brush_confs.resize(VbsDef::BRUSH_TOTAL);
	_brush = VbsDef::BRUSH_DRAW;
}

VSculptConfig::~VSculptConfig()
{
}

void VSculptConfig::setBrush(VbsDef::BRUSH brush)
{
	BLI_assert(brush >= 0 && brush < VbsDef::BRUSH_TOTAL);
	_brush = brush;
}
void VSculptConfig::setBrushStrength(float strength)
{
	_brush_confs[_brush].strength = strength;
}
void VSculptConfig::setBrushSpacing(float spacing)
{
	_brush_confs[_brush].spacing = spacing;
}
void VSculptConfig::setBrushPixelSize(int size)
{
	_brush_confs[_brush].pixel_size= size;
}
void VSculptConfig::setBrushDynamicTopology(bool topo)
{
	_brush_confs[_brush].dynamic_topo = topo;
}
void VSculptConfig::setFalloffCurve(VbsDef::CURVE type)
{
	_brush_confs[_brush].falloff = type;
}

void VSculptConfig::setBrushDetailsize(float detail)
{
	_brush_confs[_brush].detail_size = detail;
}

VbsDef::BRUSH VSculptConfig::brush()
{
	return _brush;
}

float VSculptConfig::brushStrength()
{
	return _brush_confs[_brush].strength;
}

float VSculptConfig::brushSpacing()
{
	return _brush_confs[_brush].spacing;
}

float VSculptConfig::brushPixelSize()
{
	return _brush_confs[_brush].pixel_size;
}

float VSculptConfig::brushDynamicTopology()
{
	return _brush_confs[_brush].dynamic_topo;
}

VbsDef::CURVE VSculptConfig::brushFalloffCurve()
{
	return _brush_confs[_brush].falloff;
}

float VSculptConfig::brushDetailsize()
{
	return _brush_confs[_brush].detail_size;
}
