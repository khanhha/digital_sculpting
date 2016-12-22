#ifndef SCULPT_STROKE_H
#define SCULPT_STROKE_H

#include <vector>
#include "StrokeData.h"
#include "SculptCommand.h"
#include "commonDefine.h"
#include <Eigen/Dense>
#include <tuple>

class SculptStroke
{
	class StepLogger
	{
		typedef std::tuple<BMVert*, BMVert*, BMVert> FaceLog;
		typedef std::tuple<BMVert*, Vector3f> VertLog;
	public:
		StepLogger(const std::vector<BMLeafNode*> &nodes);
		void restore();
	private:
		std::vector<BMLeafNode*> _changed_nodes;
		std::vector<FaceLog>	 _deleted_faces;
		std::vector<FaceLog>	 _created_faces;
		std::vector<VertLog>	 _changed_verts;
	};

public:
	SculptStroke(StrokeData *sdata);
	~SculptStroke();
	void add_step(bool log_step = false);
	void finish_stroke();
	void step_logger_begin();
	void step_logger_end();
private:
	void calc_symm_data(size_t symm);
	bool is_dynamic_topology();
	void update_topology();
	void push_undo_redo();
	void save_origin_node_data(const std::vector<BMLeafNode*> &nodes);
	void do_brush();
private:
	StrokeData		*_data;
	VSculptLogger	*_step_logger;
};
#endif