#include "VBvh/BMeshBvhNodeSplitter.h"
#include "VBvh/VBvhUtil.h"
VBVH_BEGIN_NAMESPACE

BMeshBvhNodeSplitter::BMeshBvhNodeSplitter(BMLeafNode *node, const BMeshBvhContext &bmbvhinfo)
	:
	_node(node),
	_bmbvh_info(bmbvhinfo)
{
	_total = node->faces().size();
	_prims = reinterpret_cast<VPrimRef*>(scalable_aligned_malloc(sizeof(VPrimRef) * _total, 32));
}

BMeshBvhNodeSplitter::~BMeshBvhNodeSplitter()
{
	scalable_aligned_free(_prims);
}

void BMeshBvhNodeSplitter::run()
{
	/*calculate primitive information*/
	BMFacesPrimInfoCompute(_node->faces().data(), _total, _prims, _priminfo, false);

	/*build a bvh node hierarchy from primitive list*/
	VBvhBinBuilder<BMLeafNode, 16> b(_priminfo, _prims, _total, 200, &_bmbvh_info);
	b.build();

	_leafs = std::move(b.leafNodes());
	_root = b.rootNode();
}

VBVH_END_NAMESPACE