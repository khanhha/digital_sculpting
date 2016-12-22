#ifndef VSCULPT_VOPERATOR_H
#define VSCULPT_VOPERATOR_H

#include <QEvent>
#include "VKernel/VScene.h"
#include <memory>
#include <vector>
#include <functional>
#include <QVariantMap>

using namespace  vk;
class VOperator
{
public:
	/*OP type*/
	typedef enum RET_STATE
	{
		OP_RET_NONE = 1 << 0,
		OP_RET_FINISHED = 1 << 1,
		OP_RET_RUNNING = 1 << 2
	}RET_STATE;
public:
	VOperator(){};
	virtual ~VOperator(){};

	virtual int  invoke(QEvent *ev){ return OP_RET_NONE; };
	virtual int  modal(QEvent *ev){ return OP_RET_NONE; };
	virtual void cancel(){};
	virtual void execute(){};
	virtual void setParam(const QString &name, const QVariant &value){ _params[name] = value; }
	QVariant	 param(const QString &name);

	/*for QML UI query information*/
	virtual QVariant query(const QString &name);
	virtual QVariantList queryList(const QString &name);
	virtual QVariantMap  queryMap(const QString &name);
protected:
	QVariantMap _params;
};

#endif