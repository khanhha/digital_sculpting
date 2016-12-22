#pragma once

#include <GTGraphics.h>
#include <map>
#include "VKernelCommon.h"
#include "VScene.h"
#include "VCamera.h"

VK_BEGIN_NAMESPACE

	class VPVWUpdater
	{
	public:
		// Construction and destruction.
		virtual ~VPVWUpdater();
		VPVWUpdater();
		VPVWUpdater(VCamera* const& camera, VScene* const& scene, gte::BufferUpdater const& updater);

		// Member access.  The functions are for deferred construction after
		// a default construction of a VPVWUpdater.
		void Set(VCamera* const& camera, VScene* const& scene, gte::BufferUpdater const& updater);
		inline VCamera* const& GetCamera() const;
		inline void SetUpdater(gte::BufferUpdater const& updater);
		inline gte::BufferUpdater const& GetUpdater() const;

		// Update the constant buffer's projection-view-world matrix (pvw-matrix)
		// when the camera's view or projection matrices change.  The input
		// 'pvwMatrixName' is the name specified in the shader program and is
		// used in calls to ConstantBuffer::SetMember<EMatrix4x4>(...).
		// If you modify the view or projection matrices directly through the
		// Camera interface, you are responsible for calling UpdatePVWMatrices().
		//
		// The Subscribe function uses the address of 'worldMatrix' as a key
		// to a std::map, so be careful to ensure that 'worldMatrix' persists
		// until a call to an Unsubscribe function.  The return value of Subscribe
		// is 'true' as long as 'cbuffer' is not already subscribed and actually
		// has a member named 'pvwMatrixName'.  The return value of Unsubscribe is
		// true if and only if the input matrix is currently subscribed.
		bool Subscribe(Transform const& worldMatrix,
			std::shared_ptr<gte::ConstantBuffer> const& cbuffer,
			std::string const& pvwMatrixName = "pvwMatrix");
		bool Unsubscribe(Transform const& worldMatrix);
		void UnsubscribeAll();

		// After any camera modifictions that change the projection or view
		// matrices, call this function to update the constant buffers that
		// are subscribed.
		void Update();

	protected:
		VCamera* mCamera;
		VScene* mScene;
		gte::BufferUpdater mUpdater;

		typedef Transform  const* PVWKey;
		typedef std::pair<std::shared_ptr<gte::ConstantBuffer>, std::string> PVWValue;
		std::map<PVWKey, PVWValue> mSubscribers;
	};


	inline VCamera* const& VPVWUpdater::GetCamera() const
	{
		return mCamera;
	}

	inline void VPVWUpdater::SetUpdater(gte::BufferUpdater const& updater)
	{
		mUpdater = updater;
	}

	inline gte::BufferUpdater const& VPVWUpdater::GetUpdater() const
	{
		return mUpdater;
	}

VK_END_NAMESPACE
