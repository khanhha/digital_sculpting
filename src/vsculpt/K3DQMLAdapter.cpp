#include "K3DQMLAdapter.h"
#include "mainwindow.h"

K3DQMLAdapter* K3DQMLAdapter::s_instance = nullptr;

const QString K3DQMLAdapter::CONTEXT_PROPERTY[K3DQMLAdapter::K3D_QML_CONTEXT_PROPERTY_TOTAL] = {
    "_k3dApp",                              // K3D_APPLICATION
    "_k3dMainWindow",                       // MAIN_WINDOW
    "_k3dMainQuickContainer",               // MAIN_QUICK_CONTAINER
    "_k3dQMLAdapter",                       // QML_ADAPTER

	"_k3dMainGLAgent",                      // MAIN_GLWIDGET_AGENT
                                            
    "_k3dSupportGLAgent",                   // SUPPORT_GL_AGENT

    "_k3dSupportTypeGLAgent",               // SUPPORT_TYPE_GL_AGENT

    "_k3dAddLevelSUpportGLAgent",           //ADD_LEVEL_SUPPORT_GL_AGENT
};

//!NOTE: THESE NAME MUST MATCH THE <dialog> in "property alias <dialog> in K3DBuilder.qml
const QString K3DQMLAdapter::QML_PROPERTY_MAIN_MODAL_DIALOG = "_mainModalDialog"; // K3DBuilder.qml's property _mainModalDialog
const QString K3DQMLAdapter::QML_PROPERTY_DIALOG[K3DQMLAdapter::K3D_DIALOG_TOTAL] = {
    "_mainWindow"                 ,      // K3D_MAIN_WINDOW,
    "_mainObjectTable"            ,      // K3D_MAIN_OBJECT_TABLE, 

    "_aboutDialog"                ,      // K3D_ABOUT_DIALOG,
    "_settingDialog"              ,      // K3D_SETTINGS_DIALOG,
    "_machineDialog"              ,      // K3D_MACHINE_DIALOG,
    "_machineAddDialog"           ,      // K3D_MACHINE_ADD_DIALOG,
    "_machineEditDialog"          ,      // K3D_MACHINE_EDIT_DIALOG,
    "_machineSelectBar"           ,      // K3D_MACHINE_SELECT_BAR,

    "_objImportFromProjectDialog" ,      // K3D_OBJ_IMPORT_FROM_PROJECT_DIALOG,
    "_objOrientateDialog"         ,      // K3D_OBJ_ORIENTATE_DIALOG,
    "_objMoveDialog"              ,      // K3D_OBJ_MOVE_DIALOG,
    "_objRescaleDialog"           ,      // K3D_OBJ_RESCALE_DIALOG,
    "_objDuplicateDialog"         ,      // K3D_OBJ_DUPLICATE_DIALOG,
    "_objArrangeDialog"           ,      // K3D_OBJ_ARRANGE_DIALOG,
    "_objMateDialog"              ,      // K3D_OBJ_MATE_DIALOG,
    
    "_objStitchDialog"            ,      // K3D_OBJ_STITCH_DIALOG,
    "_objNoteDialog"              ,      // K3D_OBJ_NOTE_DIALOG,
    "_objMeasureDialog"           ,      // K3D_OBJ_MEASURE_DIALOG,
    "_objMirrorDialog"            ,      // K3D_OBJ_MIRROR_DIALOG,
    "_objRemoveSharpDialog"       ,      // K3D_OBJ_REMOVE_SHARP_DIALOG,
    "_objCreateHoleDialog"        ,      // K3D_OBJ_CREATE_HOLE_DIALOG,
    "_objSmoothDialog"            ,      // K3D_OBJ_SMOOTH_DIALOG,
    "_objBasicCutDialog"          ,      // K3D_OBJ_BASIC_CUT_DIALOG,
    "_objAddLabelDialog"          ,      // K3D_OBJ_ADD_LABEL_DIALOG,
    "_createHollowDialog"         ,      // K3D_CREATE_HOLLOW_DIALOG,

    "_obj2SlcDialog"              ,      // K3D_OBJ_2SLC_DIALOG,
    "_objTrigReductionDialog"     ,      // K3D_OBJ_TRIG_REDUCTION_DIALOG,
    "_objTrigMeshDialog"          ,      // K3D_OBJ_TRIG_MESH_DIALOG,
    "_objShrinkWrapDialog"        ,      // K3D_OBJ_SHRINK_WRAP_DIALOG,
    "_objSharpenDialog"           ,      // K3D_OBJ_SHARPEN_DIALOG,
    "_objShellDialog"             ,      // K3D_SHELL_DIALOG,
    "_surfaceEditDialog"          ,      // K3D_SURFACE_EDIT_DIALOG,
    "_sliceViewBar"               ,      // K3D_SLICE_VIEW
    "_sliceExportDialog"          ,      // K3D_SLICE_EXPORT_DIALOG,
    "_slicePlayDialog"            ,      // K3D_SLICE_PLAY_DIALOG
    "_repairWizardDialog"         ,      // K3D_REPAIR_WIZARD_DIALOG,
    "_repairCustomDialog"         ,      // K3D_REPAIR_CUSTOM_DIALOG,
    "_supportDuplicateDialog"     ,      // K3D_SUPPORT_DUPLICATE_DIALOG,
    "_supportAddLevelDialog"      ,      // K3D_SUPPORT_ADD_LEVEL_DIALOG,
    "_print3DDialog"              ,      // K3D_PRINT_3D_DIALOG,
    "_fileSplitDialog"            ,      // K3D_FILE_SPLIT_DIALOG,

    "_objDiagnosticsTable"               // K3D_OBJECT_DIAGNOSTICS_TABLE

    ///* Add more dialog id here */
    //K3D_DIALOG_TOTAL
};

// [QML SINGLETON]
QObject* K3DQMLAdapter::qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return K3DQMLAdapter::getInstance();
}

// [C++ SINGLETON]
K3DQMLAdapter* K3DQMLAdapter::getInstance()
{
    if (!s_instance)
    { 
        s_instance = new K3DQMLAdapter();
    }

    return s_instance;
}


K3DQMLAdapter::K3DQMLAdapter()
    :_objectTableImageProvider(K3DQuickImageProvider(K3DQuickImageProvider::OBJECT_TABLE_IMAGE)),
     _importedObjImageProvider(K3DQuickImageProvider(K3DQuickImageProvider::IMPORTED_OBJECT_IMAGE)),
     _shellImageProvider(K3DQuickImageProvider(K3DQuickImageProvider::SHELL_IMAGE))
{
    if (s_instance)
    {
        delete s_instance;
    }
    s_instance = this;

    registerOperations();
}

K3DQMLAdapter::~K3DQMLAdapter()
{
}

bool K3DQMLAdapter::isK3DActionValid(int actionId)
{
    if (_k3dActionChecks[actionId] != nullptr) {
        return K3D_MEMFUNC_CALL(*MainWindow::getInstance(), _k3dActionChecks[actionId])().toBool();
    }
    else {
        return false;
    }
}

void K3DQMLAdapter::setVariantList(const QVariantList& variantList)
{

}

void K3DQMLAdapter::registerOperations()
{
    memset(_k3dActionChecks, 0x00, sizeof(_k3dActionChecks));
    /*
    for (int i = 0; i < K3D_ACTION_TOTAL; i++)
        _k3dActionChecks[i] = ...
    */

    // K3D OPERATION ============================================================================================
    memset(_k3dOp, 0x00, sizeof(_k3dOpVoid));
    _k3dOp[K3DOP_FIRST]                                             = nullptr;

    // K3D VOID OPERATION =======================================================================================
    memset(_k3dOpVoid, 0x00, sizeof(_k3dOpVoid));

    // K3DOP_QUICK_MENU -----------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_QUICK_MENU_NEW_PROJECT]                        = &MainWindow::cmdCreateNewEmptyProject;
    _k3dOpVoid[K3DOP_QUICK_MENU_OPEN_PROJECT]                       = &MainWindow::cmdOpenProject;
    _k3dOpVoid[K3DOP_QUICK_MENU_SAVE_PROJECT]                       = &MainWindow::cmdSaveProject;
    _k3dOpVoid[K3DOP_QUICK_MENU_UNDO]                               = &MainWindow::undo;
    _k3dOpVoid[K3DOP_QUICK_MENU_REDO]                               = &MainWindow::redo;

    // K3DOP_MAIN_MENU ------------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_MAIN_MENU_NEW_PROJECT]                         = &MainWindow::cmdCreateNewEmptyProject;
    _k3dOpVoid[K3DOP_MAIN_MENU_OPEN_PROJECT]                        = &MainWindow::cmdOpenProject;
    _k3dOpVoid[K3DOP_MAIN_MENU_SAVE_PROJECT]                        = &MainWindow::cmdSaveProject;
    _k3dOpVoid[K3DOP_MAIN_MENU_SAVE_PROJECT_AS]                     = &MainWindow::cmdSaveProjectAs;
    _k3dOpVoid[K3DOP_MAIN_MENU_OPEN_PROJECT_IMPORT]                 = &MainWindow::cmdImportFromProject;
    _k3dOpVoid[K3DOP_MAIN_MENU_RECENT_OBJECTS]                      = nullptr;
    _k3dOpVoid[K3DOP_MAIN_MENU_RECENT_PROJECTS]                     = nullptr;
    _k3dOpVoid[K3DOP_MAIN_MENU_MANAGE_3D_PRINTERS]                  = &MainWindow::cmdMachineSettings;
    _k3dOpVoid[K3DOP_MAIN_MENU_ADD_3D_PRINTERS]                     = &MainWindow::cmdAddMachine;
    _k3dOpVoid[K3DOP_MAIN_MENU_EDIT_3D_PRINTERS]                    = &MainWindow::cmdEditMachine;
    _k3dOpVoid[K3DOP_MAIN_MENU_ADD_SUPPORT_TYPE]                    = &MainWindow::cmdSupportTypeAdd;
    _k3dOpVoid[K3DOP_MAIN_MENU_EDIT_SUPPORT_TYPE]                   = &MainWindow::cmdSupportTypeEdit;
    _k3dOpVoid[K3DOP_MAIN_MENU_SELECT_3D_PRINTERS]                  = nullptr; // Machine is now loaded at initialization and presented in Machine Bar
    _k3dOpVoid[K3DOP_MAIN_MENU_PREFERENCES]                         = &MainWindow::cmdPreferences;
    _k3dOpVoid[K3DOP_MAIN_MENU_3D_PRINT]                            = &MainWindow::cmdPrint3D;

    // K3DOP_HELP_MENU ------------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_HELP_MENU_MANUAL_GUIDE]                        = &MainWindow::cmdKManual;
    _k3dOpVoid[K3DOP_HELP_MENU_REPORT_ISSUE]                        = &MainWindow::cmdReportIssue;

    // K3DOP_HOME_OBJECT ----------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_HOME_OBJECT_SAVE]                              = &MainWindow::cmdSaveObjects;
    _k3dOpVoid[K3DOP_HOME_OBJECT_IMPORT]                            = &MainWindow::cmdImportObjects;
    _k3dOpVoid[K3DOP_HOME_OBJECT_UNLOAD]                            = &MainWindow::cmdUnloadObjects;

    // K3DOP_HOME_ZOOM ------------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_HOME_ZOOM_OBJECTS]                             = &MainWindow::cmdZoomAll;
    _k3dOpVoid[K3DOP_HOME_ZOOM_PLATFORM]                            = &MainWindow::cmdZoomPlatform;
    _k3dOpVoid[K3DOP_HOME_ZOOM_WINDOWS]                             = &MainWindow::cmdZoomWindow;
    _k3dOpVoid[K3DOP_HOME_ZOOM_IN]                                  = &MainWindow::cmdZoomIn;
    _k3dOpVoid[K3DOP_HOME_ZOOM_OUT]                                 = &MainWindow::cmdZoomOut;


    // K3DOP_HOME_VIEW ------------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_HOME_VIEW_OBJECT_SHADE]                        = &MainWindow::cmdSetShadeMode;
    _k3dOpVoid[K3DOP_HOME_VIEW_OBJECT_WIREFRAME]                    = &MainWindow::cmdSetWireframeMode;
    _k3dOpVoid[K3DOP_HOME_VIEW_OBJECT_TRIANGLE]                     = &MainWindow::cmdSetTriangleMode;
    _k3dOpVoid[K3DOP_HOME_VIEW_OBJECT_SHADEWIRE]                    = &MainWindow::cmdSetShadeWireMode;
    _k3dOpVoid[K3DOP_HOME_VIEW_OBJECT_BBBOX]                        = &MainWindow::cmdSetBoundingBoxMode;
    _k3dOpVoid[K3DOP_HOME_VIEW_OBJECT_TRANSPARENT]                  = &MainWindow::cmdToggleTransparentViewMode;
    _k3dOpVoid[K3DOP_HOME_VIEW_ANGLE_TOP_FRONT]                     = &MainWindow::cmdSetIsoView;
    _k3dOpVoid[K3DOP_HOME_VIEW_ANGLE_TOP]                           = &MainWindow::cmdSetTopView;
    _k3dOpVoid[K3DOP_HOME_VIEW_ANGLE_BOTTOM]                        = &MainWindow::cmdSetBottomView;
    _k3dOpVoid[K3DOP_HOME_VIEW_ANGLE_BACK]                          = &MainWindow::cmdSetBackView;
    _k3dOpVoid[K3DOP_HOME_VIEW_ANGLE_FRONT]                         = &MainWindow::cmdSetFrontView;
    _k3dOpVoid[K3DOP_HOME_VIEW_ANGLE_LEFT]                          = &MainWindow::cmdSetLeftView;
    _k3dOpVoid[K3DOP_HOME_VIEW_ANGLE_RIGHT]                         = &MainWindow::cmdSetRightView;


    // K3DOP_HOME_MARK ------------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_HOME_MARK_ALL]                                 = &MainWindow::cmdSelectAll;
    _k3dOpVoid[K3DOP_HOME_MARK_CLEAR]                               = &MainWindow::cmdClearSelection;
    _k3dOpVoid[K3DOP_HOME_MARK_TOGGLE]                              = &MainWindow::cmdToggleSelection;
    _k3dOpVoid[K3DOP_HOME_MARK_DELETE]                              = &MainWindow::cmdDeleteSelected;

    // K3DOP_HOME_SHOW ------------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_HOME_SHOW_PLATFORM]                            = &MainWindow::cmdShowHidePlatform;
    _k3dOpVoid[K3DOP_HOME_SHOW_COORD]                               = &MainWindow::cmdShowHideCoordinateSystem;
    _k3dOpVoid[K3DOP_HOME_SHOW_ORIENTATION]                         = &MainWindow::cmdShowHideOrientationIndication;
    _k3dOpVoid[K3DOP_HOME_SHOW_ORIENTATION_BOX]                     = &MainWindow::cmdShowHideOrientationBoxIndication;
    _k3dOpVoid[K3DOP_HOME_SHOW_DIMENSIONS]                          = &MainWindow::cmdShowHidePartDimensions;
    _k3dOpVoid[K3DOP_HOME_SHOW_TAGNAME]                             = &MainWindow::cmdShowHideTagName;
    _k3dOpVoid[K3DOP_HOME_SHOW_RULER]                               = &MainWindow::cmdShowHideRuler;
	_k3dOpVoid[K3DOP_HOME_SHOW_OBJ_INFO]                            = &MainWindow::cmdShowHideObjInfo;
	_k3dOpVoid[K3DOP_HOME_SHOW_MARK_MENU]                           = &MainWindow::cmdShowHideMarkMenu;

    // K3DOP_TOOLS_BASIC ----------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_TOOLS_BASIC_MOVE_OBJ]                          = &MainWindow::cmdMoveObjects;
    _k3dOpVoid[K3DOP_TOOLS_BASIC_ROTATE_OBJ]                        = &MainWindow::cmdOrientObjects; //&MainWindow::cmdRotateObjects;
    _k3dOpVoid[K3DOP_TOOLS_BASIC_RESCALE_OBJ]                       = &MainWindow::cmdRescaleObjects;
    _k3dOpVoid[K3DOP_TOOLS_BASIC_DUPLICATE_OBJ]                     = &MainWindow::cmdDuplicateObjects;
    _k3dOpVoid[K3DOP_TOOLS_BASIC_MIRROR_OBJ]                        = &MainWindow::cmdMirrorObjects;
    _k3dOpVoid[K3DOP_TOOLS_BASIC_ARRANGE_OBJ]                       = &MainWindow::cmdArrangeObjects;
    _k3dOpVoid[K3DOP_TOOLS_BASIC_MATE_OBJ]                          = &MainWindow::cmdMateObjects;
    _k3dOpVoid[K3DOP_TOOLS_BASIC_EXPORT_IMG]                        = &MainWindow::cmdExportCurrentImage;

    // K3DOP_TOOLS_ADV ------------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_TOOLS_ADV_HOLLOW]                              = &MainWindow::cmdCreateHollow;
    _k3dOpVoid[K3DOP_TOOLS_ADV_WRAP]                                = &MainWindow::cmdCreateShrinkWrap;
    _k3dOpVoid[K3DOP_TOOLS_ADV_MERGE]                               = &MainWindow::cmdMergeObjects;
    _k3dOpVoid[K3DOP_TOOLS_ADV_CREATE_HOLES]                        = &MainWindow::cmdCreateHole;
    _k3dOpVoid[K3DOP_TOOLS_ADV_TRIG_REDUCE]                         = &MainWindow::cmdTriangleReduction;
    _k3dOpVoid[K3DOP_TOOLS_ADV_SMOOTH]                              = &MainWindow::cmdSmoothing;
    _k3dOpVoid[K3DOP_TOOLS_ADV_CUT]                                 = &MainWindow::cmdBasicCut;
    _k3dOpVoid[K3DOP_TOOLS_ADV_ADD_LABEL]                           = &MainWindow::cmdCreateLabel;
    _k3dOpVoid[K3DOP_TOOLS_ADV_ADD_LEVEL_SUPPORT]                   = &MainWindow::cmdAddLevelSupport;
        
    // K3DOP_TOOLS_ADD_LEVEL_SUPPORT-----------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_TOOLS_ADD_LEVEL_ARRANGE]                       = &MainWindow::cmdAddLevelSupportArrange;
    _k3dOpVoid[K3DOP_TOOLS_ADD_LEVEL_REMOVE]                        = &MainWindow::cmdAddLevelSupportRemove;
    _k3dOpVoid[K3DOP_TOOLS_ADD_LEVEL_SET_HEIGHT]                    = &MainWindow::cmdAddLevelSupportSetHeight;
    _k3dOpVoid[K3DOP_TOOLS_ADD_LEVEL_SETTING]                       = &MainWindow::cmdAddLevelSupportSetting;

    // K3DOP_TOOLS_ANNOTATIONS ----------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_TOOLS_ANNOTATIONS_MEASURE]                     = &MainWindow::cmdMeasureObjects;
    _k3dOpVoid[K3DOP_TOOLS_ANNOTATIONS_ADD_NOTE]                    = &MainWindow::cmdAddNewNote;

    // K3DOP_TOOLS_SLICE ----------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_TOOLS_SLICE_VIEW]                              = &MainWindow::cmdSliceView;
    _k3dOpVoid[K3DOP_TOOLS_SLICE_EXPORT]                            = &MainWindow::cmdExportSlices;
    _k3dOpVoid[K3DOP_TOOLS_SLICE_PLAY_SETTING]                      = &MainWindow::cmdSlicePlayDialog;
    
    // K3DOP_TOOLS_SUPPORT --------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_TOOLS_SUPPORT_AUTO]                            = &MainWindow::cmdCreateAutomaticSupport;
    _k3dOpVoid[K3DOP_TOOLS_SUPPORT_ADV]                             = &MainWindow::cmdCreateAdvanceSupport;
    _k3dOpVoid[K3DOP_TOOLS_SUPPORT_MANUAL]                          = &MainWindow::cmdCreateManualSupport;
    
    // K3DOP_REPAIR_AUTOMATIC -----------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_REPAIR_AUTOMATIC_WIZARD]                       = &MainWindow::cmdStartFixWizard;
    _k3dOpVoid[K3DOP_REPAIR_AUTOMATIC_CUSTOM]                       = &MainWindow::cmdCustomFixObjects;

    // K3DOP_REPAIR_MANUAL --------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_AUTO]             = &MainWindow::cmdAutomaticFixInvertedNormals;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_TRIG]             = &MainWindow::cmdInvertTriangles;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_SHELL]            = &MainWindow::cmdInvertShells;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_PART]             = &MainWindow::cmdInvertParts;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_INVERTED_TRIGS_MARKED]           = &MainWindow::cmdInvertMarked;

    _k3dOpVoid[K3DOP_REPAIR_MANUAL_STITCHING_AUTO]                  = &MainWindow::cmdAutomaticStitch;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_STITCHING_MANUAL]                = &MainWindow::cmdManualStitch;

    _k3dOpVoid[K3DOP_REPAIR_MANUAL_SHELLS_AUTO]                     = &MainWindow::cmdAutomaticFixNoiseShell;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_SHELLS_BAD]                      = &MainWindow::cmdAutomaticFixNoiseShell;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_SHELLS_ADV]                      = &MainWindow::cmdManualFixShell;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_SHELLS_MERGE]                    = &MainWindow::cmdMergeShellsForMarkedObject;

    _k3dOpVoid[K3DOP_REPAIR_MANUAL_HOLES_AUTO]                      = &MainWindow::cmdAutomaticFixHole;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_HOLES_PLANAR]                    = &MainWindow::cmdFixPlannarHoles;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_HOLES_FREEDOM]                   = &MainWindow::cmdFixFreeHoles;

    _k3dOpVoid[K3DOP_REPAIR_MANUAL_OVERLAPS_AUTO]                   = &MainWindow::cmdAutomaticFixOverlappingTriangle;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_OVERLAPS_REMOVE_SHARP]           = &MainWindow::cmdRemoveSharp;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_OVERLAPS_DETECT]                 = &MainWindow::cmdDetectOverlappingTriangles;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_OVERLAPS_CLEAR_MARK]             = &MainWindow::cmdClearOverlappingTriangles;

    _k3dOpVoid[K3DOP_REPAIR_MANUAL_TRIANGLES_CREATE]                = &MainWindow::cmdAddTriangles;
    _k3dOpVoid[K3DOP_REPAIR_MANUAL_TRIANGLES_DELETE]                = &MainWindow::cmdDeleteTriangles;

    // K3DOP_DIAGNOSTICS_TOOLS ----------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_DIAG_FIX_INVERTED_NORMAL]                      = &MainWindow::individualFixInvertedNormal;
    _k3dOpVoid[K3DOP_DIAG_FIX_BAD_EDGE]                             = &MainWindow::individualFixBadEdges;
    _k3dOpVoid[K3DOP_DIAG_FIX_STITCH]                               = &MainWindow::individualStitch;
    _k3dOpVoid[K3DOP_DIAG_FIX_HOLE]                                 = &MainWindow::individualFixHole;
    _k3dOpVoid[K3DOP_DIAG_FIX_SHELL]                                = &MainWindow::individualMergeAllShells; // or individualFixShells() ?
    _k3dOpVoid[K3DOP_DIAG_FIX_NOISE_SHELL]                          = &MainWindow::individualFixNoiseShell;
    _k3dOpVoid[K3DOP_DIAG_FIX_OVERLAPPING]                          = &MainWindow::individualFixOverlappingTriangle;
    _k3dOpVoid[K3DOP_DIAG_FIX_INTERSECTION]                         = &MainWindow::individualfixIntersection;
    _k3dOpVoid[K3DOP_DIAG_FIX_ALL_PROBLEMS]                         = &MainWindow::repairAllProblems;

    _k3dOpVoid[K3DOP_DIAG_UPDATE_INVERTED_NORMAL]                   = &MainWindow::updatedInvertNormal;
    _k3dOpVoid[K3DOP_DIAG_UPDATE_BAD_EDGE]                          = &MainWindow::updateBadEdge;
    _k3dOpVoid[K3DOP_DIAG_UPDATE_NEAR_BAD_EDGE]                     = &MainWindow::updatedNearBadEdge;
    _k3dOpVoid[K3DOP_DIAG_UPDATE_BAD_CONTOUR]                       = &MainWindow::updatedBadContour;
    _k3dOpVoid[K3DOP_DIAG_UPDATE_SHELL]                             = &MainWindow::updatedShell;
    _k3dOpVoid[K3DOP_DIAG_UPDATE_INTERSECTION]                      = &MainWindow::updatedIntersection;
    _k3dOpVoid[K3DOP_DIAG_UPDATE_ALL_PROBLEMS]                      = &MainWindow::updateAllProblems;

    // K3DOP_SUPPORT_MAIN ---------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_SUPPORT_CONCLUDE]                              = &MainWindow::cmdConcludeExitSupportMode;
    _k3dOpVoid[K3DOP_SUPPORT_CANCEL]                                = &MainWindow::cmdCancelSupportMode;
    _k3dOpVoid[K3DOP_SUPPORT_REGENERATE]                            = &MainWindow::cmdRegenerateSupport;
    _k3dOpVoid[K3DOP_SUPPORT_REMOVE_ALL]                            = &MainWindow::cmdRemoveAllSupports;
    
    _k3dOpVoid[K3DOP_SUPPORT_SURFACE_REMOVE]                        = &MainWindow::cmdRemoveSelectSupport;
    _k3dOpVoid[K3DOP_SUPPORT_SURFACE_CREATE]                        = &MainWindow::cmdSupportCreateArea;
    _k3dOpVoid[K3DOP_SUPPORT_SURFACE_EDIT]                          = &MainWindow::cmdSupport_EditSurface;
    _k3dOpVoid[K3DOP_SUPPORT_AREA_EDIT]                             = &MainWindow::cmdSupport_EditArea;
    _k3dOpVoid[K3DOP_SUPPORT_SURFACE_DETAILS]                       = &MainWindow::showSurfaceDetails;

    _k3dOpVoid[K3DOP_SUPPORT_ADD_COLUMN]                            = &MainWindow::cmdSupport_EditAddPoint;
    _k3dOpVoid[K3DOP_SUPPORT_ADD_WALL]                              = &MainWindow::cmdSupport_EditAddLine;
    _k3dOpVoid[K3DOP_SUPPORT_ADD_TREE]                              = &MainWindow::cmdSupport_EditAddTree;
    _k3dOpVoid[K3DOP_SUPPORT_ADD_SCAFFOLD]                          = &MainWindow::cmdSupport_EditAddScaffold;
    _k3dOpVoid[K3DOP_SUPPORT_ADD_SHORING]                           = &MainWindow::cmdSupport_EditAddGusset;
    _k3dOpVoid[K3DOP_SUPPORT_ADD_DUPLICATE]                         = &MainWindow::cmdSupport_DuplicateSurface;

    _k3dOpVoid[K3DOP_SUPPORT_MARK_LINE]                             = &MainWindow::cmdToggleMarkLineSupport;
    _k3dOpVoid[K3DOP_SUPPORT_MARK_TOGGLE]                           = &MainWindow::cmdToggleMarkSupport;
    _k3dOpVoid[K3DOP_SUPPORT_MARK_ALL]                              = &MainWindow::cmdSupportMarkAll;
    _k3dOpVoid[K3DOP_SUPPORT_MARK_CLEAR]                            = &MainWindow::cmdSupportClearMarked;
    _k3dOpVoid[K3DOP_SUPPORT_MARK_DELETE]                           = &MainWindow::cmdSupportDeleteMarked;

    // K3DOP_SUPPORT_VIEW ---------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_SUPPORT_VIEW_ALL_SUPPORTS]                     = &MainWindow::cmdViewAllSupports;
    _k3dOpVoid[K3DOP_SUPPORT_VIEW_BASE]                             = &MainWindow::cmdViewBaseSupport;
    _k3dOpVoid[K3DOP_SUPPORT_VIEW_ALL_SURFACES]                     = &MainWindow::cmdViewAllSurfaces;
    _k3dOpVoid[K3DOP_SUPPORT_VIEW_COLOR_SCHEMER]                    = &MainWindow::cmdViewSupportedSurfaceColorSchemer;

    // K3D_CONTEXT_MENU_INDIVIDUAL -------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_CONTEXT_MENU_OBJECT_TITLE]                     = nullptr;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_IMPORT_OBJECTS]                   = &MainWindow::cmdImportObjects;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_UNLOAD_SELECTED_OBJS]             = &MainWindow::cmdUnloadObjects;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_RECENT_OBJS]                      = nullptr;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_VIEW_MODE]                        = nullptr;

    	_k3dOpVoid[K3DOP_CONTEXT_MENU_SUPPORT_NONE]                     = &MainWindow::changeSupportTypeToNone;
    	_k3dOpVoid[K3DOP_CONTEXT_MENU_SUPPORT_SCAFFOLD]                 = &MainWindow::changeSupportTypeToBlock;
    	_k3dOpVoid[K3DOP_CONTEXT_MENU_SUPPORT_WALL]                     = &MainWindow::changeSupportTypeToLine;
    	_k3dOpVoid[K3DOP_CONTEXT_MENU_SUPPORT_COLUMN]                   = &MainWindow::changeSupportTypeToPoint;
    	_k3dOpVoid[K3DOP_CONTEXT_MENU_SUPPORT_SHORING]                  = &MainWindow::changeSupportTypeToGusset;

    _k3dOpVoid[K3DOP_CONTEXT_MENU_SELECT_ALL]                       = &MainWindow::selectAllObjects;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_CLEAR_SELECTION]                  = &MainWindow::clearSelectionObject;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_INVERT_SELECTION]                 = &MainWindow::invertSelectionObject;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_SHOW_ALL_OBJS]                    = &MainWindow::allObjectsVisible;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_HIDE_ALL_OBJS]                    = &MainWindow::clearVisibleObjects;
    _k3dOpVoid[K3DOP_CONTEXT_MENU_HIDE_UNSELECTED_OBJS]             = &MainWindow::hideUnselectedObjects;

    // K3DOP_OBJECT_LIST ----------------------------------------------------------------------------------------
    _k3dOpVoid[K3DOP_OBJECT_LIST_SELECT_ALL]                        = &MainWindow::selectAllObjects;
    _k3dOpVoid[K3DOP_OBJECT_LIST_CLEAR_SELECTION]                   = &MainWindow::clearSelectionObject;
    _k3dOpVoid[K3DOP_OBJECT_LIST_INVERT_SELECTION]                  = &MainWindow::invertSelectionObject;
    _k3dOpVoid[K3DOP_OBJECT_LIST_REPAIR_ALL]                        = &MainWindow::cmdStartFixWizard;

    // ##########################################################################################################
    // ==========================================================================================================
    // K3D INT OPERATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    memset(_k3dOpVariant, 0x00, sizeof(_k3dOpVariant));

    // K3DOP_HELP_MENU ------------------------------------------------------------------------------------------
    //
    _k3dOpVariant[K3DOP_HELP_MENU_ABOUT]                            = &MainWindow::cmdAbout;

    // K3DOP_QUICK_MENU -----------------------------------------------------------------------------------------
    //
    _k3dOpVariant[K3DOP_QUICK_MENU_MOUSE_SELECT]                    = &MainWindow::toggleSelectPart;
    _k3dOpVariant[K3DOP_QUICK_MENU_PICK_PLACE]                      = &MainWindow::togglePickAndPlaceMode;
    _k3dOpVariant[K3DOP_QUICK_MENU_ROTATE]                          = &MainWindow::toggleRotationMode;
    _k3dOpVariant[K3DOP_QUICK_MENU_PANNING]                         = &MainWindow::togglePanMode;

    // K3DOP_TOOLS_ADD_LEVEL_SUPPORT-----------------------------------------------------------------------------
    _k3dOpVariant[K3DOP_TOOLS_ADD_LEVEL_VIEW]                       = &MainWindow::cmdAddLevelSupportView;
    _k3dOpVariant[K3DOP_TOOLS_ADV_SHOW_LEVEL_SUPPORT]               = &MainWindow::cmdViewAllLevelSupport;

    // K3DOP_MARK_MENU -----------------------------------------------------------------------------------------
    //
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES]                        = &MainWindow::cmdToggleSelectTriangles;
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES_WINDOWS]                = &MainWindow::cmdToggleSelectTrianglesByWindows;
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES_BRUSH]                  = &MainWindow::cmdToggleSelectTrianglesByBrush;
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES_BRUSH1]                 = &MainWindow::cmdSelectTrianglesByBrush1;
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES_BRUSH2]                 = &MainWindow::cmdSelectTrianglesByBrush2;
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES_BRUSH3]                 = &MainWindow::cmdSelectTrianglesByBrush3;
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES_BRUSH4]                 = &MainWindow::cmdSelectTrianglesByBrush4;
    _k3dOpVariant[K3DOP_HOME_MARK_TRIANGLES_BRUSH5]                 = &MainWindow::cmdSelectTrianglesByBrush5;
    _k3dOpVariant[K3DOP_HOME_MARK_PLANES]                           = &MainWindow::cmdToggleSelectPlane;
    _k3dOpVariant[K3DOP_HOME_MARK_SURFACES]                         = &MainWindow::cmdToggleSelectSurfaces;
    _k3dOpVariant[K3DOP_HOME_MARK_SHELLS]                           = &MainWindow::cmdToggleSelectShells;

    // K3DOP_RECENT_OBJECTS_PROJECTS ----------------------------------------------------------------------------
    //
    _k3dOpVariant[K3DOP_RECENT_OBJECT]                              = &MainWindow::cmdOpenRecentObject;
    _k3dOpVariant[K3DOP_RECENT_PROJECT]                             = &MainWindow::cmdOpenRecentProject;
    
    // K3D_OBJECT_TABLE -----------------------------------------------------------------------------------------
    _k3dOpVariant[K3DOP_OBJECT_LIST_VISIBLE_SINGLE_OBJ]             = &MainWindow::cmdToggleVisisbleObject;
    _k3dOpVariant[K3DOP_OBJECT_LIST_LOCK_SINGLE_OBJ]                = &MainWindow::cmdLockObject;
    _k3dOpVariant[K3DOP_OBJECT_LIST_UNLOAD_SINGLE_OBJ]              = &MainWindow::cmdUnloadSingleObject;
    _k3dOpVariant[K3DOP_OBJECT_LIST_REPAIR_SINGLE_OBJ]              = &MainWindow::cmdRepairObject;
    _k3dOpVariant[K3DOP_OBJECT_LIST_SET_OBJ_COLOR]                  = &MainWindow::cmdSetObjectColor;

    _k3dOpVariant[K3DOP_CONTEXT_MENU_RENAME_OBJ]                    = &MainWindow::cmdRenameObject;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_REPAIR_OBJ]                    = &MainWindow::contextMenuRepair;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_CREATE_SUPPORT]                = &MainWindow::contextMenuSupport;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_CONVERT_TO_SLC]                = &MainWindow::contextMenuConvertToSLC;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_CREATE_TRIANGLES]              = &MainWindow::contextMenuCreateTrigMesh;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_MOVE_OBJ]                      = &MainWindow::contextMenuTranslate;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_ROTATE_OBJ]                    = &MainWindow::contextMenuOrientate;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_RESCALE_OBJ]                   = &MainWindow::contextMenuRescale;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_DUPLICATE_OBJ]                 = &MainWindow::contextMenuDuplicate;
    
    _k3dOpVariant[K3DOP_CONTEXT_MENU_VIEW_SHADE]                    = &MainWindow::cmdSetShadeModeIndividual;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_VIEW_WIREFRAME]                = &MainWindow::cmdSetWireframeModeIndividual;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_VIEW_TRIANGLE]                 = &MainWindow::cmdSetTriangleModeIndividual;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_VIEW_SHADEWIRE]                = &MainWindow::cmdSetShadeWireModeIndividual;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_VIEW_BBBOX]                    = &MainWindow::cmdSetBoundingBoxModeIndividual;
    _k3dOpVariant[K3DOP_CONTEXT_MENU_VIEW_TRANSPARENT]              = &MainWindow::cmdSetTransparencyModeIndividual;

    // K3DOP_PROGRESS_BAR ---------------------------------------------------------------------------------------
	_k3dOpVariant[K3DOP_PROGRESSBAR_INFORM_VISIBILITY]              = &MainWindow::cmdSetProgressBarIsNowVisible;

    // K3DOP_SLICE_VIEW -----------------------------------------------------------------------------------------
    _k3dOpVariant[K3DOP_SLICE_VIEW_REVOKE]                          = &MainWindow::cmdSliceViewRevoke;

    // K3DOP_MARK_BRUSH -----------------------------------------------------------------------------------------
    _k3dOpVariant[K3DOP_MARK_TRIANGLES_BRUSH_SIZE_CHANGE]           = &MainWindow::cmdSelectTrianglesBrushSizeChange;

    // K3DOP_SUPPORT --------------------------------------------------------------------------------------------
    _k3dOpVariant[K3DOP_SUPPORT_SURFACE_SELECT]                     = &MainWindow::cmdSelectSupport;
    _k3dOpVariant[K3DOP_SUPPORT_SURFACE_MERGE]                      = &MainWindow::cmdSupport_MergeSurface;
    // ##########################################################################################################
    // ==========================================================================================================
    // K3D DOUBLE PARAM OPERATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    memset(_k3dOpDoubleVariant, 0x00, sizeof(_k3dOpDoubleVariant));

    // K3DOP_MAIN_CENTRAL_QUICK_CONTAINER -----------------------------------------------------------------------
    //
    _k3dOpDoubleVariant[K3DOP_MAIN_QUICK_CONTAINER_MOUSE_EVENT]     = &MainWindow::cmdMainQuickContainerMouseEvent;

    // K3DOP_MAIN_TAB -------------------------------------------------------------------------------------------
    // 
    _k3dOpDoubleVariant[K3DOP_MAIN_TAB_MAIN_FUNCTION]               = &MainWindow::cmdSelectTab;
    _k3dOpDoubleVariant[K3DOP_MAIN_TAB_SUB_FUNCTION]                = &MainWindow::cmdCloseTab;

    // ##########################################################################################################
    // ==========================================================================================================
    // K3D OPERATION DIALOG AGENT FUNCT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    _k3dOpDialogAgent = &MainWindow::onDialogEvent;
}

QVariant K3DQMLAdapter::k3dRunOp(int k3dOpId, QVariantList paramList)
{
    if (k3dOpId >= 0 && k3dOpId < K3DOP_TOTAL) {
        //MainWindow::getInstance()->getMainGLWidget()->hideAllContextMenus();
        K3D_OPERATION_INVOKE(k3dOpId, paramList);
    }
    else return -1;
}

QVariant K3DQMLAdapter::k3dRunOpVoid(int k3dOpId)
{
    if (k3dOpId >= 0 && k3dOpId < K3DOPVOID_TOTAL) {
        //MainWindow::getInstance()->getMainGLWidget()->hideAllContextMenus();
        K3D_VOID_OPERATION_INVOKE(k3dOpId);
    }
    else return -1;
}

QVariant K3DQMLAdapter::k3dRunOpParam(int k3dOpId, QVariant param)
{
    if (k3dOpId >= 0 && k3dOpId < K3DOP_SINGLE_PARAM_TOTAL) {
        //MainWindow::getInstance()->getMainGLWidget()->hideAllContextMenus();
        K3D_1VARIANT_OPERATION_INVOKE(k3dOpId, param);
    }
    else return -1;
}

QVariant K3DQMLAdapter::k3dRunOpDoubleParam(int k3dOpId, QVariant param1, QVariant param2)
{
    if (k3dOpId >= 0 && k3dOpId < K3DOP_DOUBLE_PARAM_TOTAL) {
        //MainWindow::getInstance()->getMainGLWidget()->hideAllContextMenus();
        K3D_2VARIANT_OPERATION_INVOKE(k3dOpId, param1, param2);
    }
    else return -1;
}

QVariant K3DQMLAdapter::k3dRunDialogOp(int k3dDialogId, int eventType)
{
    if (k3dDialogId >= 0 && k3dDialogId < K3D_DIALOG_TOTAL) {
        //MainWindow::getInstance()->getMainGLWidget()->hideAllContextMenus();
        K3D_DIALOG_OPERATION_INVOKE(k3dDialogId, eventType);
    }
    else return -1;
}

QObject* K3DQMLAdapter::getK3DQMLMainModalDialog()
{
    QQmlProperty prop(KsGlobal::k3dQMLCom(), K3DQMLAdapter::QML_PROPERTY_MAIN_MODAL_DIALOG);
    QObject * obj = qvariant_cast<QObject *>(prop.read()); //!!!
    return obj;

	/* OTHER WAY :
	QVariant var;
	K3D_QML_INVOKE_RET(getMainModalDialog, var);
	return qvariant_cast<QObject *>(var);
	*/
}

QObject* K3DQMLAdapter::getK3DQMLDialog(int qmlDialogId)
{ 
    assert(qmlDialogId >= 0 && qmlDialogId < K3DQMLAdapter::K3D_DIALOG_TOTAL);
    //QString str = K3DQMLAdapter::QML_PROPERTY_DIALOG[qmlDialogId];
    QQmlProperty prop(KsGlobal::k3dQMLCom(), K3DQMLAdapter::QML_PROPERTY_DIALOG[qmlDialogId]);
    QObject * obj = qvariant_cast<QObject *>(prop.read()); //!!!
    return obj;

	/* OTHER WAY :
	QVariant var;
	K3D_QML_INVOKE_RET(getMainModelessDialog, var, qmlDialogId);
	return qvariant_cast<QObject *>(var);
	*/
}

int K3DQMLAdapter::modalDialogId()
{
    return _modalDialogId;
}

void K3DQMLAdapter::resetModalDialogId()
{
    _modalDialogId = -1;
}

/**********************************************************************************************
 * @fn  void K3DQMLAdapter::openQMLModalDialog(int dialogId)
 *
 * @brief   Opens the qml main modal dialog.
 *
 * @author  Duc Than
 * @date    5/20/2015
 *
 * @param   dialogId    Identifier for the dialog.
 **************************************************************************************************/

void K3DQMLAdapter::openQMLModalDialog(int dialogId)
{
    assert(dialogId >= 0 && dialogId < K3DQMLAdapter::K3D_DIALOG_TOTAL);

    _modalDialogId = dialogId;
    K3D_QML_INVOKE_I(openModalDialog, dialogId);
}

/**********************************************************************************************
 * @fn  void K3DQMLAdapter::openQMLDialog(int dialogId)
 *
 * @brief   Opens a qml modeless dialog.
 *
 * @author  Duc Than
 * @date    5/20/2015
 *
 * @param   dialogId    Identifier for the dialog.
 **************************************************************************************************/

void K3DQMLAdapter::openQMLDialog(int dialogId)
{
    assert(dialogId >= 0 && dialogId < K3DQMLAdapter::K3D_DIALOG_TOTAL);
    K3D_QML_INVOKE_I(openModelessDialog, dialogId);
}