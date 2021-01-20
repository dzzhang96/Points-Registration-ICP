#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <QtWidgets/QMainWindow>
#include <QObject>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <QVTKWidget.h>
#include <vtkSTLReader.h>
#include <vtkSphereSource.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkEventQtSlotConnect.h>
#include <QCheckBox.h>
#include <vtkFloatArray.h>
#include <vtkPolyLine.h>
#include <vtkIncrementalOctreePointLocator.h>
#include <vtkDijkstraGraphGeodesicPath.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h> //坐标系交互
#include <vtkExtractSurface.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkNamedColors.h>
#include <vtkSTLWriter.h>
#include "vtkAutoInit.h"
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include "ui_Registration.h"
using namespace std;



struct ImagePoint
{
	double x;
	double y;
	double z;
	vtkSmartPointer<vtkSphereSource> sphere;
	vtkSmartPointer<vtkPolyDataMapper> mapper;
	vtkSmartPointer<vtkActor>actor;
};


class MyPointPickerCommand;
class vtkInteractorStyleTrackballCamera;

class Registration : public QMainWindow
{
	Q_OBJECT

public:
	Registration(QWidget *parent = Q_NULLPTR);
	vtkSmartPointer<vtkSTLReader> reader;
	vtkSmartPointer<vtkRenderer> renderer;
	vtkSmartPointer<vtkActor> actor1;
	vtkSmartPointer<vtkSTLReader> reader2;
	vtkSmartPointer<vtkRenderer> renderer2;
	vtkSmartPointer<vtkActor> actor2;
	vtkSmartPointer<vtkRenderer> renderer3;
	vtkSmartPointer<vtkActor> actor3;
	vtkSmartPointer<vtkSTLWriter>stlwriter;

	void initSlot();
	vtkSmartPointer<vtkEventQtSlotConnect> m_EventQtConnector;

	void writeDataToTXT(double* order, double* x, double* y, double* z, QString filename, int num);
	QString fileName;
	QColor color_stl1;
	QColor color_stl2;


public slots:
	void OnAddSTL1File_clicked();
	void OnAddSTL2File_clicked();
	void StartRegistration();
	void OnRightButtonPress(vtkObject* caller, unsigned long vtk_event);
	void OnRightButtonPress_2(vtkObject* caller, unsigned long vtk_event);
	void OnSetSaveTXT_clicked();
	void OnSetSaveSTL_clicked();
	void OnChangePropertyColor_model1_clicked();
	void OnChangePropertyColor_model2_clicked();
	void OnChangeRegistration_clicked();

private:
	Ui::RegistrationClass ui;
	vtkSmartPointer<vtkRenderer> m_pRenderder;
	vtkEventQtSlotConnect* m_EventQtSlotConnect;
	vtkEventQtSlotConnect* m_EventQtSlotConnect_2;
	void addPointImage_before(double x, double y, double z);//在图像上显示点
	void addPointImage_after(double x, double y, double z);//在图像上显示点
	void addrenderer3(vector<ImagePoint*> vectorPoints);

	vtkSmartPointer<vtkIterativeClosestPointTransform> icptrans1;
};
