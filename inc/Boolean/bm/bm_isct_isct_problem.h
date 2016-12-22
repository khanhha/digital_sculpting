#ifndef BM_ISCT_ISCT_PROBLEM_H
#define BM_ISCT_ISCT_PROBLEM_H
#include <vector>
#include "BMesh/BMesh.h"
#include "util/iterPool.h"
#include "bm/bm_isct.h"
#include "bm/bm_isct_tri_problem.h"

//#define SEFL_ISCT_UPPER_LOWER_CHECK 1

namespace bm_isct
{

	class TriangleProblem;
	typedef TriangleProblem* Tprob;

	struct TriTripleTemp
	{
		Tptr t0, t1, t2;
		TriTripleTemp(Tptr tp0, Tptr tp1, Tptr tp2) :
			t0(tp0), t1(tp1), t2(tp2)
		{}
	};

	class IsctProblem
	{
	public:
		IsctProblem(VM::BMesh *mesh_, const std::vector<VM::BMFace*> &isct_faces);
		~IsctProblem();
		// access auxiliary quantized coordinates
		bool  edgeIsct(Eptr e);
		Vec3d vPos(Vptr v) const;
		Tprob getTprob(Tptr t);
		GluePt newGluePt();
		IVptr newIsctVert(Eptr e, Tptr t, GluePt glue);
		IVptr newIsctVert(Tptr t0, Tptr t1, Tptr t2, GluePt glue);
		IVptr newSplitIsctVert(Vec3d coords, GluePt glue);
		IVptr copyIsctVert(IVptr orig);
		IEptr newIsctEdge(IVptr endpoint, Tptr tri_key);
		OVptr newOrigVert(Vptr v);
		OEptr newOrigEdge(Eptr e, OVptr v0, OVptr v1);
		SEptr newSplitEdge(GVptr v0, GVptr v1, bool boundary);
		GTptr newGenericTri(GVptr v0, GVptr v1, GVptr v2);
		void releaseEdge(GEptr ge);
		void killOrigVert(OVptr ov);
		void killOrigEdge(OEptr oe);

		void resovleIntersections();
	private:
		void marshallArithmeticInput(Empty3d::TriIn &input, Tptr t) const;
		void marshallArithmeticInput(Empty3d::EdgeIn &input, Eptr e) const;
		void marshallArithmeticInput(Empty3d::TriEdgeIn &input, Eptr e, Tptr t) const;
		void marshallArithmeticInput(Empty3d::TriTriTriIn &input, Tptr t0, Tptr t1, Tptr t2) const;
		Vec3d computeCoords(Eptr e, Tptr t) const;
		Vec3d computeCoords(Tptr t0, Tptr t1, Tptr t2) const;

		BBox3d buildBox(Eptr e) const;
		BBox3d buildBox(Tptr t) const;

		GeomBlob<Eptr> edge_blob(Eptr e);

		bool checkIsct(Eptr e, Tptr t) const;
		bool checkIsct(Tptr t0, Tptr t1, Tptr t2) const;

		void bvh_edge_tri(std::function<bool(Eptr e, Tptr t)>);
	private:
		void resolveAllIntersections();
		void findIntersections();
		bool tryToFindIntersections();
		void perturbPositions();
		void reset();

		void createRealPtFromGluePt(GluePt glue);
		void createRealTriangles(Tprob tprob);
	private:
		VM::BMesh *mesh;
		std::vector<Vptr> verts;
		std::vector<Tptr> faces;
		std::vector<Eptr> edges;


		IterPool<GluePointMarker>   glue_pts;
		IterPool<TriangleProblem>   tprobs;

		IterPool<IsctVertType>      ivpool;
		IterPool<OrigVertType>      ovpool;
		IterPool<IsctEdgeType>      iepool;
		IterPool<OrigEdgeType>      oepool;
		IterPool<SplitEdgeType>     sepool;
		IterPool<GenericTriType>    gtpool;
	private:
		std::vector<Vec3d>          quantized_coords;
		std::vector<Tprob>			tri_problems;
	};
}
#endif