#ifndef VKERNEL_VEFFECT_MANAGER_H
#define VKERNEL_VEFFECT_MANAGER_H
#include <GTGraphics.h>
#include "VKernelCommon.h"

VK_BEGIN_NAMESPACE
class VEffectManager
{
public:
	VEffectManager(gte::ProgramFactory &factory, gte::BufferUpdater bufferUpdator);
	~VEffectManager();
private:
	gte::ProgramFactory &_factory;
	gte::BufferUpdater  &_bufferUpdator;
};
VK_END_NAMESPACE

#endif