#include "VCamera.h"

VK_BEGIN_NAMESPACE

VCamera::VCamera(QObject *parent)
: QObject(parent)
, m_position(0.0f, 0.0f, 0.0f)
, m_viewCenter(0.0f, 0.0f, -100.0f)
, m_upVector(0.0f, 1.0f, 0.0f)
, m_cameraToCenter(m_viewCenter - m_position)
, m_viewMatrixDirty(false)
, m_lens(new VCameraLens())
{
	updateViewMatrix();
}

VCamera::VCamera(const VCamera &cam)
	:
m_position(cam.m_position)
, m_viewCenter(cam.m_viewCenter)
, m_upVector(cam.m_upVector)
, m_cameraToCenter(cam.m_cameraToCenter)
, m_viewMatrixDirty(cam.m_viewMatrixDirty)
, m_lens(new VCameraLens(*cam.lens()))
{
	updateViewMatrix();
}

VCamera::~VCamera()
{
	if (m_lens) delete m_lens;
}

VCameraLens *VCamera::lens() const
{
	
	return m_lens;
}

void VCamera::translate(const Vector3 &vLocal, CameraTranslationOption option)
{
	Vector3 viewVector = viewCenter() - position(); // From "camera" position to view center

	// Calculate the amount to move by in world coordinates
	Vector3 vWorld; vWorld.setZero();
	if (!qFuzzyIsNull(vLocal.x())) {
		// Calculate the vector for the local x axis
		const Vector3 x = viewVector.cross(upVector()).normalized();
		vWorld += vLocal.x() * x;
	}

	if (!qFuzzyIsNull(vLocal.y()))
		vWorld += vLocal.y() * upVector();

	if (!qFuzzyIsNull(vLocal.z()))
		vWorld += vLocal.z() * viewVector.normalized();

	// Update the camera position using the calculated world vector
	setPosition(position() + vWorld);

	// May be also update the view center coordinates
	if (option == TranslateViewCenter)
		setViewCenter(viewCenter() + vWorld);

	// Refresh the camera -> view center vector
	viewVector = viewCenter() - position();

	// Calculate a new up vector. We do this by:
	// 1) Calculate a new local x-direction vector from the cross product of the new
	//    camera to view center vector and the old up vector.
	// 2) The local x vector is the normal to the plane in which the new up vector
	//    must lay. So we can take the cross product of this normal and the new
	//    x vector. The new normal vector forms the last part of the orthonormal basis
	const Vector3 x = viewVector.cross(upVector()).normalized();
	setUpVector(x.cross(viewVector).normalized());
}

void VCamera::translateViewCenter(const Vector3& vLocal)
{
	setViewCenter(viewCenter() + vLocal);

	// Refresh the camera -> view center vector
	Vector3 viewVector = viewCenter() - position();

	// Calculate a new up vector. We do this by:
	// 1) Calculate a new local x-direction vector from the cross product of the new
	//    camera to view center vector and the old up vector.
	// 2) The local x vector is the normal to the plane in which the new up vector
	//    must lay. So we can take the cross product of this normal and the new
	//    x vector. The new normal vector forms the last part of the orthonormal basis
	const Vector3 x = viewVector.cross(upVector()).normalized();
	setUpVector(x.cross(viewVector).normalized());
}

void VCamera::zoomToConstant(float dst)
{
	Vector3 dir = m_cameraToCenter.normalized();
	setPosition(m_viewCenter - dir * dst);
}

void VCamera::translateWorld(const Vector3 &vWorld, CameraTranslationOption option)
{
	// Update the camera position using the calculated world vector
	setPosition(position() + vWorld);

	// May be also update the view center coordinates
	if (option == TranslateViewCenter)
		setViewCenter(viewCenter() + vWorld);
}

Quaternion VCamera::tiltRotation(float angle) const
{
	const Vector3 viewVector = viewCenter() - position();
	const Vector3 xBasis = upVector().cross(viewVector.normalized()).normalized();
	return Quaternion(AngleAxis(-angle, xBasis));
}

Quaternion VCamera::panRotation(float angle) const
{
	return Quaternion(AngleAxis(angle, upVector()));
}

Quaternion VCamera::rollRotation(float angle) const
{
	Vector3 viewVector = (viewCenter() - position()).normalized();
	return Quaternion(AngleAxis(-angle, viewVector));
}

Quaternion VCamera::rotation(float angle, const Vector3 &axis) const
{
	return Quaternion(AngleAxis(angle, axis));
}

void VCamera::tilt(float angle)
{
	Quaternion q = tiltRotation(angle);
	rotate(q);
}

void VCamera::pan(float angle)
{
	Quaternion q = panRotation(-angle);
	rotate(q);
}

void VCamera::pan(float angle, const Vector3 &axis)
{
	Quaternion q = rotation(-angle, axis);
	rotate(q);
}

void VCamera::roll(float angle)
{
	Quaternion q = rollRotation(-angle);
	rotate(q);
}

void VCamera::tiltAboutViewCenter(float angle)
{
	Quaternion q = tiltRotation(-angle);
	rotateAboutViewCenter(q);
}

void VCamera::panAboutViewCenter(float angle)
{
	Quaternion q = panRotation(angle);
	rotateAboutViewCenter(q);
}

void VCamera::panAboutViewCenter(float angle, const Vector3 &axis)
{
	Quaternion q = rotation(angle, axis);
	rotateAboutViewCenter(q);
}

void VCamera::rollAboutViewCenter(float angle)
{
	Quaternion q = rollRotation(angle);
	rotateAboutViewCenter(q);
}

void VCamera::rotate(const Quaternion& q)
{
	setUpVector(q * upVector());
	Vector3 viewVector = viewCenter() - position();
	Vector3 cameraToCenter = q * viewVector;
	setViewCenter(position() + cameraToCenter);
}

void VCamera::rotateAboutViewCenter(const Quaternion& q)
{
	setUpVector(q * upVector());
	Vector3 viewVector = viewCenter() - position();
	Vector3 cameraToCenter = q * viewVector;
	setPosition(viewCenter() - cameraToCenter);
	setViewCenter(position() + cameraToCenter);
}

/*!
\qmlproperty enumeration Qt3DCore::Camera::projectionType

Holds the type of the camera projection (orthogonal or perspective).

\value CameraLens.OrthographicProjection Orthographic projection
\value CameraLens.PerspectiveProjection Perspective projection
*/
VCameraLens::ProjectionType VCamera::projectionType() const
{
	
	return m_lens->projectionType();
}

void VCamera::setProjectionType(VCameraLens::ProjectionType type)
{
	m_lens->setProjectionType(type);
	emit cameraChanged(this);
}

void VCamera::setNearPlane(float nearPlane)
{
	m_lens->setNearPlane(nearPlane);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::nearPlane
*/
float VCamera::nearPlane() const
{	
	return m_lens->nearPlane();
}

void VCamera::setFarPlane(float farPlane)
{
	m_lens->setFarPlane(farPlane);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::farPlane
*/
float VCamera::farPlane() const
{	
	return m_lens->farPlane();
}

void VCamera::setFieldOfView(float fieldOfView)
{
	m_lens->setFieldOfView(fieldOfView);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::fieldOfView
*/
float VCamera::fieldOfView() const
{
	return m_lens->fieldOfView();
}

void VCamera::setAspectRatio(float aspectRatio)
{
	m_lens->setAspectRatio(aspectRatio);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::aspectRatio
*/
float VCamera::aspectRatio() const
{
	return m_lens->aspectRatio();

}

void VCamera::setLeft(float left)
{
	m_lens->setLeft(left);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::left
*/
float VCamera::left() const
{
	return m_lens->left();
}

void VCamera::setRight(float right)
{
	m_lens->setRight(right);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::right
*/
float VCamera::right() const
{
	return m_lens->right();
}

void VCamera::setBottom(float bottom)
{
	m_lens->setBottom(bottom);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::bottom
*/
float VCamera::bottom() const
{
	return m_lens->bottom();
}

void VCamera::setTop(float top)
{
	m_lens->setTop(top);
	emit cameraChanged(this);
}

/*!
\qmlproperty float Qt3DCore::Camera::top
*/
float VCamera::top() const
{
	return m_lens->top();
}

/*!
\qmlproperty matrix4x4 Qt3DCore::Camera::projectionMatrix
\readonly
*/
const Matrix4x4& VCamera::projectionMatrix() const
{
	return m_lens->projectionMatrix();
}

Matrix4x4 VCamera::projViewMatrix()
{
	return m_lens->projectionMatrix() * viewMatrix();
}

void VCamera::setPosition(const Vector3 &position)
{
	m_position = position;
	m_cameraToCenter = m_viewCenter - position;
	m_viewMatrixDirty = true;
	updateViewMatrix();
	emit cameraChanged(this);
}

/*!
\qmlproperty vector3d Qt3DCore::Camera::position
*/
Vector3 VCamera::position() const
{
	return m_position;
}

void VCamera::setUpVector(const Vector3 &upVector)
{
	m_upVector = upVector;
	m_viewMatrixDirty = true;

	updateViewMatrix();
	emit cameraChanged(this);
}

/*!
\qmlproperty vector3d Qt3DCore::Camera::upVector
*/
Vector3 VCamera::upVector() const
{
	return m_upVector;
}

void VCamera::setViewCenter(const Vector3 &viewCenter)
{
	m_viewCenter = viewCenter;
	m_cameraToCenter = viewCenter - m_position;
	m_viewMatrixDirty = true;

	updateViewMatrix();
	emit cameraChanged(this);
}


void VCamera::setFrame(const Vector3 &position, const Vector3 &upVector, const Vector3 &viewCenter)
{
	m_position = position;
	m_upVector = upVector;
	m_viewCenter = viewCenter;
	m_cameraToCenter = viewCenter - m_position;
	m_viewMatrixDirty = true;

	updateViewMatrix();
	emit cameraChanged(this);
}


/*!
* Defines an orthographic projection based on \a left, \a right, \a bottom, \a
* top, \a nearPlane, \a farPlane.
*/
void  VCamera::setOrthographicProjection(float left, float right,
	float bottom, float top,
	float nearPlane, float farPlane)
{
	m_lens->setOrthographicProjection(left, right, bottom, top, nearPlane, farPlane);
}

/*!
* Defines an orthographic projection based on \a left, \a right, \a bottom, \a
* top, \a nearPlane, \a farPlane.
*/
void  VCamera::setFrustumProjection(float left, float right,
	float bottom, float top,
	float nearPlane, float farPlane)
{
	m_lens->setFrustumProjection(left, right, bottom, top, nearPlane, farPlane);
}

/*!
* Defines a perspective projection based on \a fieldOfView, \a aspectRatio, \a
* nearPlane, \a farPlane.
*/
void  VCamera::setPerspectiveProjection(float fieldOfView, float aspectRatio,
	float nearPlane, float farPlane)
{
	m_lens->setPerspectiveProjection(fieldOfView, aspectRatio, nearPlane, farPlane);
}


/*!
\qmlproperty vector3d Qt3DCore::Camera::viewCenter
*/
Vector3 VCamera::viewCenter() const
{
	
	return m_viewCenter;
}

/*!
\qmlproperty vector3d Qt3DCore::Camera::viewVector
*/
Vector3 VCamera::viewVector() const
{
	return m_cameraToCenter;
}

Vector3 VCamera::rightVector() const
{
	const Vector3 viewVector = viewCenter() - position();
	const Vector3 xBasis = upVector().cross(viewVector.normalized()).normalized();
	return xBasis;
}

/*!
\qmlproperty matrix4x4 Qt3DCore::Camera::viewMatrix
*/
const Matrix4x4& VCamera::viewMatrix() const
{
	return m_viewMatrix;
}
VK_END_NAMESPACE