#include "Registration.h"
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2)
//VTK_MODULE_INIT(vtkRenderingContextOpenGL)
VTK_MODULE_INIT(vtkRenderingFreeType)

vector<ImagePoint*> imagePointVector_before;
vector<ImagePoint*> imagePointVector_after;
QColor color = {155,155,155};
bool isPointsregistration;

Registration::Registration(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	renderer = vtkSmartPointer<vtkRenderer>::New();
	actor = vtkSmartPointer<vtkActor>::New();
	renderer2 = vtkSmartPointer<vtkRenderer>::New();
	actor2 = vtkSmartPointer<vtkActor>::New();
	renderer3 = vtkSmartPointer<vtkRenderer>::New();
	actor3 = vtkSmartPointer<vtkActor>::New();

	m_EventQtSlotConnect = vtkEventQtSlotConnect::New();
	m_EventQtSlotConnect_2 = vtkEventQtSlotConnect::New();
	initSlot();

	ui.qvtkWidget_before->GetRenderWindow()->AddRenderer(renderer);
	ui.qvtkWidget_after->GetRenderWindow()->AddRenderer(renderer2);
	ui.qvtkWidget_registration->GetRenderWindow()->AddRenderer(renderer3);


}

void Registration::initSlot()
{
	this->connect(ui.pushButton_STL1, SIGNAL(clicked()), this, SLOT(OnAddSTLFile_clicked()));
	this->connect(ui.pushButton_STL2, SIGNAL(clicked()), this, SLOT(OnAddSTLFile_clicked_2()));
	this->connect(ui.pushButton_registration, SIGNAL(clicked()), this, SLOT(StartRegistration()));
	this->connect(ui.modelColor_pushButton, SIGNAL(clicked()), this, SLOT(OnChangePropertyColor_clicked()));
	this->connect(ui.pushButton_savetxt, SIGNAL(clicked()), this, SLOT(OnSetSaveTXT_clicked()));
	this->connect(ui.checkBox_points, SIGNAL(clicked()), this, SLOT(OnChangeRegistration_clicked()));
	m_EventQtSlotConnect->Connect(ui.qvtkWidget_before->GetRenderWindow()->GetInteractor(), vtkCommand::RightButtonPressEvent, this, SLOT(OnRightButtonPress(vtkObject*, unsigned long)));
	m_EventQtSlotConnect_2->Connect(ui.qvtkWidget_after->GetRenderWindow()->GetInteractor(), vtkCommand::RightButtonPressEvent, this, SLOT(OnRightButtonPress_2(vtkObject*, unsigned long)));
}

void Registration::OnAddSTLFile_clicked()
{
	renderer->Clear();
	renderer->RemoveAllViewProps();

	QString filepath = QFileDialog::getOpenFileName(0, "Open a STL file", ".stl");
	if (filepath.isEmpty())
	{
		QMessageBox::warning(0, "Warning", "No File Be Opened");
		return; 
	}
	// 支持带中文路径的读取  
	QByteArray ba = filepath.toLocal8Bit();
	const char *fileName_str = ba.data();

	// 用vtkSTLReader读取STL图像  
	reader = vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName(fileName_str);
	reader->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(reader->GetOutput());

	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(1, 0, 0);
	//actor->GetProperty()->SetOpacity(0.8);

	renderer->AddActor(actor);
	renderer->ResetCamera();

	ui.qvtkWidget_before->GetRenderWindow()->AddRenderer(renderer);
	ui.qvtkWidget_before->show();
}



void Registration::OnAddSTLFile_clicked_2()
{
	renderer2->Clear();
	renderer2->RemoveAllViewProps();

	QString filepath = QFileDialog::getOpenFileName(0, "Open a STL file", ".stl");
	if (filepath.isEmpty())
	{
		QMessageBox::warning(0, "Warning", "No File Be Opened");
		return;
	}
	// 支持带中文路径的读取  
	QByteArray ba = filepath.toLocal8Bit();
	const char *fileName_str = ba.data();

	// 用vtkSTLReader读取STL图像  
	reader2 = vtkSmartPointer<vtkSTLReader>::New();
	reader2->SetFileName(fileName_str);
	reader2->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper2->SetInputData(reader2->GetOutput());
	actor2->SetMapper(mapper2);
	actor2->GetProperty()->SetColor(155/255.0, 155/255.0, 155/255.0);
	renderer2->AddActor(actor2);
	renderer2->ResetCamera();

	ui.qvtkWidget_after->GetRenderWindow()->AddRenderer(renderer2);
	ui.qvtkWidget_after->show();
}


vtkSmartPointer<vtkPoints>VectorPointsToVtkPoints(vector<ImagePoint*> vectorPoints, vtkPoints* vtkPoints)
{
	//创建一个浮点型数组存储"点"
	vtkFloatArray *pcoords = vtkFloatArray::New();
	//设置维度，点->3
	pcoords->SetNumberOfComponents(3);
	//设置数组个数
	pcoords->SetNumberOfTuples(vectorPoints.size());
	//指定每一个数组，具体的点坐标
	float temp[1][3] = { 0 };
	for (int i = 0; i < vectorPoints.size(); i++)
	{
		temp[0][0] = vectorPoints[i]->x;
		temp[0][1] = vectorPoints[i]->y;
		temp[0][2] = vectorPoints[i]->z;
		//设置数组中的点
		pcoords->SetTuple(i, temp[0]);
	}
	//获得四个点
	vtkPoints->SetData(pcoords);
	return vtkPoints;
}

vtkSmartPointer<vtkIterativeClosestPointTransform> icptransProcess(vtkSmartPointer<vtkPolyData> trans, vtkSmartPointer<vtkPolyData> stable)
{
	//源数据 与 目标数据
	vtkSmartPointer<vtkPolyData> transSource =
		vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> stableSource =
		vtkSmartPointer<vtkPolyData>::New();

	if (isPointsregistration)
	{
		////////////////选点配准/////////////////////////
	vtkSmartPointer<vtkPoints> vtkpoints_before =
		vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkPoints> vtkpoints_after =
		vtkSmartPointer<vtkPoints>::New();

	vtkpoints_before = VectorPointsToVtkPoints(imagePointVector_before , vtkpoints_before);
	vtkpoints_after = VectorPointsToVtkPoints(imagePointVector_after, vtkpoints_after);
	
	puts("before");
	for (size_t j = 0; j < vtkpoints_before->GetNumberOfPoints(); j++)
	{
		std::cout << vtkpoints_before->GetPoint(j)[0] << "\t" <<
			vtkpoints_before->GetPoint(j)[1] << "\t" <<
			vtkpoints_before->GetPoint(j)[2] << std::endl;
	}
	puts("after");
	for (size_t j = 0; j < vtkpoints_after->GetNumberOfPoints(); j++)
	{
		std::cout << vtkpoints_after->GetPoint(j)[0] << "\t" <<
			vtkpoints_after->GetPoint(j)[1] << "\t" <<
			vtkpoints_after->GetPoint(j)[2] << std::endl;
	}

	transSource->SetPoints(vtkpoints_after);
	stableSource->SetPoints(vtkpoints_before);

	}
	else
	{
		////////////////全局配准/////////////////////////
		transSource->SetPoints(trans->GetPoints());
		stableSource->SetPoints(stable->GetPoints());
	}

	


	vtkSmartPointer<vtkVertexGlyphFilter>  transGlyph =
		vtkSmartPointer<vtkVertexGlyphFilter>::New();
	transGlyph->SetInputData(transSource);
	transGlyph->Update();

	vtkSmartPointer<vtkVertexGlyphFilter>  stableGlyph =
		vtkSmartPointer<vtkVertexGlyphFilter>::New();
	stableGlyph->SetInputData(stableSource);
	stableGlyph->Update();


	//进行ICP配准求变换矩阵
	vtkSmartPointer<vtkIterativeClosestPointTransform> icptrans =
		vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
	icptrans->SetSource(transGlyph->GetOutput());//转换的
	icptrans->SetTarget(stableGlyph->GetOutput());//固定的
	//icptrans->SetSource(stableGlyph->GetOutput());
	//icptrans->SetTarget(transGlyph->GetOutput());
	icptrans->GetLandmarkTransform()->SetModeToRigidBody();
	icptrans->SetMaximumNumberOfIterations(50);
	icptrans->StartByMatchingCentroidsOn();
	icptrans->Modified();
	icptrans->Update();

	return icptrans;
}

void Registration::StartRegistration()
{
	//renderer3->Clear();
	//renderer3->RemoveAllViewProps();

	//获得STL的模型1
	vtkSmartPointer<vtkSTLReader> stableReader = vtkSmartPointer<vtkSTLReader>::New();
	stableReader = reader;
	stableReader->Update();
	vtkSmartPointer<vtkPolyData> stableSTL = stableReader->GetOutput();

	//获得STL的模型2
	vtkSmartPointer<vtkSTLReader> transReader1 = vtkSmartPointer<vtkSTLReader>::New();
	transReader1 = reader2;
	transReader1->Update();
	vtkSmartPointer<vtkPolyData> transSTL1 = transReader1->GetOutput();


	//分别于第一个orig进行ICP配准
	icptrans1 = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
	icptrans1 = icptransProcess(transSTL1, stableSTL);


	//变换stl模型1
	vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter1 =
		vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	transformFilter1->SetInputConnection(transReader1->GetOutputPort());
	transformFilter1->SetTransform(icptrans1);
	transformFilter1->Update();


	vtkSmartPointer<vtkMatrix4x4> m = icptrans1->GetMatrix();
	std::cout << "The resulting matrix is: " << *m << std::endl;



	vtkSmartPointer<vtkPolyDataMapper> stableMapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	//targetMapper->SetInputConnection(targetGlyph->GetOutputPort());
	stableMapper->SetInputConnection(stableReader->GetOutputPort());
	vtkSmartPointer<vtkActor> stableActor =
		vtkSmartPointer<vtkActor>::New();
	stableActor->SetMapper(stableMapper);
	//stableActor->GetProperty()->SetOpacity(0.8);
	stableActor->GetProperty()->SetColor(1, 0, 0);
	stableActor->GetProperty()->SetPointSize(3);
	//targetActor->GetProperty()->SetRepresentationToSurface();

	vtkSmartPointer<vtkPolyDataMapper> transMapper1 =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	//sourceMapper->SetInputConnection(sourceGlyph->GetOutputPort());
	transMapper1->SetInputConnection(transformFilter1->GetOutputPort());
	vtkSmartPointer<vtkActor> transActor =
		vtkSmartPointer<vtkActor>::New();
	transActor->SetMapper(transMapper1);
	//transActor->GetProperty()->SetOpacity(0.8);
	//transActor->GetProperty()->SetColor(0, 1, 0);
	transActor->GetProperty()->SetColor(double(color.red()) / 255, double(color.green()) / 255, double(color.blue()) / 255);
	transActor->GetProperty()->SetPointSize(2);

	//sourceActor->GetProperty()->SetRepresentationToWireframe();

	renderer3->AddActor(stableActor);
	renderer3->AddActor(transActor);

	//显示小球
	addrenderer3(imagePointVector_before);
	addrenderer3(imagePointVector_after);

	renderer3->ResetCamera();

	ui.qvtkWidget_registration->GetRenderWindow()->AddRenderer(renderer3);
	ui.qvtkWidget_registration->show();
	ui.qvtkWidget_registration->update();
	//加入清空imagePointVector_after？
	imagePointVector_after.swap(vector<ImagePoint*>());
}

void Registration::OnRightButtonPress(vtkObject* caller, unsigned long vtk_event)
{
	// get interactor
	vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::SafeDownCast(caller);

	//转换为世界坐标
	auto picker = vtkCellPicker::New();
	interactor->SetPicker(picker);
	picker->Pick(interactor->GetEventPosition()[0],
		interactor->GetEventPosition()[1],
		0,
		interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
	

	double picked[3];
	picker->GetPickPosition(picked);
	//picker->GetMapperPosition(picked);

	addPointImage_before(picked[0], picked[1], picked[2]);
}

void Registration::OnRightButtonPress_2(vtkObject* caller, unsigned long vtk_event)
{
	// get interactor
	vtkRenderWindowInteractor* interactor = vtkRenderWindowInteractor::SafeDownCast(caller);

	//转换为世界坐标
	auto picker = vtkCellPicker::New();
	interactor->SetPicker(picker);
	picker->Pick(interactor->GetEventPosition()[0],
		interactor->GetEventPosition()[1],
		0,
		interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

	double picked[3];
	picker->GetPickPosition(picked);
	//picker->GetMapperPosition(picked);

	addPointImage_after(picked[0], picked[1], picked[2]);
}

void Registration::addPointImage_before(double x, double y, double z)
{
	ImagePoint *point = new ImagePoint;
	point->x = x;
	point->y = y;
	point->z = z;
	point->sphere = vtkSmartPointer<vtkSphereSource>::New();
	point->sphere->SetCenter(x, y, z);
	point->sphere->SetRadius(1);

	point->mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	point->mapper->SetInputConnection(point->sphere->GetOutputPort());

	point->actor = vtkSmartPointer<vtkActor>::New();
	point->actor->SetMapper(point->mapper);
	point->actor->GetProperty()->SetColor(1, 0, 0);

	renderer->AddActor(point->actor);
	ui.qvtkWidget_before->GetRenderWindow()->Render();
	//renderer->ResetCamera();

	imagePointVector_before.push_back(point);
	//addPointTable(point);
	//pointCount++;
}

void Registration::addPointImage_after(double x, double y, double z)
{
	ImagePoint *point = new ImagePoint;
	point->x = x;
	point->y = y;
	point->z = z;
	point->sphere = vtkSmartPointer<vtkSphereSource>::New();
	point->sphere->SetCenter(x, y, z);
	point->sphere->SetRadius(1);

	point->mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	point->mapper->SetInputConnection(point->sphere->GetOutputPort());

	point->actor = vtkSmartPointer<vtkActor>::New();
	point->actor->SetMapper(point->mapper);
	point->actor->GetProperty()->SetColor(1, 0, 0);

	renderer2->AddActor(point->actor);
	ui.qvtkWidget_after->GetRenderWindow()->Render();
	//renderer2->ResetCamera();

	imagePointVector_after.push_back(point);
	//addPointTable(point);
	//pointCount++;
}

void Registration::addrenderer3(vector<ImagePoint*> vectorPoints)
{
	if (vectorPoints == imagePointVector_before) {
		for (int i = 0; i < vectorPoints.size(); i++)
		{
			vectorPoints[i]->sphere = vtkSmartPointer<vtkSphereSource>::New();
			vectorPoints[i]->sphere->SetCenter(vectorPoints[i]->x, vectorPoints[i]->y, vectorPoints[i]->z);
			vectorPoints[i]->sphere->SetRadius(1);

			vectorPoints[i]->mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			vectorPoints[i]->mapper->SetInputConnection(vectorPoints[i]->sphere->GetOutputPort());

			vectorPoints[i]->actor = vtkSmartPointer<vtkActor>::New();
			vectorPoints[i]->actor->SetMapper(vectorPoints[i]->mapper);
			vectorPoints[i]->actor->GetProperty()->SetColor(1, 0, 0);

			renderer3->AddActor(vectorPoints[i]->actor);
		}
	}
	else {
		for (int i = 0; i < vectorPoints.size(); i++)
		{
			vectorPoints[i]->sphere = vtkSmartPointer<vtkSphereSource>::New();
			vectorPoints[i]->sphere->SetCenter(vectorPoints[i]->x, vectorPoints[i]->y, vectorPoints[i]->z);
			vectorPoints[i]->sphere->SetRadius(1);

			vectorPoints[i]->mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			vectorPoints[i]->mapper->SetInputConnection(vectorPoints[i]->sphere->GetOutputPort());

			vectorPoints[i]->actor = vtkSmartPointer<vtkActor>::New();
			vectorPoints[i]->actor->SetMapper(vectorPoints[i]->mapper);

			//vectorPoints[i]->actor->GetProperty()->SetColor(1, 0, 0);

			
			vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
			transformFilter->SetInputConnection(vectorPoints[i]->sphere->GetOutputPort());
			transformFilter->SetTransform(icptrans1);
			transformFilter->Update();

			vtkSmartPointer<vtkPolyDataMapper> transformedMapper =
				vtkSmartPointer<vtkPolyDataMapper>::New();
			transformedMapper->SetInputConnection(transformFilter->GetOutputPort());
			vtkSmartPointer<vtkActor> transformedActor =
				vtkSmartPointer<vtkActor>::New();
			transformedActor->SetMapper(transformedMapper);  
			transformedActor->GetProperty()->SetColor(double(color.red()) / 255, double(color.green()) / 255, double(color.blue()) / 255);

			renderer3->AddActor(transformedActor);
		}

	}
	
}

void Registration::OnSetSaveTXT_clicked() {
	fileName = QFileDialog::getSaveFileName(this, tr("Select Save Path"), "", tr("TXT(*.txt)")); //选择路径
	//循环读取数据
	double order[100] = { 0 };
	double x[100] = { 0 };
	double y[100] = { 0 };
	double z[100] = { 0 };
	for (int i = 0; i < imagePointVector_after.size(); i++)
	{
		order[i] = i+1;
		x[i] = imagePointVector_after[i]->x;
		y[i] = imagePointVector_after[i]->y;
		z[i] = imagePointVector_after[i]->z;
	}
	writeDataToTXT(order, x, y, z, fileName, imagePointVector_after.size());
	

}

void Registration::writeDataToTXT(double* order, double* x, double* y, double* z, QString filename, int num)
{
	QFile file(filename);
	QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
	QString StrTime = time.toString("yyyy-MM-dd hh:mm:ss"); //设置显示格式
	file.open(QIODevice::WriteOnly);
	file.close();
	if (file.open(QIODevice::ReadWrite | QIODevice::Text))
	{
		QTextStream stream(&file);
		stream.seek(file.size());
		stream << QStringLiteral("测试时间:") << StrTime << "\n";
		stream << QStringLiteral("序号") << "                " << QStringLiteral("x") << "                     " << QStringLiteral("y") << "                       " << QStringLiteral("z") << "\n";
		for (int i = 0; i < num; i++)
		{
			stream << order[i] << "                " << x[i] << "                " << y[i] << "                " << z[i] << "\n";
		}
		file.close();
	}
	//ui.textEdit->append(QStringLiteral("TXT文件保存完成！"));
}

void Registration::OnChangePropertyColor_clicked()
{
	color = QColorDialog::getColor(Qt::white, this);
	actor2->GetProperty()->SetColor(double(color.red()) / 255, double(color.green()) / 255, double(color.blue()) / 255);
	ui.qvtkWidget_after->update();
}

void Registration::OnChangeRegistration_clicked()
{
	if(ui.checkBox_points->checkState()==Qt::Checked)
	{
		isPointsregistration = true;
	}
	else
	{
		isPointsregistration = false;
	}
	std::cout << isPointsregistration << std::endl;
}
