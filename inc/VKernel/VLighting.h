#ifndef VKERNEL_VLIGHTING_H
#define VKERNEL_VLIGHTING_H

#include "VKernelCommon.h"
#include <GTGraphics.h>
#include "VNode.h"

VK_BEGIN_NAMESPACE

struct VLighting : public VNode
{
	Q_OBJECT
public:
	VLighting();
	~VLighting();
public:
	Vector4			lightPosition() const;
	Vector4			lightDirection() const;
	Vector4			lightUp() const;
	Vector4			lightRight() const;
	const Vector4&			ambient() const;
	const Vector4&			diffuse() const;
	const Vector4&			specular() const;
	const Vector4&			cutoff() const;
	const Vector4&			attenuation() const;
public Q_SLOTS:
	void setLightPosition(const Vector4 &pos);
	void setLightDirection(const Vector4 &dir);
	void setLightUp(const Vector4 &up);
	void setLightRight(const Vector4 &right);
	void setAmbient(const Vector4 &val);
	void setDiffuse(const Vector4 &val);
	void setSpecular(const Vector4 &val);
	void setCutoff(const Vector4 &val);
	void setAttenuation(const Vector4 &val);
Q_SIGNALS:
	void lightingChanged(VLighting*);
private:
	Vector4			_lightPosition;      // default: (0,0,0,1)
	Vector4			_lightDirection;     // default: (0,0,-1,0)
	Vector4			_lightUp;            // default: (0,1,0,0)
	Vector4			_lightRight;         // default: (1,0,0,0)

	Vector4			_ambient;
	Vector4			_diffuse;
	Vector4			_specular;
	Vector4			_cutoff;
	Vector4			_attenuation;
};

VK_END_NAMESPACE

#endif