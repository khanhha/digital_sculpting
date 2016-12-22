#ifndef VSCULPT_VEVENT_MANAGER__H
#define VSCULPT_VEVENT_MANAGER__H
#include <list>
#include <QtGlobal>
#include <QEvent>
#include <qcoreevent.h>
#include <qnamespace.h>
#include "VKernel/VScene.h"
#include "Operator/VOperator.h"
#include "Operator/VOpTemplate.h"

using namespace vk;

class VKeyMapItem
{
	typedef VOpTemplate::OP_TYPE VOpID;
	typedef QEvent::Type		VEvType;
public:
	VKeyMapItem()
		:
		type(QEvent::None), button(Qt::NoButton), modifier(Qt::NoModifier), key(Qt::Key_unknown)
	{}
	bool match(QEvent *ev) const;
public:
	std::vector<VOpID>	 op_ids;
	VEvType				 type;
	Qt::MouseButton		 button;
	Qt::KeyboardModifier modifier;
	Qt::Key				 key;
};


class VKeyMapItemHandler
{
public:
	VKeyMapItemHandler(VKeyMapItem &item_) 
		: 
		item(item_),
		op_ids(item_.op_ids),
		op_id_next(0),
		op_modal_cur(nullptr),
		done(false)
	{
	};
public:
	VKeyMapItem &item;
	const std::vector<VOpTemplate::OP_TYPE> &op_ids;

	size_t		 op_id_next;
	VOperator	*op_modal_cur;
	bool		 done;
};

class VKeyMap
{
public:
	typedef std::function<bool(QEvent*)>	v_poll_func;
	typedef std::list<VKeyMapItem>			v_km_item_list;
	typedef v_km_item_list::iterator		v_km_item_handle;
	typedef v_km_item_list::const_iterator	v_km_item_const_handle;
public:
	VKeyMap(v_poll_func poll_)
		:
		poll(poll_)
	{};
public:
	v_poll_func		poll;
	v_km_item_list	items;
};

class VEventHandler
{
	typedef std::vector<VOpTemplate::VOperatorType> OpTypeList;
	typedef VKeyMap::v_km_item_list v_km_item_list;
public:
	VEventHandler(VKeyMap *keymap_) : keymap(keymap_){}
	~VEventHandler(){ delete keymap; };
public:
	VKeyMap	*keymap;
};

class VEventHandlerManager
{
	typedef VOpTemplate::VOperatorType VOperatorType;
public:
	VEventHandlerManager();
	~VEventHandlerManager();
	void handle(QEvent *ev);
private:
	void handle_keyitem(VKeyMapItemHandler *keyitem_handler, QEvent *ev);
	void register_event_handlers();
	void register_manipulator_event_handlers();
	void register_base_mesh_event_handlers();
	void register_sculpt_mesh_event_handlers();
	void register_transform_event_handlers();
	void register_other_event_handlers();
	void register_no_ui_event_handlers();
private:
	static bool keymap_callback_base_mesh_mode(QEvent *ev);
	static bool keymap_callback_sculpt_mode(QEvent *ev);
	static bool keymap_callback_other_modes(QEvent *ev);
	static bool keymap_callback_transform(QEvent *ev);
private:
	std::list<VEventHandler*>				_handlers;

	std::list<VKeyMapItemHandler*>			_modal_handlers;
};

#endif