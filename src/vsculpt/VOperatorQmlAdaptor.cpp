#include "VOperatorQmlAdaptor.h"
#include <QDebug>

VOperatorQmlAdaptor::VOperatorQmlAdaptor()
	:
	_op(nullptr)
{}

VOperatorQmlAdaptor::~VOperatorQmlAdaptor()
{
	if (_op) delete _op;
}

VOpTemplate::OP_TYPE VOperatorQmlAdaptor::opid()
{
	return _opid;
}

void VOperatorQmlAdaptor::setOpid(VOpTemplate::OP_TYPE &id)
{
	if (id != _opid){
		_opid = id;
		
		if (_op != nullptr){
			delete _op;
			_op = nullptr;
		}
		else{
			_op = VOpTemplate::op_template(_opid).creatorFunc();
		}

		Q_EMIT opidChanged();
	}
}

QVariant VOperatorQmlAdaptor::query(const QString &name)
{
	if (_op)
		return _op->query(name);
	else
		return QVariant();
}

void VOperatorQmlAdaptor::setParam(const QString &name, const QVariant &val)
{
	if (_op)
		_op->setParam(name, val);
}

Q_INVOKABLE void VOperatorQmlAdaptor::invoke()
{
	if (_op)
		_op->invoke(nullptr);
}

Q_INVOKABLE void VOperatorQmlAdaptor::execute()
{
	if (_op)
		_op->execute();
}

Q_INVOKABLE void VOperatorQmlAdaptor::cancel()
{
	if (_op)
		_op->cancel();
}

