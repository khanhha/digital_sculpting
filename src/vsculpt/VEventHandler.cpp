#include <QMouseEvent>
#include <QKeyEvent>
#include <iterator>
#include "VEventHandler.h"


extern tbb::mutex g_render_gui_mutex;

bool VKeyMapItem::match(QEvent *ev) const
{
	if (ev && type != QEvent::None){
		if (type != ev->type()) return false;

		bool accept = false;

		/*continue matching*/
		switch (type)
		{
		
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		{
			QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
			/*warning: button is the pressed button, which generate mouse event
			*while mouse is moving, QMouseEvent::button() is NoButton. For getting
			pressed button while mouse is moving, use QMouseEvent::buttons()*/
			if (button == mev->button()){
				accept = (modifier != Qt::NoModifier) ? (modifier == mev->modifiers()) : true;
			}
			break;
		}

		case QEvent::MouseMove:
		{
			QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
			accept = (modifier != Qt::NoModifier) ? (modifier == mev->modifiers()) : true;
			break;
		}

		case QEvent::Wheel:
		{	
			QWheelEvent *mev = dynamic_cast<QWheelEvent*>(ev);
		accept = (modifier != Qt::NoModifier) ? (modifier == mev->modifiers()) : true;
		break;
		}
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		{
			QKeyEvent *kev = dynamic_cast<QKeyEvent*>(ev);
			if (kev->key() == key){
				accept = (modifier != Qt::NoModifier) ? (modifier == kev->modifiers()) : true;
			}
		}
		case QEvent::Drop:
		{
			accept = true;
			break;
		}
		default:
			break;
		}

		return accept;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
VEventHandlerManager::VEventHandlerManager()
{
	register_event_handlers();
}

VEventHandlerManager::~VEventHandlerManager()
{
	for (auto it = _handlers.begin(); it != _handlers.end(); ++it){
		delete *it;
	}
}

void VEventHandlerManager::handle(QEvent *ev)
{
	tbb::mutex::scoped_lock lock(g_render_gui_mutex);

	/*handle modal operator first*/
	for (auto it = _modal_handlers.begin(); it != _modal_handlers.end();){
		VKeyMapItemHandler *handler = *it;
		handle_keyitem(handler, ev);
		if (handler->done){
			/*finally, this keymap finish its operators*/
			it = _modal_handlers.erase(it);
		}
		else{
			++it;
		}
	}

	for (auto it = _handlers.begin(); it != _handlers.end(); ++it){		
		const VEventHandler *evhandle = *it;
		VKeyMap *keymap = evhandle->keymap;
		if (keymap->poll(ev)){
			
			VKeyMap::v_km_item_list &km_items = keymap->items;
			for (auto it = km_items.begin(); it != km_items.end(); ++it){
				VKeyMapItem &item = *it;
				if (item.match(ev)){
					
					/*TODO*/
					/*is it right to put code here*/

					VKeyMapItemHandler *handler = new VKeyMapItemHandler(item);
					handle_keyitem(handler, ev);
					if (handler->done){
						delete handler;
						handler = nullptr;
					}
					else{
						_modal_handlers.push_back(handler);
					}
				}
			}
		}
	}
}

void VEventHandlerManager::handle_keyitem(VKeyMapItemHandler *handler, QEvent *ev)
{
	assert(!handler->done);
	

	if (handler->op_modal_cur){

		auto ret = handler->op_modal_cur->modal(ev);
		
		if (ret == VOperator::OP_RET_FINISHED){
			delete handler->op_modal_cur;
			handler->op_modal_cur = nullptr;
			
			if (handler->op_id_next == handler->op_ids.size()){
				handler->done = true;
			}
		}
	}
	else{

		for (size_t i = handler->op_id_next; i < handler->op_ids.size(); ++i){
			VOpTemplate::OP_TYPE op_id = handler->op_ids[i];

			if (VOpTemplate::op_template(op_id).pollFunc()){

				VOperator *op = VOpTemplate::op_template(op_id).creatorFunc();

				auto ret = op->invoke(ev);

				if (ret == VOperator::OP_RET_FINISHED){
					delete op;
					op = nullptr;
				}
				else if(ret == VOperator::OP_RET_RUNNING){
					/*stop here and keep running this operator for further events until it finishes*/
					handler->op_modal_cur	= op;
					handler->op_id_next		= i + 1; /*remember next operator*/
					return;
				}
			}
		}

		/*finish all item's operators*/
		handler->done = true;
	}
}

void VEventHandlerManager::register_event_handlers()
{

	/*hack: put manipulator operator here for handling it firstly, before any another operator using mouse pressed event*/
	/*TODO: both sculpt and manipulator make use of mouse pressed event, therefore, when manipulator lie
	inside object, we can only invoke only the operator appearing first in event handle array
	so, do we need a priority value for keymap item?*/
	register_manipulator_event_handlers();

	register_base_mesh_event_handlers();
	register_sculpt_mesh_event_handlers();
	register_transform_event_handlers();
	register_other_event_handlers();
}


void VEventHandlerManager::register_manipulator_event_handlers()
{
	VKeyMap *keymap = new VKeyMap(&keymap_callback_transform);
	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::OP_MANIPULATOR);
		item.type = QEvent::MouseButtonPress;
		item.button = Qt::LeftButton;

		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::OP_MANIPULATOR);
		item.type = QEvent::KeyPress;
		item.key = Qt::Key_G;
		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::OP_MANIPULATOR);
		item.type = QEvent::KeyPress;
		item.key = Qt::Key_R;
		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::OP_MANIPULATOR);
		item.type = QEvent::KeyPress;
		item.key = Qt::Key_S;
		keymap->items.push_back(item);
	}

	_handlers.push_back(new VEventHandler(keymap));
}

void VEventHandlerManager::register_base_mesh_event_handlers()
{
}

void VEventHandlerManager::register_transform_event_handlers()
{
	VKeyMap *keymap = new VKeyMap(&keymap_callback_transform);

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::TYPE_VIEW_MOVE);
		item.type = QEvent::MouseButtonPress;
		item.button = Qt::MiddleButton;

		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::TYPE_VIEW_ZOOM);
		item.type = QEvent::Wheel;

		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::TYPE_VIEW_ROTATE);
		item.type = QEvent::MouseButtonPress;
		item.button = Qt::RightButton;

		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::TYPE_VIEW3D_BOX_EDIT);
		item.type = QEvent::MouseButtonPress;
		item.button = Qt::LeftButton;
		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::TYPE_VIEW3D_BOX_EDIT);
		item.type = QEvent::MouseMove;
		keymap->items.push_back(item);
	}

	_handlers.push_back(new VEventHandler(keymap));
}


void VEventHandlerManager::register_other_event_handlers()
{
	VKeyMap *keymap = new VKeyMap(&keymap_callback_transform);

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::TYPE_DROP_MESH);
		item.type = QEvent::Drop;

		keymap->items.push_back(item);
	}

	{
		VKeyMapItem item;
		item.op_ids.push_back(VOpTemplate::OP_MANIPULATOR);
		item.type = QEvent::MouseButtonPress;
		item.button = Qt::LeftButton;

		keymap->items.push_back(item);
	}

	_handlers.push_back(new VEventHandler(keymap));
}

void VEventHandlerManager::register_sculpt_mesh_event_handlers()
{
	VKeyMap *keymap = new VKeyMap(&keymap_callback_sculpt_mode);

	{
		VKeyMapItem item;
		//item.op_ids.push_back(VOpTemplate::TYPE_BMESH_TO_MESH); /*in case of selected object is graph ==> convert to mesh*/
		//item.op_ids.push_back(VOpTemplate::TYPE_POSE_CLEAR);    /*in case of posing object ==> clear pose state*/
		item.op_ids.push_back(VOpTemplate::TYPE_SCULPT_BRUSH);
		item.type = QEvent::MouseButtonPress;
		item.button = Qt::LeftButton;

		keymap->items.push_back(item);
	}



	_handlers.push_back(new VEventHandler(keymap));
}

bool VEventHandlerManager::keymap_callback_base_mesh_mode(QEvent *ev)
{
	return true;
}

bool VEventHandlerManager::keymap_callback_sculpt_mode(QEvent *ev)
{
	return true;

}

bool VEventHandlerManager::keymap_callback_other_modes(QEvent *ev)
{
	return true;
}

bool VEventHandlerManager::keymap_callback_transform(QEvent *ev)
{
	return true;
}

