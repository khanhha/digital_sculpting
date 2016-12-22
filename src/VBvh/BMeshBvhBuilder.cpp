#include "BMeshBvhBuilder.h"
#include "VBvh/VBvhBinBuilder.h"
#include "tbb/parallel_for.h"
#include <Eigen/Dense>
#include <QElapsedTimer>

using namespace Eigen;

VBVH_BEGIN_NAMESPACE


BMeshBvhBuilder::BMeshBvhBuilder(BMesh *bm)
	:
	_bmesh(bm)
{
	_bmesh->BM_mesh_elem_index_table_check(BM_FACE);
}

BMBvh* BMeshBvhBuilder::build()
{
	QElapsedTimer timer; timer.start();

	size_t totvert = _bmesh->BM_mesh_verts_total();
	size_t totface  = _bmesh->BM_mesh_faces_total();
	VPrimRef *prims = (VPrimRef*)scalable_aligned_malloc(sizeof(VPrimRef)*totface, 32);

	const std::vector<BMFace*> &faces = _bmesh->BM_mesh_face_table();
	computeBounds(faces, prims, totface);
	
#ifdef USE_BIN_BUILDER
	VPrimInfo vpriminfo(empty);
	computePrimInfo(prims, totface, vpriminfo);
	VBvhBinBuilder<BMLeafNode, 16> b(vpriminfo, prims, totface, 200, &binfo);
	b.build();
#else
	VBvhMortonBuilder<BMLeafNode> b(prims, totface, nullptr, 400);
	b.build();
#endif

	scalable_aligned_free(prims);

	std::ofstream of("D:\\vbvh_buidler.txt");
	of << "verts: " << _bmesh->BM_mesh_verts_total() << std::endl;
	of << "faces: " << _bmesh->BM_mesh_faces_total() << std::endl;
	of << "build time: " << timer.elapsed() << std::endl; of.close();

	BMBvh *bvh =  new BMBvh(_bmesh, b.rootNode(), b.leafNodes());
	return bvh;
}

void BMeshBvhBuilder::computeBounds(const std::vector<BMFace*> &faces, VPrimRef *prims, size_t tot)
{
	size_t threads = tbb::task_scheduler_init::default_num_threads();
	size_t _Step  = tot / threads;
	size_t _Remain = tot % threads;

	// Count in parallel and separately save their local results without reducing
	tbb::parallel_for(static_cast<size_t>(0), threads, [&](size_t _Index)
	{
		Vector3f flower, fupper;
		size_t _Beg_index, _End_index;
		VCentGeomBBox3fa bounds; bounds.reset();

		// Calculate the segment position
		if (_Index < _Remain){
			_Beg_index = _Index * (_Step + 1);
			_End_index = _Beg_index + (_Step + 1);
		}
		else{
			_Beg_index = _Remain * (_Step + 1) + (_Index - _Remain) * _Step;
			_End_index = _Beg_index + _Step;
		}


		for (size_t i = _Beg_index; i < _End_index; ++i){
			BMFace* face  = faces[i];
			VPrimRef& prim = prims[i];
			BM_face_calc_bounds(face, flower, fupper);
			prim.set(
				Vec3fa(flower[0], flower[1], flower[2]), 
				Vec3fa(fupper[0], fupper[1], fupper[2]), 
				reinterpret_cast<size_t>(face));
		}
	});
}

void BMeshBvhBuilder::computePrimInfo(VPrimRef *prims, size_t tot, VPrimInfo &infor)
{
	infor = VPrimInfo(empty);
	for (size_t i = 0; i < tot; i++){
		infor.add(prims[i].bounds(), prims[i].center2());
	}
}

void BMeshBvhBuilder::addCustomDataLayer(BMeshBvhContext  &info)
{
	int cd_node_layer_index;
	
	{
		cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_VERT, CD_PROP_INT, cd_arr_off.c_str());
		if (cd_node_layer_index == -1) {
			info.bm->BM_data_layer_add_named(BM_VERT, CD_PROP_INT, cd_arr_off.c_str());
			cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_VERT, CD_PROP_INT, cd_arr_off.c_str());
		}
	}

	{
		cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_VERT, CD_PROP_POINTER, cd_node_off.c_str());
		if (cd_node_layer_index == -1) {
			info.bm->BM_data_layer_add_named(BM_VERT, CD_PROP_POINTER, cd_node_off.c_str());
		}
	}


	{
		cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_FACE, CD_PROP_INT, cd_arr_off.c_str());
		if (cd_node_layer_index == -1) {
			info.bm->BM_data_layer_add_named(BM_FACE, CD_PROP_INT, cd_arr_off.c_str());
			cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_FACE, CD_PROP_INT, cd_arr_off.c_str());
		}
	}


	{
		cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_FACE, CD_PROP_POINTER, cd_node_off.c_str());
		if (cd_node_layer_index == -1) {
			info.bm->BM_data_layer_add_named(BM_FACE, CD_PROP_POINTER, cd_node_off.c_str());
			cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_FACE, CD_PROP_POINTER, cd_node_off.c_str());
		}
	}



	cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_VERT, CD_PROP_POINTER, cd_node_off.c_str());
	info.cd_vnode = info.bm->BM_data_get_n_offset( 
		BM_VERT, CD_PROP_POINTER,
		cd_node_layer_index - info.bm->BM_data_get_layer_index(BM_VERT, CD_PROP_POINTER));


	cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_VERT, CD_PROP_INT, cd_arr_off.c_str());
	info.cd_voff = info.bm->BM_data_get_n_offset(
		BM_VERT, CD_PROP_INT,
		cd_node_layer_index - info.bm->BM_data_get_layer_index(BM_VERT, CD_PROP_INT));


	cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_FACE, CD_PROP_POINTER, cd_node_off.c_str());
	info.cd_fnode = info.bm->BM_data_get_n_offset(
		BM_FACE, CD_PROP_POINTER,
		cd_node_layer_index - info.bm->BM_data_get_layer_index(BM_FACE, CD_PROP_POINTER));


	cd_node_layer_index = info.bm->BM_data_get_named_layer_index(BM_FACE, CD_PROP_INT, cd_arr_off.c_str());
	info.cd_foff = info.bm->BM_data_get_n_offset(
		BM_FACE, CD_PROP_INT,
		cd_node_layer_index - info.bm->BM_data_get_layer_index(BM_FACE, CD_PROP_INT));
}

VBVH_END_NAMESPACE
