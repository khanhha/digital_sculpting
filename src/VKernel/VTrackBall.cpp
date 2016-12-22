#include "VTrackball.h"

using namespace  gte;

VK_BEGIN_NAMESPACE

VTrackball::VTrackball()
:
mXSize(0),
mYSize(0),
mMultiplier(0.0f),
mX0(0.0f),
mY0(0.0f),
mX1(0.0f),
mY1(0.0f),
mActive(false),
mValidVTrackball(false)
{
	mRoot = std::make_shared<VNode>();
	mInitialOrientation.MakeIdentity();
}

VTrackball::VTrackball(int xSize, int ySize, std::shared_ptr<Camera> const& camera)
	:
	mXSize(0),
	mYSize(0),
	mMultiplier(0.0f),
	mX0(0.0f),
	mY0(0.0f),
	mX1(0.0f),
	mY1(0.0f),
	mActive(false),
	mValidVTrackball(false)
{
	Set(xSize, ySize, camera);
	mInitialOrientation.MakeIdentity();
}

void VTrackball::Set(int xSize, int ySize,
	std::shared_ptr<Camera> const& camera)
{
	if (xSize > 0 && ySize > 0 && camera)
	{
		mXSize = xSize;
		mYSize = ySize;
		mCamera = camera;
		mMultiplier = 1.0f / (mXSize >= mYSize ? mYSize : mXSize);
		mX0 = 0.5f * mXSize;
		mY0 = 0.5f * mYSize;
		mX1 = mX0;
		mY1 = mY0;
		mValidVTrackball = true;
	}
	else
	{
		qCritical("Invalid VTrackball parameters.");
		mValidVTrackball = false;
	}
}

void VTrackball::SetInitialPoint(int x, int y)
{
	if (mValidVTrackball)
	{
		mX0 = (2.0f * x - mXSize) * mMultiplier;
		mY0 = (2.0f * y - mYSize) * mMultiplier;
		mInitialOrientation = mRoot->transform().GetRotation();
	}
}

void VTrackball::SetFinalPoint(int x, int y)
{
	if (mValidVTrackball)
	{
		mX1 = (2.0f * x - mXSize) * mMultiplier;
		mY1 = (2.0f * y - mYSize) * mMultiplier;
		if (mX1 != mX0 || mY1 != mY0)
		{
			UpdateOrientation();
		}
	}
}

void VTrackball::UpdateOrientation()
{
	// Get the first vector on the sphere.
	float sqrLength0 = mX0 * mX0 + mY0 * mY0;
	float length0 = sqrt(sqrLength0), invLength0 = 0.0f, z0, z1;
	if (length0 > 1.0f)
	{
		// Outside the unit disk, project onto it.
		invLength0 = 1.0f / length0;
		mX0 *= invLength0;
		mY0 *= invLength0;
		z0 = 0.0f;
	}
	else
	{
		// Compute point (mX0,mY0,z0) on negative unit hemisphere.
		z0 = 1.0f - sqrLength0;
		z0 = (z0 <= 0.0f ? 0.0f : sqrt(z0));
	}
	z0 *= -1.0f;

	// Use camera world coordinates, order is (D,U,R), so point is (z,y,x).
	EVector4 vec0{ z0, mY0, mX0, 0.0f };

	// Get the second vector on the sphere.
	float sqrLength1 = mX1 * mX1 + mY1 * mY1;
	float length1 = sqrt(sqrLength1), invLength1 = 0.0f;
	if (length1 > 1.0f)
	{
		// Outside unit disk, project onto it.
		invLength1 = 1.0f / length1;
		mX1 *= invLength1;
		mY1 *= invLength1;
		z1 = 0.0f;
	}
	else
	{
		// Compute point (mX1,mY1,z1) on negative unit hemisphere.
		z1 = 1.0f - sqrLength1;
		z1 = (z1 <= 0.0f ? 0.0f : sqrt(z1));
	}
	z1 *= -1.0f;

	// Use camera world coordinates whose order is (D,U,R), so the
	// point is (z,y,x).
	EVector4 vec1{ z1, mY1, mX1, 0.0f };

	// Create axis and angle for the rotation.
	EVector4 axis = Cross(vec0, vec1);
	float dot = Dot(vec0, vec1);
	float angle;
	if (Normalize(axis) > 0.0f)
	{
		angle = acos(std::min(std::max(dot, -1.0f), 1.0f));
	}
	else  // Vectors are parallel.
	{
		if (dot < 0.0f)
		{
			// Rotated pi radians.
			axis[0] = mY0 * invLength0;
			axis[1] = -mX0 * invLength0;
			axis[2] = 0.0f;
			angle = (float)GTE_C_PI;
		}
		else
		{
			// Rotation by zero radians.
			axis.MakeUnit(0);
			angle = 0.0f;
		}
	}

	// Compute the rotation matrix implied by VTrackball motion.  The axis
	// vector was computed in camera coordinates.  It must be converted
	// to world coordinates.  Once again, I use the camera ordering (D,U,R).
	EVector4 worldAxis =
		axis[0] * mCamera->GetDVector() +
		axis[1] * mCamera->GetUVector() +
		axis[2] * mCamera->GetRVector();

	EMatrix4x4 trackRotate = Rotation<4, float>(
		AxisAngle<4, float>(worldAxis, angle));

	// Compute the new rotation, which is the incremental rotation of
	// the VTrackball appiled after the object has been rotated by its old
	// rotation.
#if defined(GTE_USE_MAT_VEC)
	EMatrix4x4 rotate = trackRotate * mInitialOrientation;
#else
	EMatrix4x4 rotate = mInitialOrientation * trackRotate;
#endif

	// Renormalize to avoid accumulated rounding errors that can cause the
	// rotation matrix to degenerate.
	EVector4 v[3] =
	{
		rotate.GetCol(0),
		rotate.GetCol(1),
		rotate.GetCol(2)
	};
	Orthonormalize<4, float>(3, v);
	rotate.SetCol(0, v[0]);
	rotate.SetCol(1, v[1]);
	rotate.SetCol(2, v[2]);

	mRoot->transform().SetRotation(rotate);
}


VK_END_NAMESPACE
