#ifndef VKERNEL_VCAMERA_LENS_H
#define VKERNEL_VCAMERA_LENS_H

#include <QMatrix4x4>
#include <QObject>
#include "VKernelCommon.h"

VK_BEGIN_NAMESPACE

class  VCameraLens : public QObject
{
public:
	enum ProjectionType {
		OrthographicProjection,
		PerspectiveProjection,
		FrustumProjection
	};
public:
	explicit VCameraLens(QObject *parent = 0);
	VCameraLens(const VCameraLens &o);
	~VCameraLens();

	ProjectionType projectionType() const;
	float nearPlane() const;
	float farPlane() const;
	float fieldOfView() const;
	float aspectRatio() const;
	float left() const;
	float right() const;
	float bottom() const;
	float top() const;

	Matrix4x4 const& projectionMatrix() const;

	void setOrthographicProjection(float left, float right,
		float bottom, float top,
		float nearPlane, float farPlane);

	void setFrustumProjection(float left, float right,
		float bottom, float top,
		float nearPlane, float farPlane);

	void setPerspectiveProjection(float fieldOfView, float aspect,
		float nearPlane, float farPlane);

	void setProjectionType(ProjectionType projectionType);
	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);
	void setFieldOfView(float fieldOfView);
	void setAspectRatio(float aspectRatio);
	void setLeft(float left);
	void setRight(float right);
	void setBottom(float bottom);
	void setTop(float top);

private:
	inline void updateProjectionMatrix()
	{
		switch (m_projectionType) {
		case VCameraLens::OrthographicProjection:
			updateOrthographicProjection();
			break;
		case VCameraLens::PerspectiveProjection:
			updatePerpectiveProjection();
			break;
		case VCameraLens::FrustumProjection:
			updateFrustumProjection();
			break;
		}
	}

private:
	inline void updatePerpectiveProjection()
	{
		perspectiveRH(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane, m_projectionMatrix);
	}

	inline void updateOrthographicProjection()
	{
		ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane, m_projectionMatrix);
	}

	inline void updateFrustumProjection()
	{
		frustum(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane, m_projectionMatrix);
	}


	/*take from glm::frustum*/
	inline void frustum(float left, float right, float bottom, float top, float nearVal, float farVal, Matrix4x4 &Result)
	{
		Result.setZero();

		Result(0, 0) = (static_cast<float>(2) * nearVal) / (right - left);
		Result(1, 1) = (static_cast<float>(2) * nearVal) / (top - bottom);
		Result(2, 0) = (right + left) / (right - left);
		Result(2, 1) = (top + bottom) / (top - bottom);
		Result(2, 2) = -(farVal + nearVal) / (farVal - nearVal);
		Result(2, 3) = static_cast<float>(-1);
		Result(3, 2) = -(static_cast<float>(2) * farVal * nearVal) / (farVal - nearVal);
	}

	/*take from glm::ortho*/
	inline void ortho(float left, float right, float bottom, float top, float zNear, float zFar, Matrix4x4 &Result)
	{
		Result.setIdentity();

		Result(0, 0) = static_cast<float>(2) / (right - left);
		Result(1, 1) = static_cast<float>(2) / (top - bottom);
		Result(2, 2) = -static_cast<float>(2) / (zFar - zNear);
		Result(3, 0) = -(right + left) / (right - left);
		Result(3, 1) = -(top + bottom) / (top - bottom);
		Result(3, 2) = -(zFar + zNear) / (zFar - zNear);
		
	}

	/*take from glm::perspective*/
	inline void perspectiveRH(float fovy, float aspect, float zNear, float zFar, Matrix4x4 &Result)
	{
		float const tanHalfFovy = tan(0.5f * fovy * (M_PI / 180.0f));
		
		Result.setZero();

#if 1
		Result(0, 0) = 1.0f / (aspect * tanHalfFovy);
		Result(1, 1) = 1.0f / (tanHalfFovy);
		Result(2, 2) = -(zFar + zNear) / (zFar - zNear);
		Result(3, 2) = -1.0f;
		Result(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
#else
		Result(0, 0) = static_cast<float>(1) / (aspect * tanHalfFovy);
		Result(1, 1) = static_cast<float>(1) / (tanHalfFovy);
		Result(2, 2) = (zFar + zNear) / (zFar - zNear);
		Result(2, 3) = static_cast<float>(1);
		Result(3, 2) = (static_cast<float>(2) * zFar * zNear) / (zFar - zNear);
#endif
	}

private:
	VCameraLens::ProjectionType m_projectionType;

	float m_nearPlane;
	float m_farPlane;

	float m_fieldOfView;
	float m_aspectRatio;

	float m_left;
	float m_right;
	float m_bottom;
	float m_top;

	mutable Matrix4x4 m_projectionMatrix;
};

VK_END_NAMESPACE
#endif