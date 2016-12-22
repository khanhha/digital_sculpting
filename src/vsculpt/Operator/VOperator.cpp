#include "VOperator.h"
#include "VViewEditOp.h"
#include "VDropMeshOp.h"
#include "VSculptBrushOp.h"
#include "VView3DEditBoxOp.h"

QVariant VOperator::param(const QString &name)
{
	if (_params.find(name) != _params.end()){
		return _params[name];
	}
	return QVariant(QVariant::Invalid);
}

QVariant VOperator::query(const QString &name)
{
	return QVariant();
}

QVariantList VOperator::queryList(const QString &name)
{
	return QVariantList();
}

QVariantMap VOperator::queryMap(const QString &name)
{
	return QVariantMap();
}

