#include "VTransform.h"

VK_BEGIN_NAMESPACE

VTransform::VTransform()
: m_rotation()
, m_scale(1.0f, 1.0f, 1.0f)
, m_translation()
, m_eulerRotationAngles()
, m_matrixDirty(false)
{
}

VTransform::~VTransform()
{
}


void VTransform::setMatrix(const QMatrix4x4 &m)
{
	if (m != matrix()) {
		m_matrix = m;
		m_matrixDirty = false;

		QVector3D s;
		QVector3D t;
		QQuaternion r;
		decomposeQMatrix4x4(m, t, r, s);
		m_scale = s;
		m_rotation = r;
		m_translation = t;
		m_eulerRotationAngles = m_rotation.toEulerAngles();
	}
}

void VTransform::setRotationX(float rotationX)
{
	if (m_eulerRotationAngles.x() == rotationX)
		return;

	m_eulerRotationAngles.setX(rotationX);
	QQuaternion rotation = QQuaternion::fromEulerAngles(m_eulerRotationAngles);
	if (rotation != m_rotation) {
		m_rotation = rotation;
		m_matrixDirty = true;
	}
}

void VTransform::setRotationY(float rotationY)
{
	if (m_eulerRotationAngles.y() == rotationY)
		return;

	m_eulerRotationAngles.setY(rotationY);
	QQuaternion rotation = QQuaternion::fromEulerAngles(m_eulerRotationAngles);
	if (rotation != m_rotation) {
		m_rotation = rotation;
		m_matrixDirty = true;
	}
}

void VTransform::setRotationZ(float rotationZ)
{
	
	if (m_eulerRotationAngles.z() == rotationZ)
		return;

	m_eulerRotationAngles.setZ(rotationZ);
	QQuaternion rotation = QQuaternion::fromEulerAngles(m_eulerRotationAngles);
	if (rotation != m_rotation) {
		m_rotation = rotation;
		m_matrixDirty = true;
	}
}

QMatrix4x4 VTransform::matrix() const
{
	if (m_matrixDirty) {
		composeQMatrix4x4(m_translation, m_rotation, m_scale, m_matrix);
		m_matrixDirty = false;
	}
	return m_matrix;
}

float VTransform::rotationX() const
{
	
	return m_eulerRotationAngles.x();
}

float VTransform::rotationY() const
{
	
	return m_eulerRotationAngles.y();
}

float VTransform::rotationZ() const
{
	
	return m_eulerRotationAngles.z();
}

void VTransform::setScale3D(const QVector3D &scale)
{
	
	if (scale != m_scale) {
		m_scale = scale;
		m_matrixDirty = true;
	}
}

QVector3D VTransform::scale3D() const
{
	
	return m_scale;
}

void VTransform::setScale(float scale)
{
	if (scale != m_scale.x()) {
		setScale3D(QVector3D(scale, scale, scale));
	}
}

float VTransform::scale() const
{
	
	return m_scale.x();
}

void VTransform::setRotation(const QQuaternion &rotation)
{	
	if (rotation != m_rotation) {
		m_rotation = rotation;
		m_eulerRotationAngles = m_rotation.toEulerAngles();
		m_matrixDirty = true;
	}
}

QQuaternion VTransform::rotation() const
{
	return m_rotation;
}

void VTransform::setTranslation(const QVector3D &translation)
{
	
	if (translation != m_translation) {
		m_translation = translation;
		m_matrixDirty = true;
	}
}

QVector3D VTransform::translation() const
{
	return m_translation;
}

QQuaternion VTransform::fromAxisAndAngle(const QVector3D &axis, float angle)
{
	return QQuaternion::fromAxisAndAngle(axis, angle);
}

QQuaternion VTransform::fromAxisAndAngle(float x, float y, float z, float angle)
{
	return QQuaternion::fromAxisAndAngle(x, y, z, angle);
}

QQuaternion VTransform::fromAxesAndAngles(const QVector3D &axis1, float angle1,
	const QVector3D &axis2, float angle2)
{
	const QQuaternion q1 = QQuaternion::fromAxisAndAngle(axis1, angle1);
	const QQuaternion q2 = QQuaternion::fromAxisAndAngle(axis2, angle2);
	return q2 * q1;
}

QQuaternion VTransform::fromAxesAndAngles(const QVector3D &axis1, float angle1,
	const QVector3D &axis2, float angle2,
	const QVector3D &axis3, float angle3)
{
	const QQuaternion q1 = QQuaternion::fromAxisAndAngle(axis1, angle1);
	const QQuaternion q2 = QQuaternion::fromAxisAndAngle(axis2, angle2);
	const QQuaternion q3 = QQuaternion::fromAxisAndAngle(axis3, angle3);
	return q3 * q2 * q1;
}

QQuaternion VTransform::fromEulerAngles(const QVector3D &eulerAngles)
{
	return QQuaternion::fromEulerAngles(eulerAngles);
}

QQuaternion VTransform::fromEulerAngles(float pitch, float yaw, float roll)
{
	return QQuaternion::fromEulerAngles(pitch, yaw, roll);
}

QMatrix4x4 VTransform::rotateAround(const QVector3D &point, float angle, const QVector3D &axis)
{
	QMatrix4x4 m;
	m.translate(point);
	m.rotate(angle, axis);
	m.translate(-point);
	return m;
}

VK_END_NAMESPACE