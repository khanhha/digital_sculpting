#ifndef VSCULPT_VQML_ADAPTOR_H
#define VSCULPT_VQML_ADAPTOR_H

#include <QVariant>
#include <QObject>
#include "Operator/VOperator.h"
#include "Operator/VOpTemplate.h"
class VOperatorQmlAdaptor : public QObject {
	Q_OBJECT
	Q_PROPERTY(VOpTemplate::OP_TYPE opid READ opid WRITE setOpid NOTIFY opidChanged)
public:
	VOperatorQmlAdaptor();
	~VOperatorQmlAdaptor();
public:
	VOpTemplate::OP_TYPE opid();
	void  setOpid(VOpTemplate::OP_TYPE &id);
Q_SIGNALS:
	void opidChanged();
public:
	Q_INVOKABLE QVariant		query(const QString &name);
	Q_INVOKABLE void			setParam(const QString &name, const QVariant &val);
	Q_INVOKABLE void	 invoke();
	Q_INVOKABLE void	 execute();
	Q_INVOKABLE void	 cancel();
private:
	VOpTemplate::OP_TYPE	_opid;
	VOperator *_op;
};
#endif