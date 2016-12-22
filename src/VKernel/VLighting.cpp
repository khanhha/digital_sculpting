#include "VLighting.h"
#include <boost/math/constants/constants.hpp>

VK_BEGIN_NAMESPACE

VLighting::VLighting()
	:
	_lightPosition(0.0f, 0.0f, 10.0f, 1.0f),
	_lightDirection(-1.0f, -1.0f, -1.0f, 0.0f),
	_ambient({ 1.0f, 1.0f, 1.0f, 1.0f }),
	_diffuse({ 1.0f, 1.0f, 1.0f, 1.0f }),
	_specular({ 1.0f, 1.0f, 1.0f, 1.0f }),
	_cutoff({ boost::math::constants::half_pi<float>(), 0.0f, 1.0f, 1.0f }),
	_attenuation({ 1.0f, 0.0f, 0.0f, 1.0f })
{
}

VLighting::~VLighting()
{
}

Vector4 VLighting::lightPosition() const
{
	return worldTransform() * _lightPosition;
}

Vector4 VLighting::lightDirection() const
{
	return worldTransform() * _lightDirection;
}

Vector4 VLighting::lightUp() const
{
	return worldTransform() * _lightUp;
}

Vector4 VLighting::lightRight() const
{
	return worldTransform() * _lightRight;
}


void VLighting::setLightPosition(const Vector4 &pos)
{
	_lightPosition = pos;
	emit lightingChanged(this);
}

void VLighting::setLightDirection(const Vector4 &dir)
{
	_lightDirection = dir;
	emit lightingChanged(this);
}

void VLighting::setLightUp(const Vector4 &up)
{
	_lightUp = up;
	emit lightingChanged(this);
}

void VLighting::setLightRight(const Vector4 &right)
{
	_lightRight = right;
	emit lightingChanged(this);
}

void VLighting::setAmbient(const Vector4 &val)
{	
	_ambient = val;
	emit lightingChanged(this);
}
void VLighting::setDiffuse(const Vector4 &val)
{
	_diffuse = val;
	emit lightingChanged(this);
}
void VLighting::setSpecular(const Vector4 &val)
{
	_specular = val;
	emit lightingChanged(this);
}
void VLighting::setCutoff(const Vector4 &val)
{
	_cutoff = val;
	emit lightingChanged(this);
}
void VLighting::setAttenuation(const Vector4 &val)
{
	_attenuation = val;
	emit lightingChanged(this);
}
VK_END_NAMESPACE