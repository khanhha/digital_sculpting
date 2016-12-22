#ifndef VKERNEL_VCAMERA_H
#define VKERNEL_VCAMERA_H
#include "VCameraLens.h"
#include <QObject>
#include <memory>
#include <QMatrix4x4>

VK_BEGIN_NAMESPACE

class VCamera : public QObject
{
	Q_OBJECT
public:
	explicit VCamera(QObject *parent = 0);
	explicit VCamera(const VCamera &cam);

	~VCamera();

	enum CameraTranslationOption {
		TranslateViewCenter,
		DontTranslateViewCenter
	};

	VCameraLens *lens() const;

	Quaternion tiltRotation(float angle) const;
	Quaternion panRotation(float angle) const;
	Quaternion rollRotation(float angle) const;
	Quaternion rotation(float angle, const Vector3 &axis) const;

	// Translate relative to camera orientation axes
	void translate(const Vector3& vLocal, CameraTranslationOption option = TranslateViewCenter);
	void translateViewCenter(const Vector3& vLocal);
	void zoomToConstant(float dst);

	// Translate relative to world axes
	void translateWorld(const Vector3& vWorld, CameraTranslationOption option = TranslateViewCenter);

	void tilt(float angle);
	void pan(float angle);
	void pan(float angle, const Vector3 &axis);
	void roll(float angle);

	void tiltAboutViewCenter(float angle);
	void panAboutViewCenter(float angle);
	void panAboutViewCenter(float angle, const Vector3 &axis);
	void rollAboutViewCenter(float angle);

	void rotate(const Quaternion& q);
	void rotateAboutViewCenter(const Quaternion& q);

	VCameraLens::ProjectionType projectionType() const;
	float nearPlane() const;
	float farPlane() const;
	float fieldOfView() const;
	float aspectRatio() const;
	float left() const;
	float right() const;
	float bottom() const;
	float top() const;
	const Matrix4x4& projectionMatrix() const;
	Matrix4x4 projViewMatrix();

	Vector3 position() const;
	Vector3 upVector() const;
	Vector3 viewCenter() const;
	Vector3 viewVector() const;
	Vector3 rightVector() const;
	const Matrix4x4& viewMatrix() const;

public Q_SLOTS:
	void setProjectionType(VCameraLens::ProjectionType type);
	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);
	void setFieldOfView(float fieldOfView);
	void setAspectRatio(float aspectRatio);
	void setLeft(float left);
	void setRight(float right);
	void setBottom(float bottom);
	void setTop(float top);
	void setOrthographicProjection(float left, float right,
		float bottom, float top,
		float nearPlane, float farPlane);

	void setFrustumProjection(float left, float right,
		float bottom, float top,
		float nearPlane, float farPlane);

	void setPerspectiveProjection(float fieldOfView, float aspect,
		float nearPlane, float farPlane);


	void setPosition(const Vector3 &position);
	void setUpVector(const Vector3 &upVector);
	void setViewCenter(const Vector3 &viewCenter);
	void setFrame(const Vector3 &position, const Vector3 &upVector, const Vector3 &viewCenter);

Q_SIGNALS:
	void cameraChanged(VCamera *camera);
private:
	inline void updateViewMatrix()
	{
		m_viewMatrix.setIdentity();
		lookAt(m_position, m_viewCenter, m_upVector, m_viewMatrix);
	}

	inline void lookAt(const Vector3 &eye, const Vector3 &center, const Vector3 &up, Matrix4x4 &Result)
	{
		Vector3 const f((center - eye).normalized());
		Vector3 const s(f.cross(up).normalized());
		Vector3 const u(s.cross(f));

#if 0
		Result(0, 0) = s.x();
		Result(1, 0) = s.y();
		Result(2, 0) = s.z();
		Result(0, 1) = u.x();
		Result(1, 1) = u.y();
		Result(2, 1) = u.z();
		Result(0, 2) = -f.x();
		Result(1, 2) = -f.y();
		Result(2, 2) = -f.z();
		Result(3, 0) = -s.dot(eye);
		Result(3, 1) = -u.dot(eye);
		Result(3, 2) = f.dot(eye);
#else
		Result.setIdentity();
		Result(0, 0) = s.x();
		Result(0, 1) = s.y();
		Result(0, 2) = s.z();

		Result(1, 0) = u.x();
		Result(1, 1) = u.y();
		Result(1, 2) = u.z();
		
		Result(2, 0) = -f.x();
		Result(2, 1) = -f.y();
		Result(2, 2) = -f.z();
		
		Result(0, 3)  = -s.dot(eye);
		Result(1, 3)  = -u.dot(eye);
		Result(2, 3)  =  f.dot(eye);
#if COMPARE_QT_LOOKAT
		QMatrix4x4 mat; mat.setToIdentity();
		mat.lookAt(
			QVector3D(eye[0], eye[1], eye[2]),
			QVector3D(center[0], center[1], center[2]), QVector3D(up[0], up[1], up[2]));

		QVector4D testp(0.0f, 0.0f, -1.0f, 1.0f);
		QVector4D out = mat * testp;
#endif
#endif

	}
private:
	Vector3 m_position;
	Vector3 m_viewCenter;
	Vector3 m_upVector;

	Vector3		m_cameraToCenter; // The vector from the camera position to the view center
	bool		m_viewMatrixDirty;

	VCameraLens *m_lens;

	Matrix4x4   m_viewMatrix;
};

VK_END_NAMESPACE
#endif