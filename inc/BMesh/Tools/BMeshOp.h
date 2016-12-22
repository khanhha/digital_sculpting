#ifndef BMESH_BMESH_OP_H
#define BMESH_BMESH_OP_H

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <vector>
#include <iterator>
#include <Eigen/Dense>

#include "BMesh/BMesh.h"

VM_BEGIN_NAMESPACE

enum BMOpSlotElemType
{
	SLOT_BOOL,
	SLOT_FLOAT,
	SLOT_INT,
	SLOT_VEC3,
	SLOT_VERT_ARRAY,
	SLOT_EDGE_ARRAY,
	SLOT_FACE_ARRAY
};

struct BMOpSlot
{
	typedef boost::unordered_map<void*, void*>					mapping_elem;
	typedef std::vector<BMVert*>								vert_array;
	typedef std::vector<BMEdge*>								edge_array;
	typedef std::vector<BMFace*>								face_array;
	typedef boost::variant<
		bool, int, float, Vector3f, 
		mapping_elem, vert_array, edge_array, face_array>		slot_data_type;


	BMOpSlot() :name(""){}

	BMOpSlot(const BMOpSlot &other)
		:name(other.name), data(other.data){}

	BMOpSlot& operator=(const BMOpSlot &other)
	{
		name = other.name;
		data = other.data;
	}

	bool operator==(const BMOpSlot &other)
	{
		return name == other.name;
	}

	std::string		name;
	slot_data_type	data;
};

struct BMOSlotType
{
	std::string name;
};

class BMeshOp
{
	typedef std::vector<BMOpSlot>::iterator slot_iter;
public:
	void in_slot_set(const BMOpSlot &slot)
	{
		slot_iter pos;
		if (!in_slot_exist(slot.name, pos)){
			_in_slots.push_back(slot);
		}
	}

	template<typename ElemType>
	void in_slot_data_set(const std::string &name, ElemType &elem)
	{
		slot_iter pos;
		if (!in_slot_exist(name, pos))
		{
			_in_slots.push_back(BMOpSlot());
			BMOpSlot& slot = _in_slots.back();
			slot.name = name;
			slot.data = elem;
		}
	}
	
	template<typename ElemType>
	void out_slot_data_set(const std::string &name, ElemType &elem)
	{
		slot_iter pos;
		if (!out_slot_exist(name, pos))
		{
			_out_slots.push_back(BMOpSlot());
			BMOpSlot& slot = _in_slots.back();
			slot.name = name;
			slot.data = elem;
		}
	}

	template<typename ElemType>
	boost::optional<ElemType>  out_slot_data_get(const std::string &name)
	{
		slot_iter pos;
		if (out_slot_exist(name, pos))
		{
			return boost::get<ElemType>(pos->data);
		}
	}

	template<typename ElemType>
	boost::optional<ElemType> in_slot_data_get(const std::string &name)
	{
		slot_iter pos;
		if (in_slot_exist(name, pos))
		{
			return boost::get<ElemType>(pos->data);
		}
	}

	bool out_slot_exist(const std::string &name, slot_iter &pos)
	{
		pos = std::find_if(_out_slots.begin(), _out_slots.end(), [&](const BMOpSlot &slot) { return slot.name == name; });
		return pos != std::end(_out_slots);
	}

	bool in_slot_exist(const std::string &name, slot_iter &pos)
	{
		pos = std::find_if(_in_slots.begin(), _in_slots.end(), [&](const BMOpSlot &slot){ return slot.name == name; });
		return pos != std::end(_in_slots);
	}

	virtual void execute(){};
private:
	BMesh					*_bm;
	std::vector<BMOpSlot>	_in_slots;
	std::vector<BMOpSlot>	_out_slots;
};

VM_END_NAMESPACE

#endif
