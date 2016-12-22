#ifndef VTRANSFORM_H
#define VTRANSFORM_H

#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>

#include "VKernelCommon.h"

VK_BEGIN_NAMESPACE

class VTransform
{

public:
public:
	explicit VTransform();
	~VTransform();

	float scale() const;
	QVector3D scale3D() const;
	QQuaternion rotation() const;
	QVector3D translation() const;

	static QQuaternion fromAxisAndAngle(const QVector3D &axis, float angle);
	static QQuaternion fromAxisAndAngle(float x, float y, float z, float angle);

	static QQuaternion fromAxesAndAngles(const QVector3D &axis1, float angle1,
		const QVector3D &axis2, float angle2);
	static QQuaternion fromAxesAndAngles(const QVector3D &axis1, float angle1,
		const QVector3D &axis2, float angle2,
		const QVector3D &axis3, float angle3);

	static QQuaternion fromEulerAngles(const QVector3D &eulerAngles);
	static QQuaternion fromEulerAngles(float pitch, float yaw, float roll);

	static QMatrix4x4 rotateAround(const QVector3D &point, float angle, const QVector3D &axis);

	QMatrix4x4 matrix() const;

	float rotationX() const;
	float rotationY() const;
	float rotationZ() const;

public:
	void setScale(float scale);
	void setScale3D(const QVector3D &scale);
	void setRotation(const QQuaternion &rotation);
	void setTranslation(const QVector3D &translation);
	void setMatrix(const QMatrix4x4 &matrix);

	void setRotationX(float rotationX);
	void setRotationY(float rotationY);
	void setRotationZ(float rotationZ);

private:
	inline void composeQMatrix4x4(const QVector3D &position, const QQuaternion &orientation, const QVector3D &scale, QMatrix4x4 &m) const ;
	inline void decomposeQMatrix3x3(const QMatrix3x3 &m, QMatrix3x3 &Q, QVector3D &D, QVector3D &U) const;
	inline bool hasScale(const QMatrix4x4 &m) const;
	inline void decomposeQMatrix4x4(const QMatrix4x4 &m, QVector3D &position, QQuaternion &orientation, QVector3D &scale) const;

private:
	// Stored in this order as QQuaternion is bigger than QVector3D
	// Operations are applied in the order of:
	// scale, rotation, translation
	QQuaternion m_rotation;
	QVector3D	m_scale;
	QVector3D	m_translation;
	QVector3D	m_eulerRotationAngles;

	mutable QMatrix4x4 m_matrix;
	mutable bool m_matrixDirty;
};

void VTransform::composeQMatrix4x4(const QVector3D &position, const QQuaternion &orientation, const QVector3D &scale, QMatrix4x4 &m) const
{
	const QMatrix3x3 rot3x3(orientation.toRotationMatrix());

	// set up final matrix with scale, rotation and translation
	m(0, 0) = scale.x() * rot3x3(0, 0); m(0, 1) = scale.y() * rot3x3(0, 1); m(0, 2) = scale.z() * rot3x3(0, 2); m(0, 3) = position.x();
	m(1, 0) = scale.x() * rot3x3(1, 0); m(1, 1) = scale.y() * rot3x3(1, 1); m(1, 2) = scale.z() * rot3x3(1, 2); m(1, 3) = position.y();
	m(2, 0) = scale.x() * rot3x3(2, 0); m(2, 1) = scale.y() * rot3x3(2, 1); m(2, 2) = scale.z() * rot3x3(2, 2); m(2, 3) = position.z();
	// no projection term
	m(3, 0) = 0.0f; m(3, 1) = 0.0f; m(3, 2) = 0.0f; m(3, 3) = 1.0f;
}

void VTransform::decomposeQMatrix3x3(const QMatrix3x3 &m, QMatrix3x3 &Q, QVector3D &D, QVector3D &U) const
{
	// Factor M = QR = QDU where Q is orthogonal, D is diagonal,
	// and U is upper triangular with ones on its diagonal.
	// Algorithm uses Gram-Schmidt orthogonalization (the QR algorithm).
	//
	// If M = [ m0 | m1 | m2 ] and Q = [ q0 | q1 | q2 ], then
	//   q0 = m0/|m0|
	//   q1 = (m1-(q0*m1)q0)/|m1-(q0*m1)q0|
	//   q2 = (m2-(q0*m2)q0-(q1*m2)q1)/|m2-(q0*m2)q0-(q1*m2)q1|
	//
	// where |V| indicates length of vector V and A*B indicates dot
	// product of vectors A and B.  The matrix R has entries
	//
	//   r00 = q0*m0  r01 = q0*m1  r02 = q0*m2
	//   r10 = 0      r11 = q1*m1  r12 = q1*m2
	//   r20 = 0      r21 = 0      r22 = q2*m2
	//
	// so D = diag(r00,r11,r22) and U has entries u01 = r01/r00,
	// u02 = r02/r00, and u12 = r12/r11.

	// Q = rotation
	// D = scaling
	// U = shear

	// D stores the three diagonal entries r00, r11, r22
	// U stores the entries U[0] = u01, U[1] = u02, U[2] = u12

	// build orthogonal matrix Q
	float invLen = 1.0f / std::sqrt(m(0, 0) * m(0, 0) + m(1, 0) * m(1, 0) + m(2, 0) * m(2, 0));
	Q(0, 0) = m(0, 0) * invLen;
	Q(1, 0) = m(1, 0) * invLen;
	Q(2, 0) = m(2, 0) * invLen;

	float dot = Q(0, 0) * m(0, 1) + Q(1, 0) * m(1, 1) + Q(2, 0) * m(2, 1);
	Q(0, 1) = m(0, 1) - dot * Q(0, 0);
	Q(1, 1) = m(1, 1) - dot * Q(1, 0);
	Q(2, 1) = m(2, 1) - dot * Q(2, 0);
	invLen = 1.0f / std::sqrt(Q(0, 1) * Q(0, 1) + Q(1, 1) * Q(1, 1) + Q(2, 1) * Q(2, 1));
	Q(0, 1) *= invLen;
	Q(1, 1) *= invLen;
	Q(2, 1) *= invLen;

	dot = Q(0, 0) * m(0, 2) + Q(1, 0) * m(1, 2) + Q(2, 0) * m(2, 2);
	Q(0, 2) = m(0, 2) - dot * Q(0, 0);
	Q(1, 2) = m(1, 2) - dot * Q(1, 0);
	Q(2, 2) = m(2, 2) - dot * Q(2, 0);
	dot = Q(0, 1) * m(0, 2) + Q(1, 1) * m(1, 2) + Q(2, 1) * m(2, 2);
	Q(0, 2) -= dot * Q(0, 1);
	Q(1, 2) -= dot * Q(1, 1);
	Q(2, 2) -= dot * Q(2, 1);
	invLen = 1.0f / std::sqrt(Q(0, 2) * Q(0, 2) + Q(1, 2) * Q(1, 2) + Q(2, 2) * Q(2, 2));
	Q(0, 2) *= invLen;
	Q(1, 2) *= invLen;
	Q(2, 2) *= invLen;

	// guarantee that orthogonal matrix has determinant 1 (no reflections)
	const float det = Q(0, 0) * Q(1, 1) * Q(2, 2) + Q(0, 1) * Q(1, 2) * Q(2, 0) +
		Q(0, 2) * Q(1, 0) * Q(2, 1) - Q(0, 2) * Q(1, 1) * Q(2, 0) -
		Q(0, 1) * Q(1, 0) * Q(2, 2) - Q(0, 0) * Q(1, 2) * Q(2, 1);
	if (det < 0.0f)
		Q *= -1.0f;

	// build "right" matrix R
	QMatrix3x3 R(Qt::Uninitialized);
	R(0, 0) = Q(0, 0) * m(0, 0) + Q(1, 0) * m(1, 0) + Q(2, 0) * m(2, 0);
	R(0, 1) = Q(0, 0) * m(0, 1) + Q(1, 0) * m(1, 1) + Q(2, 0) * m(2, 1);
	R(1, 1) = Q(0, 1) * m(0, 1) + Q(1, 1) * m(1, 1) + Q(2, 1) * m(2, 1);
	R(0, 2) = Q(0, 0) * m(0, 2) + Q(1, 0) * m(1, 2) + Q(2, 0) * m(2, 2);
	R(1, 2) = Q(0, 1) * m(0, 2) + Q(1, 1) * m(1, 2) + Q(2, 1) * m(2, 2);
	R(2, 2) = Q(0, 2) * m(0, 2) + Q(1, 2) * m(1, 2) + Q(2, 2) * m(2, 2);

	// the scaling component
	D[0] = R(0, 0);
	D[1] = R(1, 1);
	D[2] = R(2, 2);

	// the shear component
	U[0] = R(0, 1) / D[0];
	U[1] = R(0, 2) / D[0];
	U[2] = R(1, 2) / D[1];
}

bool VTransform::hasScale(const QMatrix4x4 &m) const
{
	// If the columns are orthonormal and form a right-handed system, then there is no scale
	float t(m.determinant());
	if (!qFuzzyIsNull(t - 1.0f))
		return true;
	t = m(0, 0) * m(0, 0) + m(1, 0) * m(1, 0) + m(2, 0) * m(2, 0);
	if (!qFuzzyIsNull(t - 1.0f))
		return true;
	t = m(0, 1) * m(0, 1) + m(1, 1) * m(1, 1) + m(2, 1) * m(2, 1);
	if (!qFuzzyIsNull(t - 1.0f))
		return true;
	t = m(0, 2) * m(0, 2) + m(1, 2) * m(1, 2) + m(2, 2) * m(2, 2);
	if (!qFuzzyIsNull(t - 1.0f))
		return true;
	return false;
}

void VTransform::decomposeQMatrix4x4(const QMatrix4x4 &m, QVector3D &position, QQuaternion &orientation, QVector3D &scale) const
{
	Q_ASSERT(m.isAffine());

	const QMatrix3x3 m3x3(m.toGenericMatrix<3, 3>());

	QMatrix3x3 rot3x3(Qt::Uninitialized);
	if (hasScale(m)) {
		decomposeQMatrix3x3(m3x3, rot3x3, scale, position);
	}
	else {
		// we know there is no scaling part; no need for QDU decomposition
		scale = QVector3D(1.0f, 1.0f, 1.0f);
		rot3x3 = m3x3;
	}
	orientation = QQuaternion::fromRotationMatrix(rot3x3);
	position = QVector3D(m(0, 3), m(1, 3), m(2, 3));
}

VK_END_NAMESPACE

#endif