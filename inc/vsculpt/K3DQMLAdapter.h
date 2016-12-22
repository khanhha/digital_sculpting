#ifndef K3D_QML_ADAPTER_H
#define K3D_QML_ADAPTER_H

#include <QObject>
#include "guiutil.h"

class QJSEngine;
class QQmlEngine;

// =======================================================================================================================
//
typedef QVariant (MainWindow::*K3DOP)(QVariantList);
//
typedef QVariant (MainWindow::*K3DOP_VOID)(void);
typedef QVariant (MainWindow::*K3DOP_VAR)(QVariant);
typedef QVariant (MainWindow::*K3DOP_VAR_VAR)(QVariant, QVariant);

// GENERAL PARAM LIST --
#define K3D_OPERATION_INVOKE(k3dOpId, paramList)                                                \
        if(_k3dOp[k3dOpId] != nullptr)          {                                               \
            return K3D_MEMFUNC_CALL(*MainWindow::getInstance(), _k3dOp[k3dOpId])(paramList);    \
        }                                                                                       \
        else {                                                                                  \
            KLOG_DEBUG() << "Mem func not set!"; return -1;                                     \
        }

// VOID PARAM --
#define K3D_VOID_OPERATION_INVOKE(k3dOpId)                                                      \
        if(_k3dOpVoid[k3dOpId] != nullptr)          {                                           \
            return K3D_MEMFUNC_CALL(*MainWindow::getInstance(), _k3dOpVoid[k3dOpId])();         \
        }                                                                                       \
        else {                                                                                  \
            KLOG_DEBUG() << "Mem func not set!"; return -1;                                     \
        }

// SINGLE VARIANT PARAM --
#define K3D_1VARIANT_OPERATION_INVOKE(k3dOpId, param)                                           \
        if(_k3dOpVariant[k3dOpId] != nullptr)       {                                           \
            return K3D_MEMFUNC_CALL(*MainWindow::getInstance(), _k3dOpVariant[k3dOpId])(param); \
        }                                                                                       \
        else {                                                                                  \
            KLOG_DEBUG() << "Mem func not set!";  return -1;                                    \
        }

// DOUBLE VARIANT PARAM --
#define K3D_2VARIANT_OPERATION_INVOKE(k3dOpId, param1, param2)                                                 \
        if(_k3dOpDoubleVariant[k3dOpId] != nullptr) {                                                          \
            return K3D_MEMFUNC_CALL(*MainWindow::getInstance(), _k3dOpDoubleVariant[k3dOpId])(param1, param2); \
        }                                                                                                      \
        else {                                                                                                 \
            KLOG_DEBUG() << "Mem func not set!";  return -1;                                                   \
        }

// DIALOG OPERATION --
#define K3D_DIALOG_OPERATION_INVOKE(k3dDialogId, eventType)                                                                    \
        if(_k3dOpDialogAgent != nullptr)            {                                                                          \
            return K3D_MEMFUNC_CALL(*MainWindow::getInstance(), _k3dOpDialogAgent)(QVariant(k3dDialogId), QVariant(eventType));\
        }                                                                                                                      \
        else {                                                                                                                 \
            KLOG_DEBUG() << "Mem func not set!"; return -1;                                                                    \
        }


// -----------------------------------------------------------------------------------
// K3DBUILDER's LOCAL <QML_ITEM>.QML funcs:
#define K3D_QML_DIALOG_INVOKE(qmlDialogId, func)                                                      \
        QMetaObject::invokeMethod(K3DQMLAdapter::getInstance()->getK3DQMLDialog(qmlDialogId), #func);
#define K3D_QML_DIALOG_LOCAL_INVOKE(func)                                                             \
        QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func);

#define K3D_QML_DIALOG_INVOKE_I(qmlDialogId, func, param)                                             \
        QMetaObject::invokeMethod(K3DQMLAdapter::getInstance()->getK3DQMLDialog(qmlDialogId), #func,  \
                                  Q_ARG(QVariant, param));
#define K3D_QML_DIALOG_LOCAL_INVOKE_I(func, param)                                                    \
        QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func,                                 \
                                  Q_ARG(QVariant, param));

#define K3D_QML_DIALOG_INVOKE_II(qmlDialogId, func, param1, param2)                                   \
        QMetaObject::invokeMethod(K3DQMLAdapter::getInstance()->getK3DQMLDialog(qmlDialogId), #func,  \
                                  Q_ARG(QVariant, param1),                                            \
                                  Q_ARG(QVariant, param2));
#define K3D_QML_DIALOG_LOCAL_INVOKE_II(func, param1, param2)                                          \
        QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func,                                 \
                                  Q_ARG(QVariant, param1),                                            \
                                  Q_ARG(QVariant, param2));

#define K3D_QML_DIALOG_INVOKE_III(qmlDialogId, func, param1, param2, param3)                          \
        QMetaObject::invokeMethod(K3DQMLAdapter::getInstance()->getK3DQMLDialog(qmlDialogId), #func,  \
                                  Q_ARG(QVariant, param1),                                            \
                                  Q_ARG(QVariant, param2),                                            \
                                  Q_ARG(QVariant, param3));
#define K3D_QML_DIALOG_LOCAL_INVOKE_III(func, param1, param2, param3)                                 \
        QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func,                                 \
                                  Q_ARG(QVariant, param1),                                            \
                                  Q_ARG(QVariant, param2),                                            \
                                  Q_ARG(QVariant, param3));

#define K3D_QML_DIALOG_LOCAL_INVOKE_IX(func, param1, param2, param3, param4, param5, param6, param7, param8, param9) \
        QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func,                                 \
                                  Q_ARG(QVariant, param1),                                            \
                                  Q_ARG(QVariant, param2),                                            \
                                  Q_ARG(QVariant, param3),                                            \
                                  Q_ARG(QVariant, param4),                                            \
                                  Q_ARG(QVariant, param5),                                            \
                                  Q_ARG(QVariant, param6),                                            \
                                  Q_ARG(QVariant, param7),                                            \
                                  Q_ARG(QVariant, param8),                                            \
                                  Q_ARG(QVariant, param9));

#define K3D_QML_DIALOG_INVOKE_RET(qmlDialogId, func, ret)                                             \
        QMetaObject::invokeMethod(K3DQMLAdapter::getInstance()->getK3DQMLDialog(qmlDialogId), #func,  \
                                  Q_RETURN_ARG(QVariant, ret));
#define K3D_QML_DIALOG_LOCAL_INVOKE_RET(func, ret)                                                    \
        QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func,                                 \
                                  Q_RETURN_ARG(QVariant, ret));

#define K3D_QML_DIALOG_INVOKE_RET_I(qmlDialogId, func, ret, param)                                    \
        QMetaObject::invokeMethod(K3DQMLAdapter::getInstance()->getK3DQMLDialog(qmlDialogId), #func,  \
                                  Q_RETURN_ARG(QVariant, ret),                                        \
                                  Q_ARG(QVariant, param));
#define K3D_QML_DIALOG_LOCAL_INVOKE_RET_I(func, ret, param)                                           \
        QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func,                                 \
                                  Q_RETURN_ARG(QVariant, ret),                                        \
                                  Q_ARG(QVariant, param));

#define K3D_QML_DIALOG_INVOKE_RET_II(qmlDialogId, func, ret, param1, param2)                          \
        QMetaObject::invokeMethod(K3DQMLAdapter::getInstance()->getK3DQMLDialog(qmlDialogId), #func,  \
                                  Q_RETURN_ARG(QVariant, ret),                                        \
                                  Q_ARG(QVariant, param1),                                            \
                                  Q_ARG(QVariant, param2));
#define K3D_QML_DIALOG_LOCAL_INVOKE_RET_II(func, ret, param1, param2)                                 \
         QMetaObject::invokeMethod(this->K3DQMLItemAgent::UI(), #func,                                \
                                  Q_RETURN_ARG(QVariant, ret),                                        \
                                  Q_ARG(QVariant, param1),                                            \
                                  Q_ARG(QVariant, param2));

class K3DQMLAdapter : public QObject {
    Q_OBJECT
    
    Q_ENUMS(K3D_DEVICE_TYPE)
    Q_ENUMS(K3D_OPERATOR_TYPE)

    Q_ENUMS(K3D_TAB)
    Q_ENUMS(K3D_OPERATION)
    Q_ENUMS(K3D_VOID_OPERATION)
    Q_ENUMS(K3D_PARAM_OPERATION)
    Q_ENUMS(K3D_DOUBLE_PARAM_OPERATION)
    Q_ENUMS(K3D_TRIPLE_PARAM_OPERATION)

    Q_ENUMS(K3D_CONTEXT_MENU_GENERAL)
    Q_ENUMS(K3D_CONTEXT_MENU_INDIVIDUAL)
    Q_ENUMS(K3D_CONTEXT_MENU_REPAIR)
    Q_ENUMS(K3D_CONTEXT_MENU_SUPPORT)
    Q_ENUMS(K3D_CONTEXT_MENU_VIEW_MODE)
    
    Q_ENUMS(K3D_DIALOG)
    Q_ENUMS(K3D_SUPPORT_TYPE)
    Q_ENUMS(K3D_COLUMN_SUPPORT_TYPE)
    Q_ENUMS(K3D_ACTION)

    Q_ENUMS(K3D_QUICK_VIEW_OBJECT_VIEW_MODE)

    Q_ENUMS(K3D_PRINT_ISSUE)

public:
    enum K3D_DEVICE_TYPE {
        DEVICE_UNKNOWN  = CommonDevice::UNKNOWN,
        DEVICE_N01      = CommonDevice::N01,
        DEVICE_N02      = CommonDevice::N02,
        DEVICE_N02_MINI = CommonDevice::N02_MINI,

        DEVICE_TOTAL    = CommonDevice::TOTAL
    };

    enum K3D_OPERATOR_TYPE {
        K3D_OP_VOID,
        K3D_OP_PARAM,
        K3D_OP_DOUBLE_PARAM,

        K3D_OP_TYPE_TOTAL
    };

    enum K3D_TAB {
        TAB_VOID        = 0x00,
        TAB_MAIN_SCREEN = 0x01,
        TAB_SUPPORT     = 0x02,
        TAB_REPAIR      = 0x04,
        TABS
    };

    static const int OBJECT_IMAGE_WIDTH  = 80;
    static const int OBJECT_IMAGE_HEIGHT = 80;

    enum K3D_ACTION {
        K3D_ACTION_VOID,

        // Main Tab Bar
        K3D_ACTION_MAIN_TABS_SELECT,
        K3D_ACTION_MAIN_TABS_CLOSE_CURRENT,
        K3D_ACTION_MAIN_TABS_SWITCH_PREV,
        K3D_ACTION_MAIN_TABS_SWITCH_NEXT,

        // Main machine bar
        // 

        // Main Tool Bar
        K3D_MAIN_TOOL_OBJECT_BAR, 
        K3D_MAIN_TOOL_REPAIR_BAR,
        K3D_MAIN_TOOL_SUPPORT_BAR,

        // Main File Menu Panel
        K3D_MAIN_FILE_MENU_PANEL,

        // QuickAccessBar
        _btnNew,
        _btnOpen,
        _btnSave,

        // Share menu
        _actButtonFile,
        K3D_ACT_FILE_NEW_PROJECT,
        _actNewWizard,
        K3D_ACT_FILE_OPEN_PROJECT,
        K3D_ACT_FILE_SAVE_PROJECT,
        _actFilePrint,
        _actShareExportImage,
        K3D_ACT_FILE_SAVE_PROJECT_AS,
        _btnImportObjectFromProject,
        _btnMachineLibrary,
        _btnAddMachine,
        _btnEditMachine,
        _btnSelectMachine,
        K3D_ACT_MAIN_PREFERENCES,
        K3D_ACT_3D_PRINT,
        K3D_ACT_QUIT_APP,
        K3D_ACTION_PICK_PLACE,
        K3D_ACTION_SELECT_PART,
        K3D_ACTION_ROTATION,
        K3D_ACTION_PANNING,
        K3D_ACT_MAIN_UNDO,
        K3D_ACT_MAIN_REDO,

        // Help menu
        _actHelp,
        _actHelpReportIssue,
        _actHelpAbout,
        _actHelpHelp,
        _actNote,
        K3D_ACTION_NOTE,
        _actNoteSetting,
        _actNoteShowHide,
        _actNoteDeleteAll,
        //Recent Files Menu
        _menuRecentObjects,
        _menuRecentProjects,

        _btnAddSupport,

        // Home menu
        // Home menu/Object group
        K3D_ACT_OBJECT_IMPORT,
        K3D_ACT_OBJECT_EXPORT,
        K3D_ACT_OBJECT_UNLOAD,
        _btnAddObjFromMarkTrig,

        // Create 3D Shape
        _menuSolidShape,
        _menuCreator3D,
        _menuSolidShapeBox,
        _menuSolidShapeSphere,
        _menuSolidShapeCyLinder,
        _menuSolidShapePyramid,
        _menuSolidShapeWedge,
        _menuSolidShapeCone,
        _menuSolidShapeTorus,

        // Home menu/3D printer group
        _btn3DPrinter1,
        _btn3DPrinter2,
        // Home menu/Option group
        _btnSetting,
        _btnMachine,
        // Home menu/Help group
        _btnHelp,
        _menuHomeViews,
        _noteViews,
        _BrushViews,
        _SmoothViews,
        // View on Home
        _btnHomeShade,
        _btnHomeWireframe,
        _btnHomeTriangle,
        _btnHomeShadeWire,
        _btnHomeBoundingBox,
        _btnHomeView,
        _btnTransparency,

        _btnHomePlatform,                 // GL_KS_BIT_PLATFORM
        _btnHomeCoordinateSystem,         // GL_KS_BIT_AXIS
        _btnHomeTagName,                  // GL_KS_BIT_TAG_NAME
        _btnHomeOrientationIndicator,     // GL_KS_BIT_OI
        _btnHomeOrientationBoxIndicator,  // GL_KS_BIT_OI_BOX
        _btnPartDimension,                // GL_KS_BIT_DIMENSION
        _btnHomeRuler,                    // GL_KS_BIT_RULER
        K3D_ACTION_SHOW_HIDE_MARK_MENU,

        // Marking on Home
        K3D_ACTION_SELECT_TRIANGLE,
        K3D_ACTION_SELECT_TRIANGLE_BY_WINDOW,
        K3D_ACTION_SELECT_BRUSH,
        _btnHomeSelectTriangleByBrush1,
        _btnHomeSelectTriangleByBrush2,
        _btnHomeSelectTriangleByBrush3,
        _btnHomeSelectTriangleByBrush4,
        _btnHomeSelectTriangleByBrush5,
        _btnSmooth1,
        _btnSmooth2,
        _btnSmooth3,
        _btnSmooth4,
        _btnSmooth5,
        K3D_ACTION_SELECT_PLANE,
        K3D_ACTION_SELECT_SURFACE,
        K3D_ACTION_SELECT_SHELL,
        _btnHomeDeleteSelected,
        K3D_ACTION_SELECT_MARK_ALL,
        K3D_ACTION_SELECT_CLEAR,
        K3D_ACTION_SELECT_TOGGLE,
        K3D_ACTION_SELECT_DELETE,

        // View menu/Zoom group
        K3D_ACT_ZOOM_IN,
        K3D_ACT_ZOOM_OUT,
        K3D_ACT_ZOOM_OBJECTS,
        K3D_ACT_ZOOM_PLATFORM,
        K3D_ACT_ZOOM_WINDOW,
        // View menu/Default group
        K3D_ACT_VIEW_OBJ_BACK,
        K3D_ACT_VIEW_OBJ_FRONT,
        K3D_ACT_VIEW_OBJ_LEFT,
        K3D_ACT_VIEW_OBJ_RIGHT,
        K3D_ACT_VIEW_OBJ_TOP,
        K3D_ACT_VIEW_OBJ_BOTTOM,
        K3D_ACT_VIEW_OBJ_TOP_FRONT,

        // View Menu/Level
        K3D_ACT_SHOW_HIDE_PLATFORM_LEVEL,

        // Tools menu

        // Tools menu/Master group
        K3D_ACT_OBJECT_MOVE,
        K3D_ACT_OBJECT_ORIENTATE,
        K3D_ACT_OBJECT_RESCALE,
        K3D_ACT_OBJECT_DUPLICATE,
        K3D_ACT_OBJECT_MIRROR,
        K3D_ACT_OBJECT_ARRANGE,
        K3D_ACT_OBJECT_MATE,
        K3D_ACTION_MEASURE,
        // Tools menu/Edit group
        K3D_ACT_OBJECT_CREATE_HOLLOW,
        K3D_ACT_OBJECT_SHRINK_WRAP,
        K3D_ACT_OBJECT_SHARPEN,
        K3D_ACT_OBJECT_CREATE_HOLES,
        K3D_ACT_OBJECT_CUT,
        K3D_ACT_OBJECT_TRIANGLE_REDUCE,
        K3D_ACT_OBJECT_SMOOTH,
        K3D_ACT_OBJECT_CREATE_LABEL,
        K3D_ACT_OBJECT_MERGE,
        // Tools menu/Other group
        K3D_ACT_OBJECT_CREATE_SUPPORT_ADV,
        K3D_ACT_OBJECT_CREATE_SUPPORT_MANUAL,
        K3D_ACT_OBJECT_CREATE_SUPPORT_AUTO,
        K3D_ACT_OBJECT_ADD_LEVEL,

        // Tools menu/Platform Level
        K3D_ACT_PLATFORM_LEVEL_VIEW,
        K3D_ACT_PLATFORM_LEVEL_ARRANGE,
        K3D_ACT_PLATFORM_LEVEL_REMOVE,
        K3D_ACT_PLATFORM_LEVEL_SET_HEIGHT,
        K3D_ACT_PLATFORM_LEVEL_SETTING,

        // Slice view
        K3D_ACT_SLICE_RUN,
        K3D_ACT_SLICE_EXPORT,
        _btnExportImage,
        K3D_ACT_SLICE_SETTINGS,

        // Repair

        _actMenuInvertNormal,
        _actMenuStitching,
        _actMenuNoiseShell,
        _actMenuFixHole,
        _actMenuFixOverlappingTriangle,
        _actMenuFixTriangle,

        // Repair menu/fix
        _menuInvertNormal,
        _menuAutomaticFixNormal,
        _menuInvertTriangleNormal,
        _menuInvertShellNormal,
        _menuInvertMarkedNormal,
        _menuInvertPartNormal,
        _btnFixInvertedNormal,
        //QAction* _currentInvertNormalAction,

        // Repair/stitching
        _btnFixBadEdges,
        _menuStitching,
        _menuAutomaticStiching,
        _menuManualStitching,

        // Repair/Noise shell
        _menuNoiseShell,
        _menuAutomaticFixShell,
        _menuAutomaticFixNoiseShell,
        _menuManualFixShell,
        _menuAutomaticMergeShell,
        _btnFixNoiseShell,

        // Repair/Hole
        _menuFixHole,
        _menuAutomaticFixHole,
        _menuFixPlanaHole,
        _menuFixHoleByRule,
        _menuFixHoleFreedom,
        _btnFixHole,

        // Repair/Triangle
        _menuFixTriangle,
        _menuAutomaticFixTriangle,
        K3D_ACT_REPAIR_CREATE_TRIANGLES,
        K3D_ACT_REPAIR_DELETE_TRIANGLES,
        _btnFixTriangle,

        // Repair/OVerlapping Triangle
        _menuFixOverlappingTriangle,
        _menuAutomaticFixOverlappingTriangle,
        _menuRemoveSharp,
        _menuDetectOverlappingTriangle,
        _menuClearOvelappingDetect,
        _btnFixOverlappingTriangle,

        // Repair menu/Control group
        _cbFixObjectList,
        _btnFixWizard,
        _btnCustomRepair,
        _btnUpdateProblem,
        _lbWhatHere,

        _btnAddTriangle,
        _btnDeleteTriangle,

        // Support menu

        // Support menu/Main group
        _btnExitSupport,
        _btnCancelSupport,
        _btnSaveEnvironment,
        _btnRegenarateSupport,
        // Support menu/View Group
        _btnSupportViewAll,
        _btnSupportViewBase,
        _btnSupportViewAllSurfaces,
        _btnSupportViewSchemeColor,
        _btnSupportSliceView,

        // Support menu/Surface group
        _btnEditSupportSurface,
        K3D_ACT_SUPPORT_ADD_AREA,
        K3D_ACT_SUPPORT_MERGE,
        K3D_ACT_SUPPORT_DUPLICATE,
        _btnChangeSurfaceAngle,
        // Support menu/Add Type group
        K3D_ACT_SUPPORT_ADD_WALL,
        K3D_ACT_SUPPORT_ADD_COLUMN,
        K3D_ACT_SUPPORT_ADD_GUSSET,
        K3D_ACT_SUPPORT_ADD_TREE,
        K3D_ACT_SUPPORT_ADD_SCAFFOLD,
        // Support menu/Marking group
        K3D_ACTION_SELECT_SUPPORT,
        K3D_ACT_SUPPORT_REMOVE,
        K3D_ACT_SUPPORT_REMOVE_ALL,
        K3D_ACTION_MARK_LINE,
        _btnMarkPolyline,
        _btnMarkAll,
        _btnClearMarked,
        _btnDeleteMarked,

        _menuObjectListGeneral,
        _menuContextObjectRecent,

        _menuContextGeneralTitle,

        _menuObjectListGeneralImport,
        _menuObjectListGeneralUnloadSelected,
        K3D_ACT_OBJECT_SELECT_ALL,
        K3D_ACT_OBJECT_SELECT_INVERT,
        K3D_ACT_OBJECT_SELECT_CLEAR,
        _menuObjectListGeneralShowAll,
        _menuObjectListGeneralHideAll,
        _menuObjectListGeneralHideUnselected,

        // Context Menu Individual - Object List and Main Tab

        _menuObjectListIndividual,

        K3D_MENU_OBJECT_LIST_INIDIVIDUAL_UNLOAD,
        K3D_CONTEXT_MENU_INDIVIDUAL_RENAME,
        K3D_CONTEXT_MENU_INDIVIDUAL_REPAIR,
        K3D_CONTEXT_MENU_INDIVIDUAL_SUPPORT,
        K3D_CONTEXT_MENU_INDIVIDUAL_CONVERT_SLC,
        K3D_CONTEXT_MENU_INDIVIDUAL_CREATE_TRIANGLE, // For SLC Object
        K3D_CONTEXT_MENU_INDIVIDUAL_MOVE,
        K3D_CONTEXT_MENU_INDIVIDUAL_ROTATE,
        K3D_CONTEXT_MENU_INDIVIDUAL_RESCALE,
        K3D_CONTEXT_MENU_INDIVIDUAL_DUPLICATE,
        
        _menuContextObjectViews,

        K3D_CONTEXT_MENU_INDIVIDUAL_VIEW_SHADE,
        K3D_CONTEXT_MENU_INDIVIDUAL_VIEW_WIRE_FRAME,
        K3D_CONTEXT_MENU_INDIVIDUAL_VIEW_TRIANGLE,
        K3D_CONTEXT_MENU_INDIVIDUAL_VIEW_SHADE_WIRE,
        K3D_CONTEXT_MENU_INDIVIDUAL_VIEW_BOUDING_BOX,
        K3D_CONTEXT_MENU_INDIVIDUAL_VIEW_TRANSPARENCY,

        // Context Menu Repair - Repair Tab

        _menuRepairTab,

        _menuRepairTabMove,
        _menuRepairTabRotate,
        _menuRepairTabRescale,
        _menuRepairTabWizard,

        // Context Menu Support - Support Tab

        K3D_ACT_SUPPORT_MENU,
        K3D_ACT_SUPPORT_CONTEXT_MENU,

        K3D_ACT_SUPPORT_CONCLUDE,
        K3D_ACT_SUPPORT_CANCEL,
        K3D_ACT_SUPPORT_REGENERATE,
        K3D_ACT_SUPPORT_EDIT,
        K3D_ACT_SUPPORT_EDIT_AREA,
        K3D_ACT_SUPPORT_DETAILS,

        K3D_ACT_SUPPORT_CONTEXT_MENU_DELETE,
        K3D_ACT_SUPPORT_CONTEXT_MENU_SCAFFOLD,
        K3D_ACT_SUPPORT_CONTEXT_MENU_WALL,
        K3D_ACT_SUPPORT_CONTEXT_MENU_COLUMN,
        K3D_ACT_SUPPORT_CONTEXT_MENU_SHORING,

        _menuHelp,
        _menuNote,
        // End Context Menus

        K3D_ACTION_TOTAL
    }; // End K3D_ACTION

    enum K3D_QML_CONTEXT_PROPERTY {
        K3D_APPLICATION,
        MAIN_WINDOW,
        MAIN_QUICK_CONTAINER,
        QML_ADAPTER,

		// Main GL Widget
		MAIN_GLWIDGET_AGENT,

        // Support GL Widget
        SUPPORT_GL_AGENT,

        // Add/Edit Support GL Widget
        SUPPORT_TYPE_GL_AGENT,

        // Add Level Support GL Widget
        ADD_LEVEL_SUPPORT_GL_AGENT,

        K3D_QML_CONTEXT_PROPERTY_TOTAL
    };

    // These names are canonical, which will be used DIRECTLY in QML:
    static const QString CONTEXT_PROPERTY[K3D_QML_CONTEXT_PROPERTY_TOTAL];
    
    static const int K3D_OPERATION_NULL = -1;

    enum K3D_OPERATION {
        K3DOP_FIRST,
        K3DOP_TOTAL = 1
    };

    enum K3D_VOID_OPERATION {

        // K3DOP_QUICK_MENU ---------------------
        K3DOP_QUICK_MENU_NEW_PROJECT,
        K3DOP_QUICK_MENU_OPEN_PROJECT,
        K3DOP_QUICK_MENU_SAVE_PROJECT,
        K3DOP_QUICK_MENU_UNDO,
        K3DOP_QUICK_MENU_REDO,

        // K3DOP_MAIN_MENU ----------------------
        K3DOP_MAIN_MENU_NEW_PROJECT,
        K3DOP_MAIN_MENU_OPEN_PROJECT,
        K3DOP_MAIN_MENU_SAVE_PROJECT,
        K3DOP_MAIN_MENU_SAVE_PROJECT_AS,
        K3DOP_MAIN_MENU_OPEN_PROJECT_IMPORT,
        K3DOP_MAIN_MENU_RECENT_OBJECTS,
        K3DOP_MAIN_MENU_RECENT_PROJECTS,
        K3DOP_MAIN_MENU_MANAGE_3D_PRINTERS,
        K3DOP_MAIN_MENU_ADD_3D_PRINTERS,
        K3DOP_MAIN_MENU_EDIT_3D_PRINTERS,
        K3DOP_MAIN_MENU_SELECT_3D_PRINTERS,
        
        K3DOP_MAIN_MENU_ADD_SUPPORT_TYPE,
        K3DOP_MAIN_MENU_EDIT_SUPPORT_TYPE,

        K3DOP_MAIN_MENU_PREFERENCES,
        K3DOP_MAIN_MENU_3D_PRINT,

        // K3DOP_HELP_MENU ----------------------
        K3DOP_HELP_MENU_MANUAL_GUIDE,
        K3DOP_HELP_MENU_REPORT_ISSUE,

        // K3DOP_HOME_OBJECT --------------------
        K3DOP_HOME_OBJECT_IMPORT,
        K3DOP_HOME_OBJECT_SAVE,
        K3DOP_HOME_OBJECT_UNLOAD,
        // K3DOP_GROUP_HOME_OBJECT_TOTAL     = 10,

        // K3DOP_HOME_ZOOM ----------------------
        K3DOP_HOME_ZOOM_OBJECTS,
        K3DOP_HOME_ZOOM_PLATFORM,
        K3DOP_HOME_ZOOM_WINDOWS,
        K3DOP_HOME_ZOOM_IN,
        K3DOP_HOME_ZOOM_OUT,
        //K3DOP_GROUP_HOME_ZOOM_TOTAL       = 20,

        // K3DOP_HOME_VIEW ----------------------
        K3DOP_HOME_VIEW_OBJECT_SHADE,
        K3DOP_HOME_VIEW_OBJECT_WIREFRAME,
        K3DOP_HOME_VIEW_OBJECT_TRIANGLE,
        K3DOP_HOME_VIEW_OBJECT_SHADEWIRE,
        K3DOP_HOME_VIEW_OBJECT_BBBOX,
        K3DOP_HOME_VIEW_OBJECT_TRANSPARENT,
        K3DOP_HOME_VIEW_ANGLE_TOP_FRONT,
        K3DOP_HOME_VIEW_ANGLE_TOP,
        K3DOP_HOME_VIEW_ANGLE_BOTTOM,
        K3DOP_HOME_VIEW_ANGLE_BACK,
        K3DOP_HOME_VIEW_ANGLE_FRONT,
        K3DOP_HOME_VIEW_ANGLE_LEFT,
        K3DOP_HOME_VIEW_ANGLE_RIGHT,

        //K3DOP_GROUP_HOME_VIEW_TOTAL      = 30,

        // K3DOP_HOME_MARK ----------------------
        K3DOP_HOME_MARK_ALL,

        K3DOP_HOME_MARK_CLEAR,
        K3DOP_HOME_MARK_TOGGLE,
        K3DOP_HOME_MARK_DELETE,

        // K3DOP_HOME_SHOW -----------------------
        K3DOP_HOME_SHOW_PLATFORM,
        K3DOP_HOME_SHOW_COORD,
        K3DOP_HOME_SHOW_ORIENTATION,
        K3DOP_HOME_SHOW_ORIENTATION_BOX,
        K3DOP_HOME_SHOW_DIMENSIONS,
        K3DOP_HOME_SHOW_TAGNAME,
        K3DOP_HOME_SHOW_RULER,
		K3DOP_HOME_SHOW_OBJ_INFO,
		K3DOP_HOME_SHOW_MARK_MENU,

        // K3DOP_TOOLS_BASIC ---------------------
        K3DOP_TOOLS_BASIC_MOVE_OBJ,
        K3DOP_TOOLS_BASIC_ROTATE_OBJ,
        K3DOP_TOOLS_BASIC_RESCALE_OBJ,
        K3DOP_TOOLS_BASIC_DUPLICATE_OBJ,
        K3DOP_TOOLS_BASIC_MIRROR_OBJ,
        K3DOP_TOOLS_BASIC_ARRANGE_OBJ,
        K3DOP_TOOLS_BASIC_MATE_OBJ,
        K3DOP_TOOLS_BASIC_EXPORT_IMG,

        // K3DOP_TOOLS_ADVANCED ------------------
        K3DOP_TOOLS_ADV_HOLLOW,
        K3DOP_TOOLS_ADV_WRAP,
        K3DOP_TOOLS_ADV_MERGE,
        K3DOP_TOOLS_ADV_CREATE_HOLES,
        K3DOP_TOOLS_ADV_TRIG_REDUCE,
        K3DOP_TOOLS_ADV_SMOOTH,
        K3DOP_TOOLS_ADV_CUT,
        K3DOP_TOOLS_ADV_ADD_LABEL,
        K3DOP_TOOLS_ADV_ADD_LEVEL_SUPPORT,

        // K3DOP_TOOLS_ADD_LEVEL_SUPPORT----------
        K3DOP_TOOLS_ADD_LEVEL_ARRANGE,
        K3DOP_TOOLS_ADD_LEVEL_REMOVE,
        K3DOP_TOOLS_ADD_LEVEL_SET_HEIGHT,
        K3DOP_TOOLS_ADD_LEVEL_SETTING,

        // K3DOP_TOOLS_ANNOTATIONS ---------------
        K3DOP_TOOLS_ANNOTATIONS_MEASURE,
        K3DOP_TOOLS_ANNOTATIONS_ADD_NOTE,

        // K3DOP_TOOLS_SLICE ---------------------
        K3DOP_TOOLS_SLICE_VIEW,
        K3DOP_TOOLS_SLICE_PLAY_SETTING,
        K3DOP_TOOLS_SLICE_EXPORT,

        // K3DOP_TOOLS_SUPPORT -------------------
        K3DOP_TOOLS_SUPPORT_AUTO,
        K3DOP_TOOLS_SUPPORT_ADV,
        K3DOP_TOOLS_SUPPORT_MANUAL,

        // K3DOP_REPAIR_AUTOMATIC ----------------
        K3DOP_REPAIR_AUTOMATIC_WIZARD,
        K3DOP_REPAIR_AUTOMATIC_CUSTOM,

        // K3DOP_REPAIR_MANUAL -------------------
        K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_AUTO,
        K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_TRIG,
        K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_SHELL,
        K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_PART,
        K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_MARKED,

        K3DOP_REPAIR_MANUAL_STITCHING_AUTO,
        K3DOP_REPAIR_MANUAL_STITCHING_MANUAL,

        K3DOP_REPAIR_MANUAL_SHELLS_AUTO,
        K3DOP_REPAIR_MANUAL_SHELLS_BAD,
        K3DOP_REPAIR_MANUAL_SHELLS_MERGE,
        K3DOP_REPAIR_MANUAL_SHELLS_ADV,

        K3DOP_REPAIR_MANUAL_HOLES_AUTO,
        K3DOP_REPAIR_MANUAL_HOLES_PLANAR,
        K3DOP_REPAIR_MANUAL_HOLES_FREEDOM,

        K3DOP_REPAIR_MANUAL_OVERLAPS_AUTO,
        K3DOP_REPAIR_MANUAL_OVERLAPS_REMOVE_SHARP,
        K3DOP_REPAIR_MANUAL_OVERLAPS_DETECT,
        K3DOP_REPAIR_MANUAL_OVERLAPS_CLEAR_MARK,

        K3DOP_REPAIR_MANUAL_TRIANGLES_CREATE,
        K3DOP_REPAIR_MANUAL_TRIANGLES_DELETE,

        // K3DOP_DIAGNOSTICS_TOOLS -----------------
        K3DOP_DIAG_FIX_INVERTED_NORMAL,
        K3DOP_DIAG_FIX_BAD_EDGE,
        K3DOP_DIAG_FIX_STITCH,
        K3DOP_DIAG_FIX_HOLE,
        K3DOP_DIAG_FIX_SHELL,
        K3DOP_DIAG_FIX_NOISE_SHELL,
        K3DOP_DIAG_FIX_OVERLAPPING,
        K3DOP_DIAG_FIX_INTERSECTION,
        K3DOP_DIAG_FIX_ALL_PROBLEMS,

        K3DOP_DIAG_UPDATE_INVERTED_NORMAL,
        K3DOP_DIAG_UPDATE_BAD_EDGE,
        K3DOP_DIAG_UPDATE_NEAR_BAD_EDGE,
        K3DOP_DIAG_UPDATE_BAD_CONTOUR,
        K3DOP_DIAG_UPDATE_SHELL,
        K3DOP_DIAG_UPDATE_INTERSECTION,
        K3DOP_DIAG_UPDATE_ALL_PROBLEMS,

        // K3DOP_SUPPORT_MAIN ----------------------
        K3DOP_SUPPORT_CONCLUDE,
        K3DOP_SUPPORT_CANCEL,
        K3DOP_SUPPORT_REGENERATE,
        K3DOP_SUPPORT_REMOVE_ALL,

        K3DOP_SUPPORT_SURFACE_REMOVE,
        K3DOP_SUPPORT_SURFACE_CREATE,
        K3DOP_SUPPORT_SURFACE_EDIT,
        K3DOP_SUPPORT_AREA_EDIT,
        K3DOP_SUPPORT_SURFACE_DETAILS,

        K3DOP_SUPPORT_ADD_COLUMN,
        K3DOP_SUPPORT_ADD_WALL,
        K3DOP_SUPPORT_ADD_TREE,
        K3DOP_SUPPORT_ADD_SCAFFOLD,
        K3DOP_SUPPORT_ADD_SHORING,
        K3DOP_SUPPORT_ADD_DUPLICATE,

        K3DOP_SUPPORT_MARK_LINE,
        K3DOP_SUPPORT_MARK_TOGGLE,
        K3DOP_SUPPORT_MARK_ALL,
        K3DOP_SUPPORT_MARK_CLEAR,
        K3DOP_SUPPORT_MARK_DELETE,

        // K3DOP_SUPPORT_VIEW ----------------------
        K3DOP_SUPPORT_VIEW_ALL_SUPPORTS,
        K3DOP_SUPPORT_VIEW_BASE,
        K3DOP_SUPPORT_VIEW_ALL_SURFACES,
        K3DOP_SUPPORT_VIEW_COLOR_SCHEMER,

        // K3DOP_CONTEXT_MENU_ -------------------------------
        K3DOP_CONTEXT_MENU_OBJECT_TITLE,

        K3DOP_CONTEXT_MENU_IMPORT_OBJECTS,
        K3DOP_CONTEXT_MENU_UNLOAD_SELECTED_OBJS,
        K3DOP_CONTEXT_MENU_RECENT_OBJS,
        K3DOP_CONTEXT_MENU_VIEW_MODE,

        K3DOP_CONTEXT_MENU_SUPPORT_NONE,
        K3DOP_CONTEXT_MENU_SUPPORT_SCAFFOLD,
        K3DOP_CONTEXT_MENU_SUPPORT_WALL,
        K3DOP_CONTEXT_MENU_SUPPORT_COLUMN,
        K3DOP_CONTEXT_MENU_SUPPORT_SHORING,

        K3DOP_CONTEXT_MENU_SELECT_ALL,
        K3DOP_CONTEXT_MENU_CLEAR_SELECTION,
        K3DOP_CONTEXT_MENU_INVERT_SELECTION,

        K3DOP_CONTEXT_MENU_SHOW_ALL_OBJS,
        K3DOP_CONTEXT_MENU_HIDE_ALL_OBJS,
        K3DOP_CONTEXT_MENU_HIDE_UNSELECTED_OBJS,

        // K3DOP_OBJECT_LIST
        K3DOP_OBJECT_LIST_SELECT_ALL,
        K3DOP_OBJECT_LIST_CLEAR_SELECTION,
        K3DOP_OBJECT_LIST_INVERT_SELECTION,
        K3DOP_OBJECT_LIST_REPAIR_ALL,

        K3DOPVOID_TOTAL
    };

    enum K3D_PARAM_OPERATION {
        // K3DOP_HELP_MENU
        //
        K3DOP_HELP_MENU_ABOUT,

        // K3DOP_QUICK_MENU
        //
        K3DOP_QUICK_MENU_MOUSE_SELECT,
        K3DOP_QUICK_MENU_PICK_PLACE,
        K3DOP_QUICK_MENU_ROTATE,
        K3DOP_QUICK_MENU_PANNING,

        // K3DOP_TOOLS_ADD_LEVEL_SUPPORT----------
        K3DOP_TOOLS_ADD_LEVEL_VIEW,
        K3DOP_TOOLS_ADV_SHOW_LEVEL_SUPPORT,

        // K3DOP_MARK_MENU
        // 
        K3DOP_HOME_MARK_TRIANGLES,
        K3DOP_HOME_MARK_TRIANGLES_WINDOWS,
        K3DOP_HOME_MARK_TRIANGLES_BRUSH,
        K3DOP_HOME_MARK_TRIANGLES_BRUSH1,
        K3DOP_HOME_MARK_TRIANGLES_BRUSH2,
        K3DOP_HOME_MARK_TRIANGLES_BRUSH3,
        K3DOP_HOME_MARK_TRIANGLES_BRUSH4,
        K3DOP_HOME_MARK_TRIANGLES_BRUSH5,
        K3DOP_HOME_MARK_PLANES,
        K3DOP_HOME_MARK_SURFACES,
        K3DOP_HOME_MARK_SHELLS,

        // K3DOP_RECENT_OBJECTS_PROJECTS
        //
        K3DOP_RECENT_OBJECT,
        K3DOP_RECENT_PROJECT,

        // K3DOP_OBJECT_LIST
        K3DOP_OBJECT_LIST_VISIBLE_SINGLE_OBJ,
        K3DOP_OBJECT_LIST_LOCK_SINGLE_OBJ,
        K3DOP_OBJECT_LIST_UNLOAD_SINGLE_OBJ,
        K3DOP_OBJECT_LIST_REPAIR_SINGLE_OBJ,
        K3DOP_OBJECT_LIST_SET_OBJ_COLOR,

        // K3DOP_CONTEXT_MENU_ -------------------------------
        K3DOP_CONTEXT_MENU_RENAME_OBJ,
        K3DOP_CONTEXT_MENU_REPAIR_OBJ,
        K3DOP_CONTEXT_MENU_CONVERT_TO_SLC,
        K3DOP_CONTEXT_MENU_CREATE_SUPPORT,
        K3DOP_CONTEXT_MENU_CREATE_TRIANGLES,

        K3DOP_CONTEXT_MENU_MOVE_OBJ,
        K3DOP_CONTEXT_MENU_ROTATE_OBJ,
        K3DOP_CONTEXT_MENU_RESCALE_OBJ,
        K3DOP_CONTEXT_MENU_DUPLICATE_OBJ,

        K3DOP_CONTEXT_MENU_VIEW_SHADE,
        K3DOP_CONTEXT_MENU_VIEW_WIREFRAME,
        K3DOP_CONTEXT_MENU_VIEW_TRIANGLE,
        K3DOP_CONTEXT_MENU_VIEW_SHADEWIRE,
        K3DOP_CONTEXT_MENU_VIEW_BBBOX,
        K3DOP_CONTEXT_MENU_VIEW_TRANSPARENT,

		// K3DOP_PROGRESS_BAR
		K3DOP_PROGRESSBAR_INFORM_VISIBILITY,

        // K3DOP_SLICE_VIEW
        K3DOP_SLICE_VIEW_REVOKE,

        // K3DOP_MARK_BRUSH_SIZE_CHANGE
        K3DOP_MARK_TRIANGLES_BRUSH_SIZE_CHANGE,

        // K3DOP_SUPPORT
        K3DOP_SUPPORT_SURFACE_SELECT,
        K3DOP_SUPPORT_SURFACE_MERGE,

        K3DOP_SINGLE_PARAM_TOTAL
    };

    enum K3D_DOUBLE_PARAM_OPERATION {
        // K3DOP_MAIN_CENTRAL_QUICK_CONTAINER
        //
        K3DOP_MAIN_QUICK_CONTAINER_MOUSE_EVENT,

        // K3DOP_MAIN_TAB
        // 
        K3DOP_MAIN_TAB_MAIN_FUNCTION,
        K3DOP_MAIN_TAB_SUB_FUNCTION,

        K3DOP_DOUBLE_PARAM_TOTAL
    };

    enum K3D_TRIPLE_PARAM_OPERATION {
        K3DOP_TRIPLE_PARAM_TOTAL
    };

    enum K3D_CONTEXT_MENU_GENERAL {
        GENERAL_CONTEXT_MENU_GENERAL,
        
        GENERAL_CONTEXT_MENU_IMPORT_OBJ,
        GENERAL_CONTEXT_MENU_UNLOAD_OBJ,
        GENERAL_CONTEXT_MENU_RECENT_OBJS,

        //_SEPARATOR_5,
        GENERAL_CONTEXT_MENU_SELECT_ALL_OBJS,
        GENERAL_CONTEXT_MENU_CLEAR_SELECTION,
        GENERAL_CONTEXT_MENU_INVERT_SELECTION,

        //_SEPARATOR_5,
        GENERAL_CONTEXT_MENU_SHOW_ALL_OBJS,
        GENERAL_CONTEXT_MENU_HIDE_ALL_OBJS,
        GENERAL_CONTEXT_MENU_HIDE_UNSELECTED_OBJS,
    };

    enum K3D_CONTEXT_MENU_INDIVIDUAL {
        INDIVIDUAL_CONTEXT_MENU_OBJECT_TITLE,
        INDIVIDUAL_CONTEXT_MENU_UNLOAD_OBJ,
        INDIVIDUAL_CONTEXT_MENU_VIEW_MODE,

        INDIVIDUAL_CONTEXT_MENU_RENAME_OBJ,
        INDIVIDUAL_CONTEXT_MENU_REPAIR_OBJ,
        INDIVIDUAL_CONTEXT_MENU_CREATE_SUPPORT,
        INDIVIDUAL_CONTEXT_MENU_CREATE_TRIANGLES,
        INDIVIDUAL_CONTEXT_MENU_CONVERT_TO_SLC,
        
        INDIVIDUAL_CONTEXT_MENU_MOVE,
        INDIVIDUAL_CONTEXT_MENU_ROTATE,
        INDIVIDUAL_CONTEXT_MENU_RESCALE,
        INDIVIDUAL_CONTEXT_MENU_DUPLICATE_OBJ,
        
        INDIVIDUAL_CONTEXT_MENU_IMPORT_OBJ,
        INDIVIDUAL_CONTEXT_MENU_SHOW_ALL_OBJS,
        INDIVIDUAL_CONTEXT_MENU_HIDE_ALL_OBJS,
        INDIVIDUAL_CONTEXT_MENU_HIDE_UNSELECTED_OBJS,

        INDIVIDUAL_CONTEXT_MENU_TOTAL
    };

    enum K3D_CONTEXT_MENU_REPAIR {
        REPAIR_CONTEXT_MENU_OBJECT_TITLE,
        //_SEPARATOR_1,
        REPAIR_CONTEXT_MENU_UNLOAD_OBJ,
        REPAIR_CONTEXT_MENU_RENAME_OBJ,

        //_SEPARATOR_2,
        REPAIR_CONTEXT_MENU_MOVE,
        REPAIR_CONTEXT_MENU_ROTATE,
        REPAIR_CONTEXT_MENU_RESCALE,

        //_SEPARATOR_3,
        REPAIR_CONTEXT_MENU_MARK_TRIANGLES,
        REPAIR_CONTEXT_MENU_MARK_PLANES,
        REPAIR_CONTEXT_MENU_MARK_SURFACES,
        REPAIR_CONTEXT_MENU_MARK_SHELLS,
        REPAIR_CONTEXT_MENU_MARK_ALL,
        REPAIR_CONTEXT_MENU_MARK_CLEAR,
        REPAIR_CONTEXT_MENU_MARK_TOGGLE,
        REPAIR_CONTEXT_MENU_MARK_DELETE,

        REPAIR_CONTEXT_MENU_TOTAL
    };

    enum K3D_CONTEXT_MENU_SUPPORT {
        SUPPORT_CONTEXT_MENU_OBJECT_TITLE,
        SUPPORT_CONTEXT_MENU_CONCLUDE,
        SUPPORT_CONTEXT_MENU_CANCEL,
        SUPPORT_CONTEXT_MENU_REGENERATE,
        SUPPORT_CONTEXT_MENU_REMOVE_ALL,
        SUPPORT_CONTEXT_MENU_SUPPORT_LABEL,
        SUPPORT_CONTEXT_MENU_SUPPORT_TYPE,
        SUPPORT_CONTEXT_MENU_CREATE_SUPPORT,
        SUPPORT_CONTEXT_MENU_EDIT_SUPPORT,
        SUPPORT_CONTEXT_MENU_EDIT_AREA,
        SUPPORT_CONTEXT_MENU_SUPPORT_DETAILS,

        //_SEPARATOR_1,
        SUPPORT_CONTEXT_MENU_MARK_TRIANGLES,
        SUPPORT_CONTEXT_MENU_MARK_PLANES,
        SUPPORT_CONTEXT_MENU_MARK_SURFACES,
        SUPPORT_CONTEXT_MENU_MARK_SHELLS,
        SUPPORT_CONTEXT_MENU_MARK_ALL,
        SUPPORT_CONTEXT_MENU_MARK_CLEAR,
        SUPPORT_CONTEXT_MENU_MARK_TOGGLE,
        SUPPORT_CONTEXT_MENU_MARK_DELETE,

        SUPPORT_CONTEXT_MENU_TOTAL
    };


    enum K3D_CONTEXT_MENU_VIEW_MODE {
        OBJECT_VIEW_SHADE         = ObjectEx::VIEW_MODE_SHADE,
		OBJECT_VIEW_WIREFRAME     = ObjectEx::VIEW_MODE_WIREFRAME,
        OBJECT_VIEW_TRIANGLES     = ObjectEx::VIEW_MODE_TRIANGLE,
        OBJECT_VIEW_SHADEWIRE	  = ObjectEx::VIEW_MODE_SHADE_WIRE,
        OBJECT_VIEW_BOUNDINGBOX   = ObjectEx::VIEW_MODE_BOUNDING_BOX,

		//_SEPARATOR
		OBJECT_VIEW_TRANSPARENT, // Just to map the QML UI
        TOTAL_VIEW
    };

    enum K3D_QUICK_VIEW_OBJECT_VIEW_MODE {
        QUICK_VIEW_TRANSPARENT,   
        QUICK_VIEW_WIRE_FRAME,    
        QUICK_VIEW_SHADE_WIRE,    
        QUICK_VIEW_BOUNDING_BOX,  
        QUICK_VIEW_TRIANGLE,     
        QUICK_VIEW_SHADE,
        TOTAL
    };
    
    // -----------------------------------
    // FUNCTION OPERATION DIALOS --
    //
    enum K3D_OPERATION_DIALOG_EVENT {
        K3D_DIALOG_EVENT_SHOW,
        K3D_DIALOG_EVENT_ESC,
        K3D_DIALOG_EVENT_CLOSE,
        K3D_DIALOG_EVENT_RESIZE,
        K3D_DIALOG_EVENT_DROP,

        K3D_DIALOG_EVENT_TOTAL
    };

    enum K3D_DIALOG {
        K3D_MAIN_WINDOW,
        K3D_MAIN_OBJECT_TABLE,
        K3D_ABOUT_DIALOG,
        K3D_SETTINGS_DIALOG,
        K3D_MACHINE_DIALOG,
        K3D_MACHINE_ADD_DIALOG,
        K3D_MACHINE_EDIT_DIALOG,
        K3D_MACHINE_SELECT_BAR,

        K3D_OBJ_IMPORT_FROM_PROJECT_DIALOG,
        K3D_OBJ_ORIENTATE_DIALOG,
        K3D_OBJ_MOVE_DIALOG,
        K3D_OBJ_RESCALE_DIALOG,
        K3D_OBJ_DUPLICATE_DIALOG,
        K3D_OBJ_ARRANGE_DIALOG,
        K3D_OBJ_MATE_DIALOG,

        K3D_OBJ_STITCH_DIALOG,
        K3D_OBJ_NOTE_DIALOG,
        K3D_OBJ_MEASURE_DIALOG,
        K3D_OBJ_MIRROR_DIALOG,
        K3D_OBJ_REMOVE_SHARP_DIALOG,
        K3D_OBJ_CREATE_HOLE_DIALOG,
        K3D_OBJ_SMOOTH_DIALOG,
        K3D_OBJ_BASIC_CUT_DIALOG,
        K3D_OBJ_ADD_LABEL_DIALOG,
        K3D_CREATE_HOLLOW_DIALOG,

        K3D_OBJ_2SLC_DIALOG,
        K3D_OBJ_TRIG_REDUCTION_DIALOG,
        K3D_OBJ_TRIG_MESH_DIALOG,
        K3D_OBJ_SHRINK_WRAP_DIALOG,
        K3D_OBJ_SHARPEN_DIALOG,
        K3D_SHELL_DIALOG,
        K3D_SURFACE_EDIT_DIALOG,

        K3D_SLICE_VIEW,
        K3D_SLICE_EXPORT_DIALOG,
        K3D_SLICE_PLAY_DIALOG,

        K3D_REPAIR_WIZARD_DIALOG,
        K3D_REPAIR_CUSTOM_DIALOG,
        K3D_SUPPORT_DUPLICATE_DIALOG,
        K3D_SUPPORT_ADD_LEVEL_DIALOG,
        K3D_PRINT_3D_DIALOG,
        K3D_FILE_SPLIT_DIALOG,

        K3D_OBJECT_DIAGNOSTICS_TABLE,

        /* Add more dialog id here */
        K3D_DIALOG_TOTAL
    };
    static const QString QML_PROPERTY_DIALOG[K3D_DIALOG_TOTAL];
    static const QString QML_PROPERTY_MAIN_MODAL_DIALOG;
    //KDialogAgent* getKsDialog(int dialogId);
    
    enum K3D_SUPPORT_TYPE {
        K3D_SUPPORT_NULL      = -1,
        K3D_SUPPORT_NONE      = SupportGLWidget::SUPPORT_NONE,
        K3D_SUPPORT_WALL      = SupportGLWidget::SUPPORT_WALL,
        K3D_SUPPORT_COLUMN    = SupportGLWidget::SUPPORT_COLUMN,
        K3D_SUPPORT_SHORING   = SupportGLWidget::SUPPORT_SHORING,
        K3D_SUPPORT_SCAFFOLD  = SupportGLWidget::SUPPORT_SCAFFOLD,
        K3D_SUPPORT_TREE      = SupportGLWidget::SUPPORT_TREE,
        K3D_SUPPORT_CYLINDER_GUSSET = SupportGLWidget::SUPPORT_CYLINDER_GUSSET,
        K3D_SUPPORT_BASE      = SupportGLWidget::SUPPORT_BASE,

        K3D_SUPPORT_TYPE_TOTAL
    };

    enum K3D_COLUMN_SUPPORT_TYPE {
        K3D_COLUMN_STAR = 0,
        K3D_COLUMN_CYLINDER = 1,

        K3D_COLUMN_TOTAL,
    };

    enum K3D_BASE_SUPPORT_TYPE {
        K3D_BASE_BOX = 1,
        K3D_BASE_SURFACE = 2,

        K3D_BASE_TOTAL,
    };

    static QString getQMLSupportTypeName(int shape) {
        return shape == K3D_SUPPORT_WALL     ? tr("Wall")     :
               shape == K3D_SUPPORT_COLUMN   ? tr("Column")   :
               shape == K3D_SUPPORT_SHORING  ? tr("Shoring")  :
               shape == K3D_SUPPORT_SCAFFOLD ? tr("Scaffold") :
               shape == K3D_SUPPORT_TREE     ? tr("Tree")     : "";
    }

    static int verifyQMLSupportType(int objSupportType, int supportShape) {
        int shape = getQMLSupportType(objSupportType);
        return shape == supportShape;
    }

    static int getQMLSupportType(int objSupportType) {
        switch (objSupportType) {
            case ObjectEx::SUPPORT_TYPE_NOISE:                // [NONE]
                return  K3DQMLAdapter::K3D_SUPPORT_NONE;

            case ObjectEx::SUPPORT_TYPE_LINE:                 // [WALL]
            case ObjectEx::SURFACE_LINE_ONLY:
                return K3DQMLAdapter::K3D_SUPPORT_WALL;

            case ObjectEx::SUPPORT_TYPE_POINT:                // [COLUMN]
            case ObjectEx::SURFACE_COLUMN_ONLY:
                return K3DQMLAdapter::K3D_SUPPORT_COLUMN;

            case ObjectEx::SUPPORT_TYPE_GUSSET:               // [SHORING]
            case ObjectEx::CYLINDER_GUSSET_TYPE:
                return K3DQMLAdapter::K3D_SUPPORT_SHORING;

            case ObjectEx::SUPPORT_TYPE_BLOCK:                // [SCAFFOLD]
            case ObjectEx::SURFACE_MANUAL_SCAFFOLD:
                return K3DQMLAdapter::K3D_SUPPORT_SCAFFOLD;

            case ObjectEx::SUPPORT_TYPE_TREE_COLUMN:          // [TREE]
                return K3DQMLAdapter::K3D_SUPPORT_TREE;

            default:
                return K3DQMLAdapter::K3D_SUPPORT_NONE;
        }
    }

    enum K3D_PRINT_ISSUE {
        K3D_PRINT_FLOATING_OBJECT,
        K3D_PRINT_ERROR_OBJECT,
        K3D_PRINT_INSIDE_PLATFORM,
        K3D_PRINT_NFC_AVAIABLE,
        K3D_PRINT_MATERIAL_READY,
        K3D_PRINT_PRINTER_READY,

        K3D_PRINT_ISSUE_TOTAL
    };
public:
    K3DQMLAdapter();
    static QObject* qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine);
    static K3DQMLAdapter* getInstance();
    ~K3DQMLAdapter();

    K3DOP_VOID _k3dActionChecks[K3D_ACTION_TOTAL];
    bool isK3DActionValid(int actionId);
    QVariantList _k3dActionVariantList;
    void setVariantList(const QVariantList& variantList);

    K3DOP _k3dOp[K3DOP_TOTAL];
    Q_INVOKABLE QVariant k3dRunOp(int k3dOpId, QVariantList paramList);
    
    K3DOP_VOID _k3dOpVoid[K3DOPVOID_TOTAL];
    Q_INVOKABLE QVariant k3dRunOpVoid(int k3dOpId);

    K3DOP_VAR  _k3dOpVariant[K3DOP_SINGLE_PARAM_TOTAL];
    Q_INVOKABLE QVariant k3dRunOpParam(int k3dOpId, QVariant param);

    K3DOP_VAR_VAR  _k3dOpDoubleVariant[K3DOP_DOUBLE_PARAM_TOTAL];
    Q_INVOKABLE QVariant k3dRunOpDoubleParam(int k3dOpId, QVariant param1, QVariant param2);

    K3DOP_VAR_VAR _k3dOpDialogAgent;
    Q_INVOKABLE QVariant k3dRunDialogOp(int k3dDialogId, int eventType);

    void registerOperations();

    K3DQuickImageProvider* objectTableImageProvider() { return &_objectTableImageProvider; }
    K3DQuickImageProvider* importedObjImageProvider() { return &_importedObjImageProvider; }
    K3DQuickImageProvider* shellImageProvider()       { return &_shellImageProvider; }

    int modalDialogId();
    Q_INVOKABLE void resetModalDialogId();
    void openQMLModalDialog(int dialogId); // Open the main modal dialog
    void openQMLDialog(int dialogId);      // Open a modeless dialog
    QObject* getK3DQMLMainModalDialog();
    QObject* getK3DQMLDialog(int qmlDialogId);

private:
    static K3DQMLAdapter* s_instance;

    // Global QML Quick Image Provider
    K3DQuickImageProvider _objectTableImageProvider;
    K3DQuickImageProvider _importedObjImageProvider;
    K3DQuickImageProvider _shellImageProvider;

    // Main Modal Dialog
    int _modalDialogId; // Id of the current shown modal dialog.
    QQmlProperty* _k3dQMLMainModalDialog;

    // Main Dialog List
    //QQmlProperty* _k3dQMLDialogList[K3DQMLAdapter::K3D_DIALOG_TOTAL];
};

#endif // K3D_QML_ADAPTER_H

