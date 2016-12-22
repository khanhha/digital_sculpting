#ifndef BM_ISECT_TRI_PROBLEM_H
#define BM_ISECT_TRI_PROBLEM_H
#include "bm/bm_isct.h"

#define REAL double
extern "C" {
#include "triangle.h"
}
namespace bm_isct
{
	class IsctProblem;
	class TriangleProblem
	{
	public:
		TriangleProblem() {}
		~TriangleProblem() {}

		void init(IsctProblem *iprob, Tptr t);
	private:
		void addEdge(IsctProblem *iprob, IVptr iv, Tptr tri_key);
		void addBoundaryHelper(Eptr edge, IVptr iv);
	public:
		// specify reference glue point and edge piercing this triangle.
		IVptr addInteriorEndpoint(IsctProblem *iprob, Eptr edge, GluePt glue);

		void addInteriorPoint(IsctProblem *iprob, Tptr t0, Tptr t1, GluePt glue);
		// specify the other triangle cutting this one, the edge cut,
		// and the resulting point of intersection
		void addBoundaryEndpoint(IsctProblem *iprob, Tptr tri_key, Eptr edge, IVptr iv);
		// run after we've accumulated all the elements
		void consolidate(IsctProblem *iprob);
		bool isValid() const;
		void subdivide(IsctProblem *iprob);
	private:
		void subdivideEdge(IsctProblem *iprob, GEptr ge, ShortVec<GEptr, 8> &edges);
	private:
		friend class IsctProblem;

		ShortVec<IVptr, 4>      iverts;
		ShortVec<IEptr, 2>      iedges;
		// original triangle elements
		OVptr                   overts[3];
		OEptr                   oedges[3];

		ShortVec<GTptr, 8>      gtris;

		Tptr                    the_tri;
	};
}
#endif