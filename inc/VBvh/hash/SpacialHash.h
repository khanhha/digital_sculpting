#ifndef VBVH_SPACIAL_HASH_H
#define VBVH_SPACIAL_HASH_H
#include "VBvh/VBvhDefine.h"
#include "VBvh/hash/GridUtil.h"
#include <Eigen/Dense>
#include <functional>
#include <boost/unordered_map.hpp>
VBVH_BEGIN_NAMESPACE
// hashing function
struct SpacialHashFunctor : public std::unary_function<Eigen::Vector3i, size_t>
{
	enum
	{ // parameters for hash table
		bucket_size = 4, // 0 < bucket_size
		min_buckets = 8
	};

	size_t operator()(const Eigen::Vector3i &p) const
	{
		const size_t _HASH_P0 = 73856093u;
		const size_t _HASH_P1 = 19349663u;
		const size_t _HASH_P2 = 83492791u;

		return size_t(p[0])*_HASH_P0 ^  size_t(p[1])*_HASH_P1 ^  size_t(p[2])*_HASH_P2;
	}

	bool operator()(const  Eigen::Vector3i &s1, const  Eigen::Vector3i &s2) const
	{ 
		// test if s1 ordered before s2
		return	(s1.coeff(2) != s2.coeff(2)) ? (s1.coeff(2) < s2.coeff(2)) :
											   (s1.coeff(1) != s2.coeff(1)) ? (s1.coeff(1) < s2.coeff(1)) : (s1.coeff(0) < s2.coeff(0));
	}
};

template<typename ObjType, typename FLT = double>
class SpacialHash : public BasicGrid<FLT>
{
public:
	typedef ObjType* ObjPtr;
	typedef typename boost::unordered_multimap<Eigen::Vector3i, ObjPtr, SpacialHashFunctor> HashType;
	typedef typename HashType::iterator HashIterator;
	typedef typename ObjType::RealBox	RealBox;
	typedef typename ObjType::IndexBox	IndexBox;
	HashType hash_table; // The real HASH TABLE ***************

	SpacialHash(){}
	~SpacialHash()
	{
	 }

	void InsertObject(ObjPtr s, const Eigen::Vector3i &cell)
	{
		//if(hash_table.count(cell)==0) AllocatedCells.push_back(cell);
		hash_table.insert(typename HashType::value_type(cell, s));
	}

	void Add(ObjPtr s)
	{
		const RealBox  &b = s->bbox();
		IndexBox bb;
		BoxToIBox(b, bb);

		//then insert all the cell of bb
		for (int i = bb.min().x(); i <= bb.max().x(); i++)
			for (int j = bb.min().y(); j <= bb.max().y(); j++)
				for (int k = bb.min().z(); k <= bb.max().z(); k++){
					InsertObject(s, Eigen::Vector3i(i, j, k));
				}
	}

	template <class OBJITER>
	void Set(const OBJITER &_oBegin, const OBJITER & _oEnd, const RealBox &_bbox = RealBox())
	{
		OBJITER i;
		Box3x b;
		Box3x &bbox = this->bbox;
		CoordType &dim = this->dim;
		Eigen::Vector3i &siz = this->siz;
		CoordType &voxel = this->voxel;

		int _size = (int)std::distance<OBJITER>(_oBegin, _oEnd);
		if (!_bbox.isNull()) {
			this->bbox = _bbox;
		}
		else{
			for (i = _oBegin; i != _oEnd; ++i){
				this->bbox.extend((*i).bbox());
			}
			///inflate the bb calculated
			//CoordType delta = bbox.diagonal() / 100.0;
			//bbox.min() -= delta;
			//bbox.max() += delta;
		}

		dim = bbox.max() - bbox.min();
		BestDim(_size, dim, siz);
		// find voxel size
		voxel[0] = dim[0] / siz[0];
		voxel[1] = dim[1] / siz[1];
		voxel[2] = dim[2] / siz[2];

		for (i = _oBegin; i != _oEnd; ++i)
			Add(&(*i));
	}
};

VBVH_END_NAMESPACE

#endif