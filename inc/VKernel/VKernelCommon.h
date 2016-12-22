#ifndef VKERNEL_VKERNEL_COMMON_H
#define VKERNEL_VKERNEL_COMMON_H

#define VK_BEGIN_NAMESPACE namespace vk{
#define VK_END_NAMESPACE }

#include <Eigen/Dense>
#include <Eigen/Geometry>
VK_BEGIN_NAMESPACE

typedef Eigen::Matrix<float, 2, 1, Eigen::DontAlign> Vector2;
typedef Eigen::Matrix<float, 3, 1, Eigen::DontAlign> Vector3;
typedef Eigen::Matrix<float, 4, 1, Eigen::DontAlign> Vector4;
typedef Eigen::Matrix<float, 3, 3, Eigen::DontAlign> Matrix3x3;
typedef Eigen::Matrix<float, 4, 4, Eigen::DontAlign> Matrix4x4;

typedef Eigen::Quaternionf	Quaternion;
typedef Eigen::AngleAxisf	AngleAxis;
typedef Eigen::Transform <float, 3, Eigen::Affine> Transform;

VK_END_NAMESPACE
#endif // !VKERNEL_VKERNEL_COMMON_H
