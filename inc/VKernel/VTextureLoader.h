#ifndef VKERNEL_VTEXTURE_LOADER_H
#define VKERNEL_VTEXTURE_LOADER_H
#include <GTGraphics.h>
#include "VKernelCommon.h"
VK_BEGIN_NAMESPACE
class VTextureLoader
{
public:
	static gte::Texture2* load(std::string const& filename);
};
VK_END_NAMESPACE
#endif