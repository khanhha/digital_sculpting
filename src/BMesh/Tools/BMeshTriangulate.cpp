#include "Tools/BMeshTriangulate.h"
#include "BaseLib/Point3Dd.h"
#include "BaseLib/MathUtil.h"
#include <vector>

VM_BEGIN_NAMESPACE
BMeshTriangulate::BMeshTriangulate(BMesh *bm, const int quad_method, const int ngon_method, const bool tag_only)
	:
	_bm(bm),
	_quad_method(quad_method),
	_ngon_method(ngon_method),
	_tag_only(tag_only)
{}

BMeshTriangulate::~BMeshTriangulate()
{
}

void BMeshTriangulate::run()
{
	std::vector<BMFace*> non_tri_faces;
	BMFace *face;
	BMIter iter;

	BM_ITER_MESH(face, &iter, _bm, BM_FACES_OF_MESH) {
		if (face->len > 3) {
			if (_tag_only == false || BM_elem_flag_test(face, BM_ELEM_TAG)) {
				non_tri_faces.push_back(face);
			}
		}
	}
	
	for (auto it = non_tri_faces.begin(); it != non_tri_faces.end(); ++it){
		_bm->BM_face_triangulate(
			*it,
			nullptr, nullptr, nullptr,
			_quad_method, _ngon_method, _tag_only);
	}
}

VM_END_NAMESPACE