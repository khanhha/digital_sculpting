#include "vsculpt/QtQuickAppViewer.h"
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include "VView3DRegionQuickItem.h"
#include "VContext.h"
#include "VOperatorQmlAdaptor.h"
#include "Operator/VOpTemplate.h"
#include "VbsQt/VbsDef.h"
#include <QUrl>
#include<tbb/mutex.h>
#include <QOpenGLDebugLogger>
#include <QQmlContext>

//#include <openvdb/openvdb.h>

tbb::mutex g_render_gui_mutex;

#if 0
void testVDB()
{
    // Initialize the OpenVDB library.  This must be called at least
    // once per program and may safely be called multiple times.
    openvdb::initialize();
    // Create an empty floating-point grid with background value 0. 
    openvdb::FloatGrid::Ptr grid = openvdb::FloatGrid::create();
    std::cout << "Testing random access:" << std::endl;
    // Get an accessor for coordinate-based access to voxels.
    openvdb::FloatGrid::Accessor accessor = grid->getAccessor();
    // Define a coordinate with large signed indices.
    openvdb::Coord xyz(1000, -200000000, 30000000);

    // Set the voxel value at (1000, -200000000, 30000000) to 1.
    accessor.setValue(xyz, 1.0);

    // Verify that the voxel value at (1000, -200000000, 30000000) is 1.
    std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;

    // Reset the coordinates to those of a different voxel.
    xyz.reset(1000, 200000000, -30000000);

    // Verify that the voxel value at (1000, 200000000, -30000000) is
    // the background value, 0.
    std::cout << "Grid" << xyz << " = " << accessor.getValue(xyz) << std::endl;

    // Set the voxel value at (1000, 200000000, -30000000) to 2.
    accessor.setValue(xyz, 2.0);
    // Set the voxels at the two extremes of the available coordinate space.
    // For 32-bit signed coordinates these are (-2147483648, -2147483648, -2147483648)
    // and (2147483647, 2147483647, 2147483647).
    accessor.setValue(openvdb::Coord::min(), 3.0f);
    accessor.setValue(openvdb::Coord::max(), 4.0f);
    std::cout << "Testing sequential access:" << std::endl;
    // Print all active ("on") voxels by means of an iterator.
    for (openvdb::FloatGrid::ValueOnCIter iter = grid->cbeginValueOn(); iter; ++iter) {
        std::cout << "Grid" << iter.getCoord() << " = " << *iter << std::endl;
    }
}
#endif
int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);

	VOpTemplate::register_operators();

	qmlRegisterType<VView3DRegionQuickItem>("vsculpt_qt5", 1, 0, "VRenderQuickItem");
	qmlRegisterType<VOperatorQmlAdaptor>("com.k3d.qmladapter", 1, 0, "VOperatorQmlAdaptor");
	qmlRegisterType<VOpTemplate>("com.k3d.qmladapter", 1, 0, "VOpTemplate");
	qmlRegisterType<VbsDef>("com.k3d.qmladapter", 1, 0, "VbsDef");



	qmlRegisterSingletonType(QUrl("qrc:///qml/MainGBSingletonObject.qml"), "MainGBSingletonObject", 1, 0, "MAINGB");

	VContext *context = VContext::instance();

 	QtQuickAppViewer viewer;
	viewer.setSource(QUrl("qrc:///qml/main.qml"));
	viewer.showMaximized();

	viewer.rootContext()->setContextProperty("VScene",context->scene());

	context->setWindow(&viewer);

	return app.exec();
}
