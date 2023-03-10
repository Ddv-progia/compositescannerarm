#ADD_DEFINITIONS(
 # -D_SCL_SECURE_NO_WARNINGS 
 # -DBOOST_RESULT_OF_USE_DECLTYPE
 # -DBOOST_ALL_DYN_LINK
 # -DFUSION_MAX_MAP_SIZE=40
 # -DFUSION_MAX_VECTOR_SIZE=40)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

INCLUDE_DIRECTORIES(DevTalk-Drivers-APL-Techno-Motion
  PUBLIC  ${DevTalk-Interface_SOURCE_DIR}
          ${DevTalk-Interface_BINARY_DIR}
  ${DevTalk-Interface_SOURCE_DIR}
  ${DevTalk-Interface_BINARY_DIR}
  ${UtsCommonLibrary_SOURCE_DIR}/include
  ${UtsCommonLibrary_BINARY_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${BERKELEY_DB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
  ${EIGEN3_INCLUDE_DIRS}
  ${FFTW_INCLUDE_DIRS}
  ${QT_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTSCRIPT_INCLUDE_DIR}
  ${QT_QTSCRIPTTOOLS_INCLUDE_DIR}
  ${QWT_INCLUDE_DIRS}
  ${ZEROC_ICE_CORE_INCLUDE_DIR}
  ${PUGIXML_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS})

LINK_DIRECTORIES(
  ${BERKELEY_DB_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
  ${FFTW_LIBRARY_DIRS})

QT5_ADD_RESOURCES(CompositeScanner_Generated_Resources
  Gui/tango-icons.qrc)

QT5_WRAP_UI(CompositeScanner_Generated_UI
  Gui/AssembleScanDialog.ui
  Gui/AssignColorForColorBarDialog.ui
  Gui/MainWindow.ui
  Gui/ManualControlDialog.ui
  Gui/ProcessingParametersDialog.ui
  Gui/SingleCommandPanel.ui
  Gui/KernelDialog.ui
  Gui/DefectClassificationWidget.ui)

SET(CompositeScanner_Data_FILES 
  Core/DevicesConfiguration.rtg
  Core/ScanData.rtg
  )
  
 
RTGEN_TO_CPP(CompositeScanner_Generated_Data
  OUTPUT_DIRECTORY Core
  INCLUDE_DIRECTORY Core
  ${CompositeScanner_Data_FILES})


add_executable(CompositeScanner #WIN32
  ${CompositeScanner_Generated_Resources}
  ${CompositeScanner_Generated_UI}
  ${CompositeScanner_Generated_Data}

  Core/AssembleScanTask.cc
  Core/AssembleScanTask.hh
  Core/BackgroundTaskExecutor.cc
  Core/BackgroundTaskExecutor.hh
  Core/ConfigurationLocator.cc
  Core/ConfigurationLocator.hh
  Core/CommonXmlLoad.hh
  Core/CommonXmlSave.hh
  Core/Devices.cc
  Core/Devices.hh
  Core/DevicesConfiguration.rtg
  Core/InitializeMediaFoundationTask.cc
  Core/InitializeMediaFoundationTask.hh
  Core/LoadScanTask.cc
  Core/LoadScanTask.hh
  Core/ObjectKeeper.cc
  Core/ObjectKeeper.hh
  Core/ProgressReportingTask.cc
  Core/ProgressReportingTask.hh
  Core/PersistentVariable.hh
  Core/SaveScanTask.cc
  Core/SaveScanTask.hh
  Core/ScanAlgorithms.cc
  Core/ScanAlgorithms.hh
  Core/ScanData.rtg
  Core/ScanDataMetatypes.hh
  Core/ScanDataPlots.cc
  Core/ScanDataPlots.hh
  Core/ScanDefectRenderingTask.cc
  Core/ScanDefectRenderingTask.hh
  Core/ScanFactory.cc
  Core/ScanFactory.hh
  Core/ScanCollector.cc
  Core/ScanCollector.hh
  Core/ScanIO.cc
  Core/ScanIO.hh
  Core/ScanProcessingTask.cc
  Core/ScanProcessingTask.hh
  Core/WMACompression.cc
  Core/WMACompression.hh
  Core/ImageProcessing.cc
  Core/ImageProcessing.hh

  Core/Qwt/ColumnPolynomialRasterData.cc
  Core/Qwt/ColumnPolynomialRasterData.hh
  Core/Qwt/DefectPointsItem.cc
  Core/Qwt/DefectPointsItem.hh
  Core/Qwt/DoubleVectorVerticalSeriesData.cc
  Core/Qwt/DoubleVectorVerticalSeriesData.hh
  Core/Qwt/MultiArrayColorSliceSeriesData.cc
  Core/Qwt/MultiArrayColorSliceSeriesData.hh
  Core/Qwt/MultiArrayColorSliceVerticalSeriesData.cc
  Core/Qwt/MultiArrayColorSliceVerticalSeriesData.hh
  Core/Qwt/NormalizedRangeRasterData.cc
  Core/Qwt/NormalizedRangeRasterData.hh
  Core/Qwt/MultiArraySliceSeriesData.cc
  Core/Qwt/MultiArraySliceSeriesData.hh
  Core/Qwt/MultiArraySliceVerticalSeriesData.cc
  Core/Qwt/MultiArraySliceVerticalSeriesData.hh
  Core/Qwt/RangeScanLineSeriesData.cc
  Core/Qwt/RangeScanLineSeriesData.hh
  Core/Qwt/RangeRasterData.cc
  Core/Qwt/RangeRasterData.hh
  Core/Qwt/SourceScanLineSeriesData.cc
  Core/Qwt/SourceScanLineSeriesData.hh
  Core/Qwt/VerticalPolinomialSeriesData.cc
  Core/Qwt/VerticalPolinomialSeriesData.hh
  
  RealTime/RTReceiverI.cc
  RealTime/RTReceiverI.h
  RealTime/RTContext.cc
  RealTime/RTContext.h
  RealTime/RTAudioCollector.cc
  RealTime/RTAudioCollector.h
  RealTime/RTScanCollector.cc
  RealTime/RTScanCollector.h
  RealTime/RTHead.cc
  RealTime/RTHead.h
  RealTime/onewavewidget.h
  RealTime/onewavewidget.cpp
  RealTime/qxtspanslider.h
  RealTime/qxtspanslider.cpp
  RealTime/chartvievforonewavewidget.h
  RealTime/chartvievforonewavewidget.cpp
  RealTime/fieldwidget.h
  RealTime/fieldwidget.cpp
  RealTime/cursor.h
  RealTime/cursor.cpp
  RealTime/area.h
  RealTime/area.cpp

  Gui/AssembleScanDialog.cc
  Gui/AssembleScanDialog.hh
  Gui/AssignColorForColorBarDialog.cc
  Gui/AssignColorForColorBarDialog.hh
  Gui/DefectsTable.cc
  Gui/DefectsTable.hh
  Gui/DefectClassificationTable.cc
  Gui/DefectClassificationTable.hh
  Gui/RangeDelegate.hh
  Gui/ExtremumDelegate.hh
  Gui/CoilManualControl.cc
  Gui/CoilManualControl.hh
  Gui/CommonActions.hh
  Gui/CommonActions.cc
  Gui/ColoredRangeSelector.cc
  Gui/ColoredRangeSelector.hh
  Gui/EditorWindow.cc
  Gui/EditorWindow.hh
  Gui/MainWindow.cc
  Gui/MainWindow.hh
  Gui/ManualControlDialog.cc
  Gui/ManualControlDialog.hh
  Gui/PeakSpectrogramAction.cc
  Gui/PeakSpectrogramAction.hh
  Gui/PhaseSpectrogram.cc
  Gui/PhaseSpectrogram.hh
  Gui/ProcessingParametersDialog.cc
  Gui/ProcessingParametersDialog.hh
  Gui/Saveable.hh
  Gui/ScanControlDialog.cc
  Gui/ScanControlDialog.hh
  Gui/ScanDisplayWindow.cc
  Gui/ScanDisplayWindow.hh
  Gui/StandardColorMap.hh
  Gui/FrequencyRose.cc
  Gui/FrequencyRose.hh
  main.cc)

SOURCE_GROUP("Generated Files" FILES
  ${CompositeScanner_Generated_UI}
  ${CompositeScanner_Generated_MOC}
  ${CompositeScanner_Generated_Resources}
  ${CompositeScanner_Generated_Data})

SOURCE_GROUP(Core REGULAR_EXPRESSION Core/.*)
SOURCE_GROUP(Core/Qwt REGULAR_EXPRESSION Core/Qwt/.*)
SOURCE_GROUP(Core/QtScript REGULAR_EXPRESSION Core/QtScript/.*)
SOURCE_GROUP(Gui REGULAR_EXPRESSION Gui/.*)
SOURCE_GROUP(RealTime REGULAR_EXPRESSION RealTime/.*)
SOURCE_GROUP("" FILES main.cc)

target_link_libraries(CompositeScanner
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${BERKELEY_DB_LIBRARIES}
  ${Boost_LIBRARIES}
  #${QWT_LIBRARIES}
  #${ZEROC_ICE_CORE_LIBRARY}
  #${ZEROC_ICE_UTIL_LIBRARY}
  ${OpenCV_LIBS}
  #opencv
  qwt 
  pugixml
  fftw3
  fftw3f
  Mfplat
  Mfreadwrite
  mfuuid
  Propsys
  wmcodecdspuuid

  Qt5::Widgets
  Qt5::Xml
  Qt5::Script
  Qt5::ScriptTools
  Qt5::Charts

  DevTalk-Interface
  UCL-PlotView
  UCL-Reflection-Binding
  UCL-Regression-Analysis
  UCL-Signal-Processing
  DevTalk-Drivers-Unitest-APL-System)

set_target_properties(CompositeScanner PROPERTIES AUTOMOC ON)