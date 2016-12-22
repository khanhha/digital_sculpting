#ifndef BOOL_BM_ISECT_H
#define BOOL_BM_ISECT_H

#include "bbox.h"
#include "quantization.h"
#include "empty3d.h"
#include "aabvh.h"
#include "BMesh/BMesh.h"
#include "util/shortVec.h"
#include "BaseLib/MathUtil.h"

namespace bm_isct
{

#define TBB_PARALLEL 1

	struct GenericVertType;
	struct IsctVertType;
	struct OrigVertType;
	struct GenericEdgeType;
	struct IsctEdgeType;
	struct OrigEdgeType;
	struct SplitEdgeType;
	struct GenericTriType;
	struct GluePointMarker;

	typedef GenericVertType*		GVptr;
	typedef IsctVertType*           IVptr;
	typedef OrigVertType*	        OVptr;
	typedef GenericEdgeType*		GEptr;
	typedef IsctEdgeType*           IEptr;
	typedef OrigEdgeType*	        OEptr;
	typedef SplitEdgeType*	        SEptr;
	typedef GenericTriType*			GTptr;

	typedef GluePointMarker*		GluePt;

	typedef VM::BMVert*					Vptr;
	typedef VM::BMEdge*					Eptr;
	typedef VM::BMFace*					Tptr;
	

	/*api flag*/
	enum F_PRIVATE_FLAG
	{
		F_ISCT = 1 << 0
	};

	enum V_PRIVATE_FLAG
	{
		V_VISITED = 1 << 0
	};

	enum E_PRIVATE_FLAG
	{
		E_VISITED = 1 << 0
	};

	/*app flag for self intersection, boolean*/
	enum V_ISECT_FLAG
	{
		V_POSITIVE_SIDE		= 1 << 0, /*on positive (upper) in respect of triangle's normal vector*/
		V_NEGATIVE_SIDE		= 1 << 1,  /*on negative (lower) in respect of triangle's normal vector*/
		V_SUBDIVIDED_ISECT	= 1 << 2	/*lie exactly on intersection segments*/
	};

	enum E_ISECT_FLAG
	{
		E_SUBDIVIDED_ISECT = 1 << 0	/*lie exactly on intersection segments*/
	};

	enum F_ISECT_FLAG
	{
		F_VISITED			= 1 << 0,
		F_OUTER_HULL		= 1 << 1,
		F_OBJECT_0			= 1 << 3,
		F_OBJECT_1			= 1 << 4,
		F_INSIDE			= 1 << 5,
		F_OUTSIDE			= 1 << 6
	};

	enum BoolType
	{
		BOOLEAN_UNION = 1 << 0,
		BOOLEAN_DIFFERENCE = 1 << 1,
		BOOLEAN_INTERSECTION = 1 << 2
	};

	struct GenericVertType
	{
		virtual ~GenericVertType() {}
		Vptr                    concrete;
		Vec3d                   coord;

		bool                    boundary;
		uint                    idx; // temporary for triangulation marshalling

		ShortVec<GEptr, 4>       edges;
	};

	struct IsctVertType : public GenericVertType
	{
		GluePt                  glue_marker;
	};

	struct GenericEdgeType
	{
		virtual ~GenericEdgeType() {}
		Eptr                    concrete;

		bool                    boundary;
		uint                    idx; // temporary for triangulation marshalling

		GVptr                   ends[2];
		ShortVec<IVptr, 1>      interior;
	};

	struct IsctEdgeType : public GenericEdgeType
	{
	public:
		// use to detect duplicate instances within a triangle
		Tptr                    other_tri_key;
	};

	struct OrigEdgeType : public GenericEdgeType {};
	struct SplitEdgeType : public GenericEdgeType {};
	struct OrigVertType : public GenericVertType {};

	struct GenericTriType
	{
		Tptr                    concrete; /*original triangle*/
		GVptr                   verts[3];
	};

	struct GluePointMarker
	{
		// list of all the vertices to be glued...
		ShortVec<IVptr, 3>      copies;
		bool                    split_type; // splits are introduced
		// manually, not via intersection
		// and therefore use only e pointer
		bool                    edge_tri_type; // true if edge-tri intersection
		// false if tri-tri-tri
		Eptr                    e;
		Tptr                    t[3];
	};

	template<uint LEN> inline
		IEptr find_edge(ShortVec<IEptr, LEN> &vec, Tptr key)
	{
		//    for(IEptr ie : vec) {
		for (auto it = vec.begin(); it != vec.end(); ++it) {
			auto &ie = *it;
			if (ie->other_tri_key == key)
				return ie;
		}
		return nullptr;
	}

	inline Vptr commonVert(Tptr t0, Tptr t1)
	{
		VM::BMLoop *l_iter, *l_first;
		VM::BMVert *verts_1[3];
		VM::BM_face_as_array_vert_tri(t1, verts_1);
		l_iter = l_first = BM_FACE_FIRST_LOOP(t0);
		do {
			if (l_iter->v == verts_1[0] || l_iter->v == verts_1[1] || l_iter->v == verts_1[2])
				return l_iter->v;
		} while ((l_iter = l_iter->next) != l_first);
		
		return nullptr;
	}

	inline bool hasCommonVert(Tptr t0, Tptr t1)
	{
		VM::BMLoop *l_iter, *l_first;
		VM::BMVert *verts_1[3];
		VM::BM_face_as_array_vert_tri(t1, verts_1);
		l_iter = l_first = BM_FACE_FIRST_LOOP(t0);
		do {
			if (l_iter->v == verts_1[0] || l_iter->v == verts_1[1] || l_iter->v == verts_1[2])
				return true;
		} while ((l_iter = l_iter->next) != l_first);

		return false;
	}

	inline bool hasCommonVert(Eptr e, Tptr t)
	{
		VM::BMLoop *l_iter, *l_first;
		l_iter = l_first = BM_FACE_FIRST_LOOP(t);
		do {
			if (VM::BM_vert_in_edge(e, l_iter->v)) return true;
		} while ((l_iter = l_iter->next) != l_first);
		
		return false;
	}

	inline void disconnectGE(GEptr ge)
	{
		ge->ends[0]->edges.erase(ge);
		ge->ends[1]->edges.erase(ge);
		//    for(IVptr iv : ge->interior)
		for (auto it = ge->interior.begin(); it != ge->interior.end(); ++it)
		{
			auto &iv = *it;
			iv->edges.erase(ge);
		}
	}

	void	collapse_short_subdivided_isct_edges(VM::BMesh *mesh, float threshold);
	float	edge_avg_len(const std::vector<VM::BMFace*> &faces);

#ifdef ISCT_TEST
	inline Point3Dd test_convert(const Vec3d &co){ return Point3Dd(co[0], co[1], co[2]); };
	inline void debug_output_tri_segments(Tptr f, std::vector<Point3Dd> &segments)
	{
		Vptr verts[3];
		BM_face_as_array_vert_tri(f, verts);
		segments.push_back(MathUtil::convert(verts[0]->co));
		segments.push_back(MathUtil::convert(verts[1]->co));
		segments.push_back(MathUtil::convert(verts[1]->co));
		segments.push_back(MathUtil::convert(verts[2]->co));
		segments.push_back(MathUtil::convert(verts[2]->co));
		segments.push_back(MathUtil::convert(verts[0]->co));
	}
#endif


}
#endif