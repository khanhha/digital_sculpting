/*
* ***** BEGIN GPL LICENSE BLOCK *****
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
* Contributor(s): Joseph Eagar, Geoffrey Bantle, Campbell Barton
*
* ***** END GPL LICENSE BLOCK *****
*/

/** \file blender/bmesh/intern/bmesh_inline.h
*  \ingroup bmesh
*
* BM Inline functions.
*/

#ifndef __BMESH_INLINE_H__
#define __BMESH_INLINE_H__

#include "BMUtilDefine.h"
#include "BMeshClass.h"

VM_BEGIN_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////
//private
/**
* Internal BMHeader.api_flag
* \note Ensure different parts of the API do not conflict
* on using these internal flags!*/
enum {
	_FLAG_JF = (1 << 0),  /* join faces */
	_FLAG_MF = (1 << 1),  /* make face */
	_FLAG_MV = (1 << 1),  /* make face, vertex */
	_FLAG_OVERLAP = (1 << 2),  /* general overlap flag  */
	_FLAG_WALK = (1 << 3),  /* general walk flag (keep clean) */
};
template<typename T>
void BM_ELEM_API_FLAG_ENABLE(T *elm, int flag){ elm->head.api_flag |= flag; }
template<typename T>
void BM_ELEM_API_FLAG_DISABLE(T *elm, int flag){ elm->head.api_flag &= ~flag; }
template<typename T>
bool BM_ELEM_API_FLAG_TEST(T *elm, int flag){ return (elm->head.api_flag & flag) != 0; }
template<typename T>
void BM_ELEM_API_FLAG_CLEAR(T *elm){ elm->head.api_flag = 0; }
/////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
void BM_elem_app_flag_enable(T *elm, int flag){elm->head.app_flag |= flag;}
template<typename T>
void BM_elem_app_flag_disable(T *elm, int flag){ elm->head.app_flag &= ~flag; }
template<typename T>
bool BM_elem_app_flag_test(T *elm, int flag){ return (elm->head.app_flag & flag) != 0; }
template<typename T>
void BM_elem_app_flag_clear(T *elm){ elm->head.app_flag = 0; }
template<typename T>
char BM_elem_app_flag_value(T *elm){ return elm->head.app_flag; }


/* stuff for dealing with header flags */
template<typename T>
char BM_elem_flag_test(const T *elm, const char hflag)
{
	return elm->head.hflag & hflag;
}
template<typename T>
bool BM_elem_flag_test_bool(const T *elm, const char hflag)
{
	return (elm->head.hflag & hflag) != 0;
}
template<typename T>
void BM_elem_flag_enable(T *elm, const char hflag)
{
	elm->head.hflag |= hflag;
}
template<typename T>
void BM_elem_flag_disable(T *elm, const char hflag)
{
	elm->head.hflag &= (char)~hflag;
}
template<typename T>
void BM_elem_flag_set(T *elm, const char hflag, const int val)
{
	if (val)  BM_elem_flag_enable(elm, hflag);
	else      BM_elem_flag_disable(elm, hflag);
}
template<typename T>
void BM_elem_flag_toggle(T *elm, const char hflag)
{
	elm->head.hflag ^= hflag;
}
template<typename TA, typename TB>
void BM_elem_flag_merge(TA *elma, TB *elmb)
{
	elma->head.hflag = elmb->head.hflag = elma->head.hflag | elmb->head.hflag;
}
template<typename TC, typename TA, typename TB>
void BM_elem_flag_merge_into(TC *elm, const TA *elma, const TB *elmb)
{
	elm->head.hflag = elma->head.hflag | elmb->head.hflag;
}

	/**
	* notes on #BM_elem_index_set(...) usage,
	* Set index is sometimes abused as temp storage, other times we cant be
	* sure if the index values are valid because certain operations have modified
	* the mesh structure.
	*
	* To set the elements to valid indices 'BM_mesh_elem_index_ensure' should be used
	* rather then adding inline loops, however there are cases where we still
	* set the index directly
	*
	* In an attempt to manage this,
	* here are 5 tags I'm adding to uses of #BM_elem_index_set
	*
	* - 'set_inline'  -- since the data is already being looped over set to a
	*                    valid value inline.
	*
	* - 'set_dirty!'  -- intentionally sets the index to an invalid value,
	*                    flagging 'bm->elem_index_dirty' so we don't use it.
	*
	* - 'set_ok'      -- this is valid use since the part of the code is low level.
	*
	* - 'set_ok_invalid'  -- set to -1 on purpose since this should not be
	*                    used without a full array re-index, do this on
	*                    adding new vert/edge/faces since they may be added at
	*                    the end of the array.
	*
	* - campbell */

template<typename T>
void BM_elem_index_set(T *elm, const int index)
{
	elm->head.index = index;
}
template<typename T>
int BM_elem_index_get(const T *elm)
{
	return elm->head.index;
}

template<typename T>
int& BM_elem_aux_data_int_get(T* elm, const int &pos)
{
	return (reinterpret_cast<int*>(elm->head.aux_data.data()))[pos];
}

template<typename T>
void BM_elem_aux_data_int_set(T* elm, const int &pos, const int &val)
{
	(reinterpret_cast<int*>(elm->head.aux_data.data()))[pos] = val;
}

VM_END_NAMESPACE
#endif /* __BMESH_INLINE_H__ */
