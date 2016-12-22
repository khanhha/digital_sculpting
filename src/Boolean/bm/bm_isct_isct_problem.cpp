#include "bm/bm_isct_isct_problem.h"
#include "BaseLib/MathUtil.h"

namespace bm_isct
{
	using namespace VM;

	IsctProblem::IsctProblem(VM::BMesh *mesh_, const std::vector<BMFace*> &isct_faces)
		: mesh(mesh_),
		faces(isct_faces)
	{
		{
			BMIter iter;
			BMFace *f;
			BM_ITER_MESH(f, &iter, mesh, BM_FACES_OF_MESH){
				BM_ELEM_API_FLAG_CLEAR(f);
			}
		}

		BMLoop *l_iter, *l_first, *l_rad_iter;
		for (auto it = faces.begin(); it != faces.end(); ++it){
			BMFace *f = *it;
			BM_ELEM_API_FLAG_ENABLE(f, F_ISCT);
		}

		/*clear flag and extend the intersection face set*/
		for (auto it = isct_faces.begin(); it != isct_faces.end(); ++it){
			BMFace *f = *it;
			BLI_assert(f->len == 3);

			l_iter = l_first = BM_FACE_FIRST_LOOP(f);
			do {
				l_rad_iter = l_iter;
				do {
					if (!BM_ELEM_API_FLAG_TEST(l_rad_iter->f, F_ISCT)){
						BM_ELEM_API_FLAG_ENABLE(l_rad_iter->f, F_ISCT);
						faces.push_back(l_rad_iter->f);
					}
				} while ((l_rad_iter = l_rad_iter->radial_next) != l_iter);
			} while ((l_iter = l_iter->next) != l_first);
		}

		size_t totfaces = faces.size();
		verts.reserve(totfaces * 3);
		edges.reserve(totfaces * 3);

		/*clear flag and extend the intersection face set*/
		for (auto it = faces.begin(); it != faces.end(); ++it){
			BMFace *f = *it;
			BLI_assert(f->len == 3);
			
			l_iter = l_first = BM_FACE_FIRST_LOOP(f);
			do {
				BM_ELEM_API_FLAG_CLEAR(l_iter->v);
				BM_ELEM_API_FLAG_CLEAR(l_iter->e);	
			} while ((l_iter = l_iter->next) != l_first);
		}

		size_t vert_count = 0;
		size_t edge_count = 0;
		for (size_t i = 0; i < totfaces; ++i){
			
			l_iter = l_first = BM_FACE_FIRST_LOOP(faces[i]);
			do {
				if (!BM_ELEM_API_FLAG_TEST(l_iter->v, V_VISITED)){
					BM_ELEM_API_FLAG_ENABLE(l_iter->v, V_VISITED);
					BM_elem_index_set(l_iter->v, vert_count++);
					verts.push_back(l_iter->v);
				}

				if (!BM_ELEM_API_FLAG_TEST(l_iter->e, E_VISITED)){
					BM_ELEM_API_FLAG_ENABLE(l_iter->e, E_VISITED);
					BM_elem_index_set(l_iter->e, edge_count++);
					edges.push_back(l_iter->e);
				}
			} while ((l_iter = l_iter->next) != l_first);
		}

		mesh->BM_mesh_elem_index_dirty_set(BM_VERT | BM_EDGE);

		// Calibrate the quantization unit...
		float maxMag = 0.0f;
		//        for(VertData &v : TopoCache::mesh->verts) {
		for (auto it = verts.begin(); it != verts.end(); ++it) {
			auto &v = *it;
			maxMag = std::max<float>(maxMag, v->co.cwiseAbs().maxCoeff());
		}
		Quantization::callibrate(maxMag);

		// and use vertex auxiliary data to store quantized vertex coordinates
		uint N = verts.size();
		quantized_coords.resize(N);
		uint write = 0;
		for (size_t i = 0; i < N; ++i){
			Vector3f raw = verts[i]->co;
			quantized_coords[write].x = Quantization::quantize(raw[0]);
			quantized_coords[write].y = Quantization::quantize(raw[1]);
			quantized_coords[write].z = Quantization::quantize(raw[2]);
			//v->data = &(quantized_coords[write]);
			write++;
		}

		
		tri_problems.resize(mesh->BM_mesh_faces_total(), nullptr);
	}

	IsctProblem::~IsctProblem()
	{
	}

	/*both two adjacent faces required to be intersection faces*/
	bool  IsctProblem::edgeIsct(Eptr e)
	{
		BMLoop *l_iter = e->l;
		do {
			if (!BM_ELEM_API_FLAG_TEST(l_iter->f, F_ISCT)){
				return false;
			}
		} while ((l_iter = l_iter->radial_next) != e->l);
		
		return true;
	}


	Vec3d IsctProblem::vPos(Vptr v) const
	{
		return quantized_coords[BM_elem_index_get(v)];
	}

	Tprob IsctProblem::getTprob(Tptr t)
	{
		size_t t_idx = BM_elem_index_get(t);
		Tprob prob = reinterpret_cast<Tprob>(tri_problems[t_idx]);
		if (!prob) {
			tri_problems[t_idx] = prob = tprobs.alloc();
			prob->init(this, t);
		}
		return prob;
	}


	GluePt IsctProblem::newGluePt() {
		GluePt glue = glue_pts.alloc();
		glue->split_type = false;
		return glue;
	}

	IVptr IsctProblem::newIsctVert(Eptr e, Tptr t, GluePt glue) {
		IVptr       iv = ivpool.alloc();
		iv->concrete = nullptr;
		iv->coord = computeCoords(e, t);
		iv->glue_marker = glue;
		glue->copies.push_back(iv);
		return      iv;
	}

	IVptr IsctProblem::newIsctVert(Tptr t0, Tptr t1, Tptr t2, GluePt glue) {
		IVptr       iv = ivpool.alloc();
		iv->concrete = nullptr;
		iv->coord = computeCoords(t0, t1, t2);
		iv->glue_marker = glue;
		glue->copies.push_back(iv);
		return      iv;
	}

	IVptr IsctProblem::newSplitIsctVert(Vec3d coords, GluePt glue) {
		IVptr       iv = ivpool.alloc();
		iv->concrete = nullptr;
		iv->coord = coords;
		iv->glue_marker = glue;
		glue->copies.push_back(iv);
		return      iv;
	}

	IVptr IsctProblem::copyIsctVert(IVptr orig) {
		IVptr       iv = ivpool.alloc();
		iv->concrete = nullptr;
		iv->coord = orig->coord;
		iv->glue_marker = orig->glue_marker;
		orig->glue_marker->copies.push_back(iv);
		return      iv;
	}

	IEptr IsctProblem::newIsctEdge(IVptr endpoint, Tptr tri_key) {
		IEptr       ie = iepool.alloc();
		ie->concrete = nullptr;
		ie->boundary = false;
		ie->ends[0] = endpoint;
		endpoint->edges.push_back(ie);
		ie->ends[1] = nullptr; // other end null
		ie->other_tri_key = tri_key;
		return      ie;
	}

	OVptr IsctProblem::newOrigVert(Vptr v) {
		OVptr       ov = ovpool.alloc();
		ov->concrete = v;
		ov->coord = vPos(v);
		ov->boundary = true;
		return      ov;
	}

	OEptr IsctProblem::newOrigEdge(Eptr e, OVptr v0, OVptr v1) {
		OEptr       oe = oepool.alloc();
		oe->concrete = e;
		oe->boundary = true;
		oe->ends[0] = v0;
		oe->ends[1] = v1;
		v0->edges.push_back(oe);
		v1->edges.push_back(oe);
		return      oe;
	}

	SEptr IsctProblem::newSplitEdge(GVptr v0, GVptr v1, bool boundary) {
		SEptr       se = sepool.alloc();
		se->concrete = nullptr;
		se->boundary = boundary;
		se->ends[0] = v0;
		se->ends[1] = v1;
		v0->edges.push_back(se);
		v1->edges.push_back(se);
		return      se;
	}

	GTptr IsctProblem::newGenericTri(GVptr v0, GVptr v1, GVptr v2) {
		GTptr       gt = gtpool.alloc();
		gt->verts[0] = v0;
		gt->verts[1] = v1;
		gt->verts[2] = v2;
		gt->concrete = nullptr;
		return      gt;
	}

	void IsctProblem::releaseEdge(GEptr ge) {
		disconnectGE(ge);
		IEptr       ie = dynamic_cast<IEptr>(ge);
		if (ie) {
			iepool.free(ie);
		}
		else {
			OEptr   oe = dynamic_cast<OEptr>(ge);
			ENSURE(oe);
			oepool.free(oe);
		}
	}

	void IsctProblem::killOrigVert(OVptr ov) {
		ovpool.free(ov);
	}

	void IsctProblem::killOrigEdge(OEptr oe) {
		oepool.free(oe);
	}

	void IsctProblem::marshallArithmeticInput(Empty3d::EdgeIn &input, Eptr e) const {
		input.p[0] = vPos(e->v1);
		input.p[1] = vPos(e->v2);
	}
	
	void IsctProblem::marshallArithmeticInput( Empty3d::TriIn &input, Tptr t) const 
	{
		BMLoop *l = BM_FACE_FIRST_LOOP(t);
		input.p[0] = vPos(l->v);
		input.p[1] = vPos(l->next->v);
		input.p[2] = vPos(l->next->next->v);
	}
	
	void IsctProblem::marshallArithmeticInput(Empty3d::TriEdgeIn &input, Eptr e, Tptr t) const 
	{
		marshallArithmeticInput(input.edge, e);
		marshallArithmeticInput(input.tri, t);
	}

	void IsctProblem::marshallArithmeticInput(Empty3d::TriTriTriIn &input, Tptr t0, Tptr t1, Tptr t2) const 
	{
		marshallArithmeticInput(input.tri[0], t0);
		marshallArithmeticInput(input.tri[1], t1);
		marshallArithmeticInput(input.tri[2], t2);
	}

	Vec3d IsctProblem::computeCoords(Eptr e, Tptr t) const
	{
		Empty3d::TriEdgeIn input;
		marshallArithmeticInput(input, e, t);
		Vec3d coords = Empty3d::coordsExact(input);
		return coords;
	}

	Vec3d IsctProblem::computeCoords(
		Tptr t0, Tptr t1, Tptr t2
		) const {
		Empty3d::TriTriTriIn input;
		marshallArithmeticInput(input, t0, t1, t2);
		Vec3d coords = Empty3d::coordsExact(input);
		return coords;
	}


	BBox3d IsctProblem::buildBox(Eptr e) const
	{
		Vec3d p0 = vPos(e->v1);
		Vec3d p1 = vPos(e->v2);
		return BBox3d(min(p0, p1), max(p0, p1));
	}

	BBox3d IsctProblem::buildBox(Tptr t) const
	{
		BMLoop *l = BM_FACE_FIRST_LOOP(t);
		Vec3d p0 = vPos(l->v);
		Vec3d p1 = vPos(l->next->v);
		Vec3d p2 = vPos(l->next->next->v);
		return BBox3d(min(p0, min(p1, p2)), max(p0, max(p1, p2)));
	}

	GeomBlob<Eptr> IsctProblem::edge_blob(Eptr e)
	{
		GeomBlob<Eptr>  blob;
		blob.bbox = buildBox(e);
		blob.point = (blob.bbox.minp + blob.bbox.maxp) / 2.0;
		blob.id = e;
		return blob;
	}

	bool IsctProblem::checkIsct(Eptr e, Tptr t) const
	{
		// simple bounding box cull; for acceleration, not correctness
		BBox3d      ebox = buildBox(e);
		BBox3d      tbox = buildBox(t);
		if (!hasIsct(ebox, tbox))
			return      false;

		// must check whether the edge and triangle share a vertex
		// if so, then trivially we know they intersect in exactly that vertex
		// so we discard this case from consideration.
		if (hasCommonVert(e, t))
			return      false;

		Empty3d::TriEdgeIn input;
		marshallArithmeticInput(input, e, t);
		//bool empty = Empty3d::isEmpty(input);
		bool empty = Empty3d::emptyExact(input);
		return !empty;
	}

	bool IsctProblem::checkIsct(Tptr t0, Tptr t1, Tptr t2) const 
	{
		// This function should only be called if we've already
		// identified that the intersection edges
		//      (t0,t1), (t0,t2), (t1,t2)
		// exist.
		// From this, we can conclude that each pair of triangles
		// shares no more than a single vertex in common.
		//  If each of these shared vertices is different from each other,
		// then we could legitimately have a triple intersection point,
		// but if all three pairs share the same vertex in common, then
		// the intersection of the three triangles must be that vertex.
		// So, we must check for such a single vertex in common amongst
		// the three triangles
		Vptr common = commonVert(t0, t1);
		if (common) {
			BMLoop *l_iter, *l_first;
			l_iter = l_first = BM_FACE_FIRST_LOOP(t2);
			do {
				if (common == l_iter->v) return false;
			} while ((l_iter = l_iter->next) != l_first);
		}

		Empty3d::TriTriTriIn input;
		marshallArithmeticInput(input, t0, t1, t2);
		//bool empty = Empty3d::isEmpty(input);
		bool empty = Empty3d::emptyExact(input);
		return !empty;
	}


	void IsctProblem::bvh_edge_tri(std::function<bool(Eptr e, Tptr t)> func)
	{
		std::vector< GeomBlob<Eptr> > edge_geoms;
		std::for_each(
			edges.begin(), edges.end(), 
			[&](Eptr e) 
		{
			edge_geoms.push_back(edge_blob(e));
		});
		
		AABVH<Eptr> edgeBVH(edge_geoms);

		// use the acceleration structure
		bool aborted = false;
		std::for_each(faces.begin(), faces.end(),
			[&](Tptr t) {
			BBox3d bbox = buildBox(t);
			if (!aborted) {
				edgeBVH.for_each_in_box(bbox, [&](Eptr e) {
					if (!func(e, t))
						aborted = true;
				});
			}
		});
	}

	// if we encounter ambiguous degeneracies, then this
	// routine returns false, indicating that the computation aborted.
	bool IsctProblem::tryToFindIntersections()
	{
		Empty3d::degeneracy_count = 0;
		// Find all edge-triangle intersection points
		//for_edge_tri([&](Eptr eisct, Tptr tisct)->bool{
		bvh_edge_tri([&](Eptr eisct, Tptr tisct)->bool{
			if (checkIsct(eisct, tisct)) {
				
				GluePt      glue = newGluePt();
				glue->edge_tri_type = true;
				glue->e = eisct;
				glue->t[0] = tisct;
				
				// first add point and edges to the pierced triangle
				IVptr iv = getTprob(tisct)->addInteriorEndpoint(this, eisct, glue);
				
				BMLoop *l_iter = eisct->l;
				do {
					Tptr tri = l_iter->f;
					getTprob(tri)->addBoundaryEndpoint(this, tisct, eisct, iv);
				} while ((l_iter = l_iter->radial_next) != eisct->l);
			}
			if (Empty3d::degeneracy_count > 0)
				return false; // break
			else
				return true; // continue
		});
		if (Empty3d::degeneracy_count > 0) {
			return false;   // restart / abort
		}

		// we're going to peek into the triangle problems in order to
		// identify potential candidates for Tri-Tri-Tri intersections
		std::vector<TriTripleTemp> triples;
		tprobs.for_each([&](Tprob tprob) {
			Tptr t0 = tprob->the_tri;
			// Scan pairs of existing edges to create candidate triples
			for (uint i = 0; i < tprob->iedges.size(); i++)
				for (uint j = i + 1; j < tprob->iedges.size(); j++){
					//func(vec[i], vec[j]);
					IEptr &ie1 = tprob->iedges[i];
					IEptr &ie2 = tprob->iedges[j];
					Tptr t1 = ie1->other_tri_key;
					Tptr t2 = ie2->other_tri_key;
					// This triple might be considered three times,
					// one for each triangle it contains.
					// To prevent duplication, only proceed if this is
					// the least triangle according to an arbitrary ordering
					if (t0 < t1 && t0 < t2) {
						// now look for the third edge.  We're not
						// sure if it exists...
						Tprob prob1 = reinterpret_cast<Tprob>(getTprob(t1));
						//					for (IEptr ie : prob1->iedges) {
						for (auto it = prob1->iedges.begin(); it != prob1->iedges.end(); ++it) {
							auto &ie = *it;
							if (ie->other_tri_key == t2) {
								// ADD THE TRIPLE
								triples.push_back(TriTripleTemp(t0, t1, t2));
							}
						}
					}
				}

			//for_pairs<IEptr,2>(tprob->iedges, [&](IEptr &ie1, IEptr &ie2){
			//    Tptr t1 = ie1->other_tri_key;
			//    Tptr t2 = ie2->other_tri_key;
			//    // This triple might be considered three times,
			//    // one for each triangle it contains.
			//    // To prevent duplication, only proceed if this is
			//    // the least triangle according to an arbitrary ordering
			//    if(t0 < t1 && t0 < t2) {
			//        // now look for the third edge.  We're not
			//        // sure if it exists...
			//        Tprob prob1 = reinterpret_cast<Tprob>(t1->data);
			//        for(IEptr ie : prob1->iedges) {
			//            if(ie->other_tri_key == t2) {
			//                // ADD THE TRIPLE
			//                triples.push_back(TriTripleTemp(t0, t1, t2));
			//            }
			//        }
			//    }
			//});
		});

		// Now, we've collected a list of Tri-Tri-Tri intersection candidates.
		// Check to see if the intersections actually exist.
		//    for(TriTripleTemp t : triples) {
		for (auto it = triples.begin(); it != triples.end(); ++it) {
			auto &t = *it;

			if (!checkIsct(t.t0, t.t1, t.t2))    continue;

			// Abort if we encounter a degeneracy
			if (Empty3d::degeneracy_count > 0)   break;

			GluePt      glue = newGluePt();
			glue->edge_tri_type = false;
			glue->t[0] = t.t0;
			glue->t[1] = t.t1;
			glue->t[2] = t.t2;
			getTprob(t.t0)->addInteriorPoint(this, t.t1, t.t2, glue);
			getTprob(t.t1)->addInteriorPoint(this, t.t0, t.t2, glue);
			getTprob(t.t2)->addInteriorPoint(this, t.t0, t.t1, glue);
		}

		if (Empty3d::degeneracy_count > 0) {
			return false;   // restart / abort
		}

		return true;
	}

	// In that case, we can perturb the positions of points
	void IsctProblem::perturbPositions()
	{
		const double EPSILON = 1.0e-5; // perturbation epsilon
		
		for (auto it = quantized_coords.begin(); it != quantized_coords.end(); ++it) {
			auto &coord = *it;
			Vec3d perturbation(
				Quantization::quantize(drand(-EPSILON, EPSILON)),
				Quantization::quantize(drand(-EPSILON, EPSILON)),
				Quantization::quantize(drand(-EPSILON, EPSILON)));
			coord += perturbation;
		}
	}

	void IsctProblem::reset()
	{
		// the data pointer in the triangles points to tproblems
		// that we're about to destroy,
		// so zero out all those pointers first!
		std::fill(tri_problems.begin(), tri_problems.end(), nullptr);

		glue_pts.clear();
		tprobs.clear();

		ivpool.clear();
		ovpool.clear();
		iepool.clear();
		oepool.clear();
		sepool.clear();
		gtpool.clear();
	}

	void IsctProblem::findIntersections()
	{
		int nTrys = 5;
		perturbPositions(); // always perturb for safety...
		while (nTrys > 0) {
			if (!tryToFindIntersections()) {
				reset();
				perturbPositions();
				nTrys--;
			}
			else {
				break;
			}
		}
		if (nTrys <= 0) {
			CORK_ERROR("Ran out of tries to perturb the mesh");
			//assert(false);
		}

		// ok all points put together,
		// all triangle problems assembled.
		// Some intersection edges may have original vertices as endpoints
		// we consolidate the problems to check for cases like these.
		tprobs.for_each([&](Tprob tprob) {
			tprob->consolidate(this);
		});
	}

	void IsctProblem::resolveAllIntersections()
	{
		// solve a subdivision problem in each triangle
		tprobs.for_each([&](Tprob tprob) {
			tprob->subdivide(this);
		});

		// now we have diced up triangles inside each triangle problem

		// Let's go through the glue points and create a new concrete
		// vertex object for each of these.
		glue_pts.for_each([&](GluePt glue) {
			createRealPtFromGluePt(glue);
		});

		// Now that we have concrete vertices plugged in, we can
		// go through the diced triangle pieces and create concrete triangles
		// for each of those.
		// Along the way, let's go ahead and hook up edges as appropriate
		tprobs.for_each([&](Tprob tprob) {
			createRealTriangles(tprob);
		});

		std::vector<BMEdge*> isect_edges;
		// mark all edges as normal by zero-ing out the data pointer
		//TopoCache::edges.for_each([](Eptr e) {
		//	e->data = 0;
		//});
		// then iterate over the edges formed by intersections
		// (i.e. those edges without the boundary flag set in each triangle)
		// and mark those by setting the data pointer
		iepool.for_each([&](IEptr ie) {
			// every ie must be non-boundary
			Vptr v1 = ie->ends[0]->concrete;
			Vptr v2 = ie->ends[1]->concrete;
			Eptr e = BM_edge_exists(v1, v2);
			if (e && !BM_elem_app_flag_test(e, E_SUBDIVIDED_ISECT)){
				BM_elem_app_flag_enable(e, E_SUBDIVIDED_ISECT);
				isect_edges.push_back(e);
			}
		});
		sepool.for_each([&](SEptr se) {
			if(se->boundary)    return; // continue
			Vptr v1 = se->ends[0]->concrete;
			Vptr v2 = se->ends[1]->concrete;
			Eptr e = BM_edge_exists(v1, v2);
			if (e && !BM_elem_app_flag_test(e, E_SUBDIVIDED_ISECT)){
				BM_elem_app_flag_enable(e, E_SUBDIVIDED_ISECT);
				isect_edges.push_back(e);
			}
		});
	}


	void IsctProblem::createRealPtFromGluePt(GluePt glue)
	{
		ENSURE(glue->copies.size() > 0);
		const Vec3d &tmpco = glue->copies[0]->coord;
		Vector3f co(static_cast<float>(tmpco[0]), static_cast<float>(tmpco[1]), static_cast<float>(tmpco[2]));
		Vptr        v = mesh->BM_vert_create(co, nullptr, BM_CREATE_NOP);
#if 0
		VertData    &data = TopoCache::mesh->verts[v->ref];
		data.pos = glue->copies[0]->coord;
		fillOutVertData(glue, data);
#endif
		for (auto it = glue->copies.begin(); it != glue->copies.end(); ++it){
			auto &iv = *it;
			iv->concrete = v;
		}

	}

	void IsctProblem::createRealTriangles(Tprob tprob)
	{
		BMVert *verts[3];
		for (auto it = tprob->gtris.begin(); it != tprob->gtris.end(); ++it) {
			auto &gt = *it;
	
			verts[0] = verts[1] = verts[2] = nullptr;
			for (uint k = 0; k < 3; k++) {
				verts[k] = gt->verts[k]->concrete;
			}

			BLI_assert(verts[0] && verts[1] && verts[2]);
			BMFace *new_tri = mesh->BM_face_create_quad_tri(verts[0], verts[1], verts[2], nullptr, nullptr, BM_CREATE_NOP);
			gt->concrete = new_tri;
			new_tri->no = tprob->the_tri->no;
			BM_elem_app_flag_enable(new_tri, BM_elem_app_flag_value(tprob->the_tri));
#if 0
			fillOutTriData(t, tprob->the_tri);
#endif
		}

		// Once all the pieces are hooked up, let's kill the old triangle!
		if(tprob->gtris.size() > 0) mesh->BM_face_kill_loose(tprob->the_tri, false);
	}

	void IsctProblem::resovleIntersections()
	{
		findIntersections();
		resolveAllIntersections();
	}

}