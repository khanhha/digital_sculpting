#ifndef SCULPT_STROKE_DATA_H
#define SCULPT_STROKE_DATA_H
#include <Eigen/Dense>
#include "BezierCurve.h"
#include "VKernel/VMeshObject.h"
#include "Vbvh/BMeshBvh.h"
#include "VKernel/VScene.h"
#include "VKernel/VMeshObject.h"
using namespace VBvh;

enum SYMNFlag
{
    SYMM_X = (1 << 0),
    SYMM_Y = (1 << 1),
    SYMM_Z = (1 << 2)
};
struct SymmetricData
{
    Vector3f first_hit_pos;
	Vector3f cur_pos;
	Vector3f view_dir;
	Vector3f grab_delta;
};

class BVHRenderer;
class VSculptCommand;

struct StrokeData
{
	vk::VScene			*scene;
	vk::VMeshObject		*object;
	BMBvh			*bvh;
	BMesh			*bm;
	VSculptCommand	*undo_redo_logger;

    SymmetricData symn_data;
    Vector3f last_pos;
	Vector3f cur_pos;
	Vector3f first_hit_pos;
	Vector3f view_dir;
	Vector3f grab_delta;
	
	int brush_type;
    int flag;
	int sym_flag; //symm is a bit combination of XYZ - 1 is mirror X; 2 is Y; 3 is XY; 4 is Z; 5 is XZ; 6 is YZ; 7 is XYZ

	float world_radius;
	float rotate_angle; /*angle between first mouse and current mouse direction. Currently for rotate brush*/
	float max_edge_len;
	float min_edge_len;
	float edge_len_unit_threshold;

	float scale;
	float pinch_factor;
	float brush_strength;

    bool ensure_min_edge_len;

	BezierCurve *deform_curve;
};
#endif