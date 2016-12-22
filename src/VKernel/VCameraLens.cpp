#include "VCameraLens.h"

VK_BEGIN_NAMESPACE

VCameraLens::VCameraLens(QObject *parent)
:
  m_projectionType(VCameraLens::OrthographicProjection)
, m_nearPlane(0.1f)
, m_farPlane(1024.0f)
, m_fieldOfView(45.0f)
, m_aspectRatio(1.0f)
, m_left(-0.5f)
, m_right(0.5f)
, m_bottom(-0.5f)
, m_top(0.5f)
{
	updateProjectionMatrix();
}

VCameraLens::VCameraLens(const VCameraLens &o)
	:
	m_projectionType(o.m_projectionType)
	, m_nearPlane(o.m_nearPlane)
	, m_farPlane(o.m_farPlane)
	, m_fieldOfView(o.m_fieldOfView)
	, m_aspectRatio(o.m_aspectRatio)
	, m_left(o.m_left)
	, m_right(o.m_right)
	, m_bottom(o.m_bottom)
	, m_top(o.m_top)
{
	updateProjectionMatrix();
}

VCameraLens::~VCameraLens(){}

/*!
* Sets the lens' projection type \a projectionType.
*
* \note Qt3DCore:: VCameraLens::Frustum and
* Qt3DCore:: VCameraLens::PerspectiveProjection are two different ways of
* specifying the same projection.
*/
void  VCameraLens::setProjectionType( VCameraLens::ProjectionType projectionType)
{
	
	if (m_projectionType != projectionType) {
		m_projectionType = projectionType;
		updateProjectionMatrix();
	}
}

/*!
* Returns the lens' projection type.
*/
 VCameraLens::ProjectionType  VCameraLens::projectionType() const
{
	
	return m_projectionType;
}

/*!
* Defines an orthographic projection based on \a left, \a right, \a bottom, \a
* top, \a nearPlane, \a farPlane.
*/
void  VCameraLens::setOrthographicProjection(float left, float right,
	float bottom, float top,
	float nearPlane, float farPlane)
{
	
	setLeft(left);
	setRight(right);
	setBottom(bottom);
	setTop(top);
	setNearPlane(nearPlane);
	setFarPlane(farPlane);
	setProjectionType(OrthographicProjection);
	updateProjectionMatrix();
}

/*!
* Defines an orthographic projection based on \a left, \a right, \a bottom, \a
* top, \a nearPlane, \a farPlane.
*/
void  VCameraLens::setFrustumProjection(float left, float right,
	float bottom, float top,
	float nearPlane, float farPlane)
{
	
	setLeft(left);
	setRight(right);
	setBottom(bottom);
	setTop(top);
	setNearPlane(nearPlane);
	setFarPlane(farPlane);
	setProjectionType(FrustumProjection);

	updateProjectionMatrix();
}

/*!
* Defines a perspective projection based on \a fieldOfView, \a aspectRatio, \a
* nearPlane, \a farPlane.
*/
void  VCameraLens::setPerspectiveProjection(float fieldOfView, float aspectRatio,
	float nearPlane, float farPlane)
{
	
	setFieldOfView(fieldOfView);
	setAspectRatio(aspectRatio);
	setNearPlane(nearPlane);
	setFarPlane(farPlane);
	setProjectionType(PerspectiveProjection);

	updateProjectionMatrix();
}

/*!
* Sets the projection's near plane to \a nearPlane. This triggers a projection
* matrix update.
*/
void  VCameraLens::setNearPlane(float nearPlane)
{
	
	if (qFuzzyCompare(m_nearPlane, nearPlane))
		return;
	m_nearPlane = nearPlane;

	updateProjectionMatrix();
}

/*!
* Returns the projection's near plane.
*/
float  VCameraLens::nearPlane() const
{
	return m_nearPlane;
}

/*!
* Sets the projection's far plane to \a farPlane. This triggers a projection
* matrix update.
*/
void  VCameraLens::setFarPlane(float farPlane)
{
	
	if (qFuzzyCompare(m_farPlane, farPlane))
		return;
	m_farPlane = farPlane;

	updateProjectionMatrix();
}

/*!
* Returns the projection's far plane.
*/
float  VCameraLens::farPlane() const
{
	
	return m_farPlane;
}

/*!
* Sets the projection's field of view to \a fieldOfView degrees. This triggers
* a projection matrix update.
*
* \note this has no effect if the projection type is not
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
void  VCameraLens::setFieldOfView(float fieldOfView)
{
	
	if (qFuzzyCompare(m_fieldOfView, fieldOfView))
		return;
	m_fieldOfView = fieldOfView;

	updateProjectionMatrix();
}

/*!
* Returns the projection's field of view in degrees.
*
* \note: The return value may be undefined if the projection type is not
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
float  VCameraLens::fieldOfView() const
{
	
	return m_fieldOfView;
}

/*!
* Sets the projection's aspect ratio to \a aspectRatio. This triggers a projection
* matrix update.
*
* \note this has no effect if the projection type is not
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
void  VCameraLens::setAspectRatio(float aspectRatio)
{
	if (qFuzzyCompare(m_aspectRatio, aspectRatio))
		return;
	m_aspectRatio = aspectRatio;

	updateProjectionMatrix();
}

/*!
* Returns the projection's aspect ratio.
*
* \note: The return value may be undefined if the projection type is not
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
float  VCameraLens::aspectRatio() const
{
	return m_aspectRatio;
}

/*!
* Sets the projection's lower left window coordinate to \a left. This
* triggers a projection matrix update.
*
* \note this has no effect if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
void  VCameraLens::setLeft(float left)
{
	if (qFuzzyCompare(m_left, left))
		return;
	m_left = left;

	updateProjectionMatrix();
}

/*!
* Returns the lower left window coordinate of the projection.
*
* \note The return value may be undefined if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
float  VCameraLens::left() const
{
	
	return m_left;
}

/*!
* Sets the projection's upper right window coordinate to \a right. This triggers
* a projection matrix update.
*
* \note this has no effect if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
void  VCameraLens::setRight(float right)
{
	
	if (qFuzzyCompare(m_right, right))
		return;
	m_right = right;

	updateProjectionMatrix();
}

/*!
* Returns the upper right window coordinate of the projection.
*
* \note The return value may be undefined if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
float  VCameraLens::right() const
{
	
	return m_right;
}

/*!
* Sets the projection's bottom window coordinate to \a bottom. This triggers a
* projection matrix update.
*
* \note this has no effect if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
void  VCameraLens::setBottom(float bottom)
{
	
	if (qFuzzyCompare(m_bottom, bottom))
		return;
	m_bottom = bottom;

	updateProjectionMatrix();
}

/*!
* Returns the bottom window coordinate of the projection.
*
* \note The return value may be undefined if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
float  VCameraLens::bottom() const
{
	
	return m_bottom;
}

/*!
* Sets the projection's top window coordinate to \a top. This triggers a
* projection matrix update.
*
* \note this has no effect if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
void  VCameraLens::setTop(float top)
{
	if (qFuzzyCompare(m_top, top))
		return;
	m_top = top;

	updateProjectionMatrix();
}

/*!
* Returns the bottom window coordinate of the projection.
*
* \note The return value may be undefined if the projection type is
* Qt3DCore:: VCameraLens::PerspectiveProjection.
*/
float  VCameraLens::top() const
{
	return m_top;
}

/*!
* Returns the projection matrix.
*/
Matrix4x4 const&  VCameraLens::projectionMatrix() const
{
	
	return m_projectionMatrix;
}

VK_END_NAMESPACE