#include "bm/bm_isct_tri_problem.h"
#include "bm/bm_isct_isct_problem.h"
#include <fstream>

namespace bm_isct
{
	using namespace  VM;
	void TriangleProblem::init(IsctProblem *iprob, Tptr t)
	{
		the_tri = t;
		uint k = 0;
		BMLoop *l_iter, *l_first;
		k = 0;
		l_iter = l_first = BM_FACE_FIRST_LOOP(t);
		do {
			BMVert *opp_v = !BM_vert_in_edge(l_iter->e, l_iter->next->v) ? l_iter->next->v : l_iter->prev->v;
			overts[k] = iprob->newOrigVert(opp_v);
			k++;
		} while ((l_iter = l_iter->next) != l_first);

		k = 0;
		l_iter = l_first = BM_FACE_FIRST_LOOP(t);
		do {
			oedges[k] = iprob->newOrigEdge(l_iter->e, overts[(k + 1) % 3], overts[(k + 2) % 3]);
			k++;
		} while ((l_iter = l_iter->next) != l_first);

#if _DEBUG
		for (size_t k = 0; k < 3; ++k){
			BLI_assert(BM_vert_in_edge(oedges[k]->concrete, overts[(k + 1) % 3]->concrete));
			BLI_assert(BM_vert_in_edge(oedges[k]->concrete, overts[(k + 2) % 3]->concrete));
		}
#endif
	}

	void TriangleProblem::addEdge(IsctProblem *iprob, IVptr iv, Tptr tri_key)
	{
		IEptr       ie = find_edge(iedges, tri_key);
		if (ie) { // if the edge is already present
			ie->ends[1] = iv;
			iv->edges.push_back(ie);
		}
		else { // if the edge is being added
			ie = iprob->newIsctEdge(iv, tri_key);
			iedges.push_back(ie);
		}
	}

	void TriangleProblem::addBoundaryHelper(Eptr edge, IVptr iv)
	{
		iv->boundary = true;
		iverts.push_back(iv);
		// hook up point to boundary edge interior!
		for (uint k = 0; k < 3; k++) {
			OEptr   oe = oedges[k];
			if (oe->concrete == edge) {
				oe->interior.push_back(iv);
				iv->edges.push_back(oe);
				break;
			}
		}
	}

	IVptr TriangleProblem::addInteriorEndpoint(IsctProblem *iprob, Eptr edge, GluePt glue)
	{
		IVptr       iv = iprob->newIsctVert(edge, the_tri, glue);
		

		iv->boundary = false;
		iverts.push_back(iv);
		
		BMLoop *l_iter = edge->l;
		do {
			addEdge(iprob, iv, l_iter->f);
		} while ((l_iter = l_iter->radial_next) != edge->l);
		
		return iv;
	}


	void TriangleProblem::addInteriorPoint(IsctProblem *iprob, Tptr t0, Tptr t1, GluePt glue) 
	{
		// note this generates wasted re-computation of coordinates 3X
		IVptr       iv = iprob->newIsctVert(the_tri, t0, t1, glue);
		iv->boundary = false;
		iverts.push_back(iv);

		// find the 2 interior edges
		for (auto it = iedges.begin(); it != iedges.end(); ++it) {
			auto &ie = *it;
			if (ie->other_tri_key == t0 ||
				ie->other_tri_key == t1) {
				ie->interior.push_back(iv);
				iv->edges.push_back(ie);
			}
		}
	}

	void TriangleProblem::addBoundaryEndpoint(IsctProblem *iprob, Tptr tri_key, Eptr edge, IVptr iv)
	{
		iv = iprob->copyIsctVert(iv);
		addBoundaryHelper(edge, iv);
		// handle edge extending into interior
		addEdge(iprob, iv, tri_key);
	}

	void TriangleProblem::consolidate(IsctProblem *iprob)
	{
		// identify all intersection edges missing endpoints
		// and check to see if we can assign an original vertex
		// as the appropriate endpoint.
		for (auto it = iedges.begin(); it != iedges.end(); ++it) {
			auto &ie = *it;
			if (ie->ends[1] == nullptr) {
				// try to figure out which vertex must be the endpoint...
				Vptr vert = commonVert(the_tri, ie->other_tri_key);
				
				if (!vert) {
#if 0
					std::cout << "the  edge is "
						<< ie->ends[0] << ",  "
						<< ie->ends[1] << std::endl;
					IVptr iv = dynamic_cast<IVptr>(ie->ends[0]);
					std::cout << "   "
						<< iv->glue_marker->edge_tri_type
						<< std::endl;
					std::cout << "the   tri is " << the_tri << ": " << *the_tri << std::endl;
					std::cout << "other tri is " << ie->other_tri_key << ": "
						<< *(ie->other_tri_key) << std::endl;
					std::cout << "coordinates for triangles" << std::endl;
					std::cout << "the tri" << std::endl;
					for (uint k = 0; k < 3; k++)
						std::cout << iprob->vPos(the_tri->verts[k])
						<< std::endl;
					for (uint k = 0; k < 3; k++)
						std::cout << iprob->vPos(ie->other_tri_key->verts[k])
						<< std::endl;
					std::cout << "degen count:"
						<< Empty3d::degeneracy_count << std::endl;
					std::cout << "exact count: "
						<< Empty3d::exact_count << std::endl;
#endif
				}
				BLI_assert(vert); // bad if we can't find a common vertex
				// then, find the corresponding OVptr, and connect
				for (uint k = 0; k < 3; k++) {
					if (overts[k]->concrete == vert) {
						ie->ends[1] = overts[k];
						overts[k]->edges.push_back(ie);
						break;
					}
				}

				BM_elem_app_flag_enable(vert, V_SUBDIVIDED_ISECT);
			}
		}

		BLI_assert(isValid());
	}

	bool TriangleProblem::isValid() const
	{
		BLI_assert(the_tri);
		return true;
	}

	void TriangleProblem::subdivide(IsctProblem *iprob)
	{
		// collect all the points, and create more points as necessary
		ShortVec<GVptr, 7> points;
		for (uint k = 0; k<3; k++) {
			points.push_back(overts[k]);
			//std::cout << k << ": id " << overts[k]->concrete->ref << std::endl;
		}
		for (auto it = iverts.begin(); it != iverts.end(); ++it) {
			auto &iv = *it;
			//iprob->buildConcreteVert(iv);
			points.push_back(iv);
			/*std::cout << "  " << points.size() - 1
			<< " (" << iv->glue_marker->edge_tri_type
			<< ") ";
			if(iv->glue_marker->edge_tri_type) {
			Eptr e = iv->glue_marker->e;
			Vec3d p0 = iprob->vPos(e->verts[0]);
			Vec3d p1 = iprob->vPos(e->verts[1]);
			std::cout << " "
			<< e->verts[0]->ref << p0
			<< "  " << e->verts[1]->ref << p1;

			Tptr t = iv->glue_marker->t[0];
			p0 = iprob->vPos(t->verts[0]);
			p1 = iprob->vPos(t->verts[1]);
			Vec3d p2 = iprob->vPos(t->verts[2]);
			std::cout << "        "
			<< t->verts[0]->ref << p0
			<< "  " << t->verts[1]->ref << p1
			<< "  " << t->verts[2]->ref << p2;
			}
			std::cout
			<< std::endl;*/
		}
		for (uint i = 0; i<points.size(); i++)
			points[i]->idx = i;

		// split edges and marshall data
		// for safety, we zero out references to pre-subdivided edges,
		// which may have been destroyed
		ShortVec<GEptr, 8> edges;
		for (uint k = 0; k<3; k++) {
			//std::cout << "oedge:  "
			//          << oedges[k]->ends[0]->idx << "; "
			//          << oedges[k]->ends[1]->idx << std::endl;
			//for(IVptr iv : oedges[k]->interior)
			//    std::cout << "  " << iv->idx << std::endl;
			subdivideEdge(iprob, oedges[k], edges);
			oedges[k] = nullptr;
		}
		//std::cout << "THE TRI: " << the_tri->verts[0]->ref
		//          << "; " << the_tri->verts[1]->ref
		//          << "; " << the_tri->verts[2]->ref
		//          << std::endl;
		//        for(IEptr &ie : iedges) {
		for (auto it = iedges.begin(); it != iedges.end(); ++it) {
			auto &ie = *it;
			//std::cout << "iedge:  "
			//          << ie->ends[0]->idx << "; "
			//          << ie->ends[1]->idx << std::endl;
			//std::cout << "other tri: " << ie->other_tri_key->verts[0]->ref
			//          << "; " << ie->other_tri_key->verts[1]->ref
			//          << "; " << ie->other_tri_key->verts[2]->ref
			//          << std::endl;
			//for(IVptr iv : ie->interior)
			//    std::cout << "  " << iv->idx 
			//              << " (" << iv->glue_marker->edge_tri_type
			//              << ") " << std::endl;
			subdivideEdge(iprob, ie, edges);
			ie = nullptr;
		}
		for (uint i = 0; i<edges.size(); i++)
			edges[i]->idx = i;

		// find 2 dimensions to project onto
		// get normal
		Vec3d normal = cross(overts[1]->coord - overts[0]->coord,
			overts[2]->coord - overts[0]->coord);
		uint normdim = maxDim(abs(normal));
		uint dim0 = (normdim + 1) % 3;
		uint dim1 = (normdim + 2) % 3;
		double sign_flip = (normal.v[normdim] < 0.0) ? -1.0 : 1.0;

		struct triangulateio in, out;

		/* Define input points. */
		in.numberofpoints = points.size();
		in.numberofpointattributes = 0;
		in.pointlist = new REAL[in.numberofpoints * 2];
		in.pointattributelist = nullptr;
		in.pointmarkerlist = new int[in.numberofpoints];
		for (int k = 0; k<in.numberofpoints; k++) {
			in.pointlist[k * 2 + 0] = points[k]->coord.v[dim0];
			in.pointlist[k * 2 + 1] = points[k]->coord.v[dim1] * sign_flip;
			in.pointmarkerlist[k] = (points[k]->boundary) ? 1 : 0;

		}

		/* Define the input segments */
		in.numberofsegments = edges.size();
		in.numberofholes = 0;// yes, zero
		in.numberofregions = 0;// not using regions
		in.segmentlist = new int[in.numberofsegments * 2];
		in.segmentmarkerlist = new int[in.numberofsegments];
		for (int k = 0; k<in.numberofsegments; k++) {
			in.segmentlist[k * 2 + 0] = edges[k]->ends[0]->idx;
			in.segmentlist[k * 2 + 1] = edges[k]->ends[1]->idx;
			in.segmentmarkerlist[k] = (edges[k]->boundary) ? 1 : 0;
		}

		// to be safe... declare 0 triangle attributes on input
		in.numberoftriangles = 0;
		in.numberoftriangleattributes = 0;

		/* set for flags.... */
		out.pointlist = nullptr;
		out.pointattributelist = nullptr; // not necessary if using -N or 0 attr
		out.pointmarkerlist = nullptr;
		out.trianglelist = nullptr; // not necessary if using -E
		//out.triangleattributelist = null; // not necessary if using -E or 0 attr
		//out.trianglearealist = // only needed with -r and -a
		//out.neighborlist = null; // only neccesary if -n is used
		out.segmentlist = nullptr; // NEED THIS; output segments go here
		out.segmentmarkerlist = nullptr; // NEED THIS for OUTPUT SEGMENTS
		//out.edgelist = null; // only necessary if -e is used
		//out.edgemarkerlist = null; // only necessary if -e is used

		// solve the triangulation problem
		char *params = (char*)("pzQYY");
		//char *debug_params = (char*)("pzYYVC");
		triangulate(params, &in, &out, nullptr);

		if (out.numberofpoints != in.numberofpoints) {
			std::cout << "out.numberofpoints: "
				<< out.numberofpoints << std::endl;
			std::cout << "points.size(): " << points.size() << std::endl;
			std::cout << "dumping out the points' coordinates" << std::endl;
			for (uint k = 0; k<points.size(); k++) {
				GVptr gv = points[k];
				std::cout << "  " << gv->coord
					<< "  " << gv->idx << std::endl;
			}

			std::cout << "dumping out the segments" << std::endl;
			for (int k = 0; k<in.numberofsegments; k++)
				std::cout << "  " << in.segmentlist[k * 2 + 0]
				<< "; " << in.segmentlist[k * 2 + 1]
				<< " (" << in.segmentmarkerlist[k]
				<< ") " << std::endl;

			std::cout << "dumping out the solved for triangles now..."
				<< std::endl;
			for (int k = 0; k<out.numberoftriangles; k++) {
				std::cout << "  "
					<< out.trianglelist[(k * 3) + 0] << "; "
					<< out.trianglelist[(k * 3) + 1] << "; "
					<< out.trianglelist[(k * 3) + 2] << std::endl;
			}
		}
		ENSURE(out.numberofpoints == in.numberofpoints);

		//std::cout << "number of points in: " << in.numberofpoints
		//          << std::endl;
		//std::cout << "number of edges in: " << in.numberofsegments
		//          << std::endl;
		//std::cout << "number of triangles out: " << out.numberoftriangles
		//          << std::endl;

		gtris.resize(out.numberoftriangles);
		for (int k = 0; k<out.numberoftriangles; k++) {
			GVptr       gv0 = points[out.trianglelist[(k * 3) + 0]];
			GVptr       gv1 = points[out.trianglelist[(k * 3) + 1]];
			GVptr       gv2 = points[out.trianglelist[(k * 3) + 2]];
			gtris[k] = iprob->newGenericTri(gv0, gv1, gv2);
		}


		// clean up after triangulate...
		// in free
		free(in.pointlist);
		free(in.pointmarkerlist);
		free(in.segmentlist);
		free(in.segmentmarkerlist);
		// out free
		free(out.pointlist);
		//free(out.pointattributelist);
		free(out.pointmarkerlist);
		free(out.trianglelist);
		//free(out.triangleattributelist);
		//free(out.trianglearealist);
		//free(out.neighborlist);
		free(out.segmentlist);
		free(out.segmentmarkerlist);
		//free(out.edgelist);
		//free(out.edgemarkerlist);
	}

	void TriangleProblem::subdivideEdge(IsctProblem *iprob, GEptr ge, ShortVec<GEptr, 8> &edges)
	{
		if (ge->interior.size() == 0) {
			//if(typeid(ge) == typeid(IEptr)) { // generate new edge
			//    iprob->buildConcreteEdge(ge);
			//}
			edges.push_back(ge);
		}
		else if (ge->interior.size() == 1) { // common case
			SEptr se0 = iprob->newSplitEdge(ge->ends[0], ge->interior[0], ge->boundary);
			SEptr se1 = iprob->newSplitEdge(ge->interior[0], ge->ends[1], ge->boundary);
			//iprob->buildConcreteEdge(se0);
			//iprob->buildConcreteEdge(se1);
			edges.push_back(se0);
			edges.push_back(se1);

			// get rid of old edge
			iprob->releaseEdge(ge);
		}
		else { // sorting is the uncommon case
			// determine the primary dimension and direction of the edge
			Vec3d       dir = ge->ends[1]->coord - ge->ends[0]->coord;
			uint        dim = (fabs(dir.x) > fabs(dir.y)) ?
								((fabs(dir.x) > fabs(dir.z)) ? 0 : 2) :
									((fabs(dir.y) > fabs(dir.z)) ? 1 : 2);
			double      sign = (dir.v[dim] > 0.0) ? 1.0 : -1.0;

			// pack the interior vertices into a vector for sorting
			std::vector< std::pair<double, IVptr> > verts;
			//            for(IVptr iv : ge->interior) {
			for (auto it = ge->interior.begin(); it != ge->interior.end(); ++it) {
				auto &iv = *it;

				verts.push_back(std::make_pair(
					// if the sort is ascending, then we're good...
					sign * iv->coord.v[dim],
					iv
					));
			}
			// ... and sort the vector
			std::sort(verts.begin(), verts.end());
			// then, write the verts into a new container with the endpoints
			std::vector<GVptr>  allv(verts.size() + 2);
			allv[0] = ge->ends[0];
			allv[allv.size() - 1] = ge->ends[1];
			for (uint k = 0; k < verts.size(); k++)
				allv[k + 1] = verts[k].second;

			// now create and accumulate new split edges
			for (uint i = 1; i < allv.size(); i++) {
				SEptr   se = iprob->newSplitEdge(allv[i - 1],
					allv[i],
					ge->boundary);
				edges.push_back(se);
			}
			// get rid of old edge
			iprob->releaseEdge(ge);
		}
	}

}