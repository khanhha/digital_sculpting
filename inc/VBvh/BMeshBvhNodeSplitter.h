#ifndef VBVH_BMESHBVH_NODE_SPLITER_H
#define VBVH_BMESHBVH_NODE_SPLITER_H
#include "VBvh/VBvhDefine.h"
#include "VBvh/BMeshBvh.h"
#include "VBvh/VPrimRef.h"
#include "VBvh/VPrimInfor.h"
#include "VBvh/VBvhBinBuilder.h"

VBVH_BEGIN_NAMESPACE
class BMeshBvhNodeSplitter
{
public:
	BMeshBvhNodeSplitter(BMLeafNode *node, const BMeshBvhContext &bmbvhinfo);
	~BMeshBvhNodeSplitter();
	void run();
	BaseNode *root(){ return _root; };
	std::vector<BMLeafNode*> leafs(){ return _leafs; }
private:
	const BMeshBvhContext &_bmbvh_info;
	BMLeafNode	*_node;
	VPrimInfo    _priminfo;
	VPrimRef	*_prims;
	size_t		 _total;

	/*result*/
	BaseNode *_root;
	std::vector<BMLeafNode*> _leafs;
};

VBVH_END_NAMESPACE
#endif