// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include "VPVWUpdater.h"

using namespace gte;

VK_BEGIN_NAMESPACE

VPVWUpdater::~VPVWUpdater()
{
}

VPVWUpdater::VPVWUpdater()
{
	Set(nullptr, nullptr, [](std::shared_ptr<Buffer> const&) {});
}

VPVWUpdater::VPVWUpdater(VCamera* const& camera, VScene* const& scene, BufferUpdater const& updater)
{
	Set(camera, scene, updater);
}

void VPVWUpdater::Set(VCamera* const& camera, VScene* const& scene, BufferUpdater const& updater)
{
	mCamera = camera;
	mScene = scene;
	mUpdater = updater;
}

bool VPVWUpdater::Subscribe(Transform  const& worldMatrix,
	std::shared_ptr<ConstantBuffer> const& cbuffer,
	std::string const& pvwMatrixName)
{
	if (cbuffer && cbuffer->HasMember(pvwMatrixName))
	{
		if (mSubscribers.find(&worldMatrix) == mSubscribers.end())
		{
			mSubscribers.insert(std::make_pair(&worldMatrix,
				std::make_pair(cbuffer, pvwMatrixName)));
			return true;
		}
	}
	return false;
}

bool VPVWUpdater::Unsubscribe(Transform const& worldMatrix)
{
	return mSubscribers.erase(&worldMatrix) > 0;
}

void VPVWUpdater::UnsubscribeAll()
{
	mSubscribers.clear();
}

void VPVWUpdater::Update()
{
	if (!mCamera || !mScene) return;

	// The function is called knowing that mCamera is not null.
	Matrix4x4 pvMatrix = mCamera->projectionMatrix() * mCamera->viewMatrix();
	Matrix4x4 rootMatrix = mScene->worldTransform().matrix();

	for (auto& element : mSubscribers)
	{
		// Compute the new projection-view-world matrix.  The matrix
		// *element.first is the model-to-world matrix for the associated
		// object.
#if defined(GTE_USE_MAT_VEC)
		Matrix4x4 pvwMatrix = pvMatrix * rootMatrix * (element.first->matrix());
#else
		Matrix4x4 pvwMatrix = (*element.first) * pvMatrix;
#endif
		// Copy the source matrix into the system memory of the constant
		// buffer.
		element.second.first->SetMember(element.second.second, pvwMatrix);

		// Allow the caller to update GPU memory as desired.
		mUpdater(element.second.first);
	}
}

VK_END_NAMESPACE