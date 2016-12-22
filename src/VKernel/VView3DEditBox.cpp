#include "VView3DEditBox.h"
#include "VView3DRegion.h"
#include "VContext.h"
#include "VTextureLoader.h"
#include <boost/algorithm/clamp.hpp>
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include <wrap/io_trimesh/import.h>
#include <QDebug>
using namespace  gte;
using namespace Eigen;
VK_BEGIN_NAMESPACE

VView3DEditBox::VView3DEditBox()
{
	_select.stt = BoxSelect::SEL_NONE;
	_camera = std::make_shared<VCamera>();
	_region_size = 0.4f;
	createBoxGeometry();
}

VView3DEditBox::~VView3DEditBox()
{
}

void VView3DEditBox::onRender(VView3DRegion *region)
{
	ensureEffect();

	QRect rvp = region->viewport();
	float boxsize = std::min<float>(rvp.width(), rvp.height()); boxsize = 0.15f * boxsize;
	_rect = QRect(rvp.x(), rvp.y(), boxsize, boxsize);

	VCamera *rcam = region->camera();
	Vector3f bviewcenter(0.0f, 0.0f, 0.0f);
	Vector3f bcampos = bviewcenter -  5.0f * rcam->viewVector().normalized();
	_camera->setFrame(bcampos, rcam->upVector(), bviewcenter);
	_camera->setPerspectiveProjection(rcam->fieldOfView(), 1.0f, rcam->nearPlane(), rcam->farPlane());

	EMatrix4x4 pvw = _camera->projectionMatrix() * _camera->viewMatrix();
	_effect->SetPVWMatrix(pvw);
	GL::engine()->Update(_effect->GetPVWMatrixConstant());
	
	updateHighlightColor();
	if (_box.updateGL){
		GL::engine()->Update(_box.vbuffer);
		_box.updateGL = false;
	}

	QRect save_vp = region->viewport();
	GL::engine()->SetViewport(_rect.x(), _rect.y(), _rect.width(), _rect.height());
	GL::engine()->Enable(_effect);
	GL::engine()->DrawBuffer(_box.vbuffer, _box.ibuffer, _effect);
	GL::engine()->Disable(_effect);
	GL::engine()->SetViewport(save_vp.x(), save_vp.y(), save_vp.width(), save_vp.height());

	/*we don't need texture's CPU memory anymore*/
	_texture->DestroyStorage();
}

void VView3DEditBox::ensureEffect()
{
	if (!_effect){
		GLSLProgramFactory factory;
		EVector4 color(1.0f, 0.0f, 0.0f, 1.0f);

		_texture = std::shared_ptr<Texture2>(VTextureLoader::load("..\\..\\qml\\cube.png"));
		_effect =
			std::make_shared<Texture2ColorEffect>(factory, _texture,
			SamplerState::MIN_L_MAG_L_MIP_P, SamplerState::CLAMP,
			SamplerState::CLAMP);
	}
}

bool VView3DEditBox::select(Vector2f pos)
{
	_select.stt = BoxSelect::SEL_NONE;

	if (_rect.contains(QPoint(pos[0], pos[1]))){
		Vector3f org = _camera->position();
		Matrix4x4 pvw_inv = _camera->projViewMatrix().inverse();
		Vector3f dir = MathUtil::unproject(pvw_inv, _rect, Vector3f(pos[0], pos[1], 0.5f));
		dir = (dir - org).normalized();

		Vector3f bmin(-1.0f, -1.0f, -1.0f);
		Vector3f bmax(1.0f, 1.0f, 1.0f);
		Vector3f hit;
		if (MathGeom::isect_ray_aligned_box_v3(org, dir, bmin, bmax, hit)){
			BoxSelect::State stt;
			Vector3f pos;
			classifyPointRegion(hit, _select.stt, _select.dir);
		}
	}

	return _select.stt != BoxSelect::SEL_NONE;
}

bool VView3DEditBox::viewdir(Vector3f &d)
{
	d.setZero();
	if (_select.stt != BoxSelect::SEL_NONE){
		if (_select.stt == BoxSelect::SEL_CORNER){
			d = -_select.dir;
		}
		else if(_select.stt == BoxSelect::SEL_SIDE){
			d = -_select.dir;
		}
		else{
			/*BoxSelect::SEL_EDGE*/
			d = -_select.dir;
		}
		return true;
	}

	return false;
}

QRect VView3DEditBox::rect()
{
	return _rect;
}

void VView3DEditBox::createBoxGeometry()
{
	class MyVertex;
	class MyEdge;
	class MyFace;
	struct MyUsedTypes : public vcg::UsedTypes<vcg::Use<MyVertex>::AsVertexType, vcg::Use<MyEdge>::AsEdgeType, vcg::Use<MyFace>::AsFaceType>{};
	class  MyVertex : public vcg::Vertex< MyUsedTypes, vcg::vertex::VFAdj, vcg::vertex::Coord3f, vcg::vertex::TexCoord2f, vcg::vertex::Mark, vcg::vertex::BitFlags>{};
	class  MyEdge : public vcg::Edge< MyUsedTypes> {};
	class  MyFace : public vcg::Face< MyUsedTypes,vcg::face::VFAdj, vcg::face::VertexRef, vcg::face::BitFlags > {};
	class  MyMesh : public vcg::tri::TriMesh<std::vector<MyVertex>, std::vector<MyFace> > {};

	std::string file("..\\..\\qml\\box_view.ply");
	MyMesh mesh;
	vcg::tri::io::Importer<MyMesh>::Open(mesh, file.data());

	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	vformat.Bind(VA_TEXCOORD, DF_R32G32_FLOAT, 0);
	vformat.Bind(VA_COLOR,	  DF_R32G32B32A32_FLOAT, 0);
	size_t numvert = mesh.face.size() * 3;
	auto vbuffer = std::shared_ptr<VertexBuffer>(new VertexBuffer(vformat, numvert));
	vbuffer->SetUsage(Resource::Usage::DYNAMIC_UPDATE);
	VertexData *vdata = vbuffer->Get<VertexData>();

	auto ibuffer = std::shared_ptr<IndexBuffer>(new IndexBuffer(IPType::IP_TRIMESH, numvert, sizeof(unsigned)));
	unsigned *idata = ibuffer->Get<unsigned>();

	MyMesh::FaceIterator fi;
	size_t cnt = 0;
	for (fi = mesh.face.begin(); fi != mesh.face.end(); ++fi)
	{
		if (!fi->IsD()) //    <---- Check added
		{
			for (size_t j = 0; j < 3; ++j){
				MyMesh::VertexPointer v = fi->V(j);
				vdata[cnt].position = EVector3(v->P()[0], v->P()[1], v->P()[2]);
				vdata[cnt].texture= EVector2(v->T().u(), v->T().v());
				vdata[cnt].color = EVector4(1.0f, 1.0f, 1.0f, 1.0f);
				idata[cnt] = cnt;
				cnt++;
			}
		}
	}

	_box.vbuffer = vbuffer;
	_box.ibuffer = ibuffer;
	_box.updateGL = true;
	_box.verts.resize(numvert);
	for (size_t i = 0; i < numvert; ++i){
		_box.verts[i].vdata = &vdata[i];
		_box.verts[i].stt = BoxSelect::SEL_NONE;
		_box.verts[i].pos = Vector3f(0.0f, 0.0f, 0.0f);
	}
	_box.stt = _box.stt = BoxSelect::SEL_NONE;
	classifyVertsRegion();
}

void VView3DEditBox::updateHighlightColor()
{
	const float select_hight_light = 0.5f;
	const float view_hight_light = 0.8f;

	/*find current opposite view direction*/
	Vector3f opvdir = -_camera->viewVector().normalized();
	BoxSelect::State vstt = BoxSelect::SEL_NONE;
	Vector3f vpos;
	neareastRegion(opvdir, vstt, vpos);

	if ((vstt != BoxSelect::SEL_NONE) && (_box.vstt != vstt && (_box.vdir - vpos).squaredNorm() > 0.01f)){
		resetHightlight();
	}
	if ((_select.stt != BoxSelect::SEL_NONE) || (_box.stt != _select.stt && (_box.dir - _select.dir).squaredNorm() > 0.01f)){
		resetHightlight();
	}

	if ((vstt != BoxSelect::SEL_NONE) && (_box.vstt != vstt && (_box.vdir - vpos).squaredNorm() > 0.01f)){
		for (auto it = _box.verts.begin(); it != _box.verts.end(); ++it){
			VertexInfor &vinfor = *it;
			if (vinfor.stt == vstt && (vinfor.pos - vpos).squaredNorm() < FLT_EPSILON){
				for (size_t j = 0; j < 3; ++j){
					vinfor.vdata->color = gte::EVector4(view_hight_light, view_hight_light, view_hight_light, 1.0f);
				}
			}
		}
		_box.updateGL = true;
	}

	if ((_select.stt != BoxSelect::SEL_NONE) || (_box.stt != _select.stt && (_box.dir - _select.dir).squaredNorm() > 0.01f)){
		for (auto it = _box.verts.begin(); it != _box.verts.end(); ++it){
			VertexInfor &vinfor = *it;
			if (vinfor.stt == _select.stt && (vinfor.pos - _select.dir).squaredNorm() < FLT_EPSILON){
				for (size_t j = 0; j < 3; ++j){
					vinfor.vdata->color = gte::EVector4(select_hight_light, select_hight_light, select_hight_light, 1.0f);
				}
			}
		}
		_box.updateGL = true;
	}
}

void VView3DEditBox::resetHightlight()
{
	for (auto it = _box.verts.begin(); it != _box.verts.end(); ++it){
		it->vdata->color = EVector4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void VView3DEditBox::classifyVertsRegion()
{
	size_t totvert = _box.verts.size();
	for (size_t i = 0; i < totvert; i += 3){
		BoxSelect::State stt;
		Vector3f pos;

		Vector3f center; center.setZero();
		for (size_t j = 0; j < 3; ++j){
			center += _box.verts[i + j].vdata->position;
		}
		center /= 3.0f;
		if (!classifyPointRegion(center, stt, pos))
			BLI_assert(false);

		for (size_t j = 0; j < 3; ++j){
			_box.verts[i + j].stt = stt;
			_box.verts[i + j].pos = pos;
		}
	}
}

bool VView3DEditBox::classifyPointRegion(const Eigen::Vector3f &coord, BoxSelect::State &stt, Vector3f &pos) const
{
	size_t middle_axis_cnt = 0;
	for (size_t i = 0; i < 3; ++i){
		if (std::abs(coord[i]) < 1.0f - _region_size){
			middle_axis_cnt++;
		}
	}

	if (middle_axis_cnt == 0){
		stt = BoxSelect::SEL_CORNER;
		for (size_t i = 0; i < 3; ++i){
			pos[i] = coord[i] < 0 ? -1.0f : 1.0f;
		}
	}
	else if (middle_axis_cnt == 1){
		stt = BoxSelect::SEL_EDGE;
		int edge_axis = -1;
		for (size_t i = 0; i < 3; ++i){
			if (std::abs(coord[i]) < 1.0f - _region_size){
				edge_axis = i;
				break;
			}
		}
		BLI_assert(edge_axis >= 0);
		for (size_t i = 1; i <= 2; ++i){
			int other_axis = (edge_axis + i) % 3;
			pos[other_axis] = coord[other_axis] > 0 ? 1.0f : -1.0f;
		}
		pos[edge_axis] = 0.0f;
	}
	else if (middle_axis_cnt == 2){
		stt = BoxSelect::SEL_SIDE;
		for (size_t i = 0; i < 3; ++i){
			if (std::abs(coord[i]) < 1.0f - _region_size){
				pos[i] = 0.0f;
			}
			else{
				pos[i] = coord[i] > 0 ? 1.0f : -1.0f;
			}
		}
	}
	else{
		stt = BoxSelect::SEL_NONE;
	}

	return stt != BoxSelect::SEL_NONE;

}

/*
*dir: run from origin(0,0,0) through some point on box's surface
*find the region nearest to this direction
*/
bool VView3DEditBox::neareastRegion(Vector3f dir, BoxSelect::State &stt, Eigen::Vector3f &pos)
{
	float max_dot = -10000.0f;
	Vector3i min_pos; min_pos.setZero();
	for (int i = -1; i <= 1; ++i){
		for (int j = -1; j <= 1; ++j){
			for (int k = -1; k <= 1; ++k){
				Vector3f pp((float)i, (float)j, (float)k); pp.normalize();
				float val = pp.dot(dir);
				if (val > max_dot){
					max_dot = val;
					min_pos = Vector3i(i,j,k);
				}
			}
		}
	}

	size_t zero_cnt = 0;
	for (size_t i = 0; i < 3; ++i){
		if (min_pos[i] == 0) zero_cnt++;
	}

	if (zero_cnt == 0){
		stt = BoxSelect::SEL_CORNER;
	}
	else if (zero_cnt == 1){
		stt = BoxSelect::SEL_EDGE;
	}
	else if (zero_cnt == 2){
		stt = BoxSelect::SEL_SIDE;
	}
	else{
		stt = BoxSelect::SEL_NONE;
	}

	pos = min_pos.cast<float>();

	return stt != BoxSelect::SEL_NONE;
}

void VView3DEditBox::resetSelect()
{
	_select.stt = BoxSelect::SEL_NONE;
	_select.dir.setZero();
}



VK_END_NAMESPACE