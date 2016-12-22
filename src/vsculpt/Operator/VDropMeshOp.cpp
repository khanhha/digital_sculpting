#include "VDropMeshOp.h"

#include <QMimeData>

#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include <wrap/io_trimesh/import.h>

#include <cstdlib>
#include <sys/timeb.h>
#include <iostream>
#include <string>
#include <qlogging.h>

#include "MathUtil.h"

using namespace VM;
using namespace vk;
VDropMeshOp::VDropMeshOp()
{}

VDropMeshOp::~VDropMeshOp()
{}

bool VDropMeshOp::poll()
{
	return true;
}

VOperator* VDropMeshOp::create()
{
	return new VDropMeshOp();
}

int  VDropMeshOp::invoke(QEvent *ev)
{
	QDropEvent *dropEv = dynamic_cast<QDropEvent*>(ev);
	if (dropEv){
		const QMimeData *mimeData = dropEv->mimeData();

		QString filePath;

		if (mimeData->hasUrls()) {
			QStringList pathList;
			QList<QUrl> urlList = mimeData->urls();
			filePath = urlList.at(0).toLocalFile();
			if (filePath.isEmpty() == false){
				BMesh *bmesh = import_vcg(filePath);
				if (bmesh){
					VMeshObject *node = new VMeshObject(VContext::instance()->scene(), bmesh);
					VContext::instance()->scene()->addNode(node);
					VContext::instance()->scene()->markActiveObject(node);

					VContext::instance()->window()->update();

				}
			}
		}
	}

	return OP_RET_FINISHED;
}

BMesh* VDropMeshOp::import_vcg(const QString& filepath)
{
	class MyVertex;
	class MyEdge;
	class MyFace;
	struct MyUsedTypes : public vcg::UsedTypes<vcg::Use<MyVertex>::AsVertexType, vcg::Use<MyEdge>::AsEdgeType, vcg::Use<MyFace>::AsFaceType>{};
	class  MyVertex : public vcg::Vertex< MyUsedTypes, vcg::vertex::VFAdj, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::Mark, vcg::vertex::BitFlags>{};
	class  MyEdge : public vcg::Edge< MyUsedTypes> {};
	class  MyFace : public vcg::Face< MyUsedTypes,vcg::face::VFAdj, vcg::face::VertexRef, vcg::face::BitFlags > {};
	class  MyMesh : public vcg::tri::TriMesh<std::vector<MyVertex>, std::vector<MyFace> > {};

	std::string file = filepath.toStdString();
	MyMesh mesh;
	qInfo("reading file");
	int err = vcg::tri::io::Importer<MyMesh>::Open(mesh, file.data());
	if (err){
		return nullptr;
	}
	else{
		if (filepath.contains(".stl")){
			qInfo("removing duplicated vertices");
			vcg::tri::Clean<MyMesh>::RemoveDuplicateVertex(mesh);
		}

		qInfo("creating bmesh");
		qInfo("creating vertices");

		BMesh *bm = new BMesh();
		std::unordered_map<MyMesh::VertexPointer, BMVert*> bmverts;

		MyMesh::FaceIterator fi;
		MyMesh::VertexIterator vi;
		for (size_t i  = 0; i < mesh.vert.size(); ++i){
			MyMesh::VertexPointer v = &mesh.vert[i];
			MyMesh::CoordType &coord = v->P();
			if (!v->IsD()){
				Vector3f co(coord[0], coord[1], coord[2]);
				bmverts[v] = bm->BM_vert_create(co, nullptr, BM_CREATE_NOP);
			}
		}

		qInfo("creating faces");
		// If you loop in a mesh with deleted elements you have to skip them!
		for (fi = mesh.face.begin(); fi != mesh.face.end(); ++fi)
		{
			if (!fi->IsD()) //    <---- Check added
			{
				BMVert *verts[3] = { nullptr, nullptr, nullptr };
				for (size_t j = 0; j < 3; ++j){
					MyMesh::VertexPointer v = fi->V(j);
					verts[j] = bmverts[v];
				}

				if (verts[0] && verts[1] && verts[2]){
					bm->BM_face_create_quad_tri(verts[0], verts[1], verts[2], nullptr, nullptr, BM_CREATE_NOP);
				}
			}
		}

		qInfo("updating normals");
		bm->BM_mesh_normals_update();
		qInfo("Done importing");

		return bm;
	}
}
