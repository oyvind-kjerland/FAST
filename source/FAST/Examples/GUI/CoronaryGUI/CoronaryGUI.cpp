#include "CoronaryGUI.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <FAST/Importers/ImageFileImporter.hpp>
#include<boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include "FAST/TestDataPath.hpp"
#include "FAST/Visualization/VolumeRenderer/VolumeRenderer.hpp"
#include "FAST/Visualization/VolumeRenderer/OpacityTransferFunction.hpp"
#include "FAST/Visualization/VolumeRenderer/ColorTransferFunction.hpp"


#include "FAST/Algorithms/CoronarySegmentation/MedianFilter.hpp"
#include "FAST/Algorithms/CoronarySegmentation/ConvertHU.hpp"
#include "FAST/Importers/RCAALineSetImporter.hpp"
#include "FAST/Algorithms/TubeSegmentationAndCenterlineExtraction/TubeSegmentationAndCenterlineExtraction.hpp"
#include "FAST/Algorithms/ImageGradient/ImageGradient.hpp"
#include "FAST/Algorithms/CoronarySegmentation/NormalizeVectorField.hpp"
#include "FAST/Algorithms/CoronarySegmentation/FrangiTDF.hpp"
#include "FAST/Algorithms/GradientVectorFlow/EulerGradientVectorFlow.hpp"
#include "FAST/Algorithms/CoronarySegmentation/Hessian.hpp"


#include "FAST/Utility.hpp"

namespace fast {

CoronaryGUI::CoronaryGUI() {

	// Create image importers and exporters for caching
	imageFileImporter = ImageFileImporter::New();
	metaImageExporter = MetaImageExporter::New();
	
}


void CoronaryGUI::createLineSetImporters(std::string datasetPath, Vector3f spacing)
{
	line0Importer = RCAALineSetImporter::New();
	line1Importer = RCAALineSetImporter::New();
	line2Importer = RCAALineSetImporter::New();
	line3Importer = RCAALineSetImporter::New();
	
	line0Importer->setSpacing(spacing);
	line1Importer->setSpacing(spacing);
	line2Importer->setSpacing(spacing);
	line3Importer->setSpacing(spacing);

	line0Importer->setFilename(Config::getTestDataPath() + datasetPath + "/vessel0/reference.txt");
	line1Importer->setFilename(Config::getTestDataPath() + datasetPath + "/vessel1/reference.txt");
	line2Importer->setFilename(Config::getTestDataPath() + datasetPath + "/vessel2/reference.txt");
	line3Importer->setFilename(Config::getTestDataPath() + datasetPath + "/vessel3/reference.txt");

	lineRenderer = LineRenderer::New();
	
	
	lineRenderer->addInputConnection(line0Importer->getOutputPort(), Color::Blue(), 5);
	lineRenderer->addInputConnection(line1Importer->getOutputPort(), Color::Blue(), 5);
	lineRenderer->addInputConnection(line2Importer->getOutputPort(), Color::Blue(), 5);
	lineRenderer->addInputConnection(line3Importer->getOutputPort(), Color::Blue(), 5);
	
	//lineRenderer->setDefaultDrawOnTop(true);

	view->addRenderer(lineRenderer);
}


void CoronaryGUI::updateSliceX(int value) {
	mSliceRendererX->setSliceToRender(value);
	// Update label
	std::string text = "Slice X: " + boost::lexical_cast<std::string>(value);
	mSliceLabelX->setText(text.c_str());
}

void CoronaryGUI::updateSliceY(int value) {
	mSliceRendererY->setSliceToRender(value);
	// Update label
	std::string text = "Slice Y: " + boost::lexical_cast<std::string>(value);
	mSliceLabelY->setText(text.c_str());
}

void CoronaryGUI::updateSliceZ(int value) {
	mSliceRendererZ->setSliceToRender(value);
	// Update label
	std::string text = "Slice Z: " + boost::lexical_cast<std::string>(value);
	mSliceLabelZ->setText(text.c_str());
}

void CoronaryGUI::setDataset(std::string dataset)
{
	currentDataset = dataset;
	folderPath = Config::getTestDataPath() + "dataset" + currentDataset + "/";
}

void CoronaryGUI::showSlices()
{
	initView();
	createGUI();
	//enableFullscreen();
}

void CoronaryGUI::showReference()
{
	// Need the input dataset to obtain spacing
	std::string filename = Config::getTestDataPath() + "dataset" + currentDataset + "/image" + currentDataset + ".mhd";
	ImageFileImporter::pointer tmpImageImporter = ImageFileImporter::New();
	tmpImageImporter->setFilename(filename);
	tmpImageImporter->update();
	Image::pointer importedImage = tmpImageImporter->getOutputData<Image>();
	spacing = importedImage->getSpacing();
	createLineSetImporters("dataset" + currentDataset, spacing);
}

void CoronaryGUI::disableCache()
{
	useCache = false;
}

void CoronaryGUI::visualizeImage(std::string filename)
{
	std::string path = Config::getTestDataPath() + "dataset" + currentDataset + "/" + filename;
	importImage(path);
	
	
	Image::pointer image = imageFileImporter->getOutputData<Image>();

	
	slicePort = imageFileImporter->getOutputPort();




}


void CoronaryGUI::initView()
{
	// Create a 3D view
	view = createView();
	view->set3DMode();


	// Calculate correct intensity window and level
	Image::pointer image = slicePort.getData();

	float max = image->calculateMaximumIntensity();
	float min = image->calculateMinimumIntensity();

	intensityWindow = max - min;
	intensityLevel = intensityWindow / 2.0f;


	// Set up slice renderers
	mSliceRendererX = SliceRenderer::New();
	mSliceRendererX->setInputConnection(slicePort);
	mSliceRendererX->setSlicePlane(PlaneType::PLANE_X);
	mSliceRendererX->setSliceToRender(0);
	mSliceRendererX->setIntensityWindow(intensityWindow);
	mSliceRendererX->setIntensityLevel(intensityLevel);

	mSliceRendererY = SliceRenderer::New();
	mSliceRendererY->setInputConnection(slicePort);
	mSliceRendererY->setSlicePlane(PlaneType::PLANE_Y);
	mSliceRendererY->setSliceToRender(0);
	mSliceRendererY->setIntensityWindow(intensityWindow);
	mSliceRendererY->setIntensityLevel(intensityLevel);

	mSliceRendererZ = SliceRenderer::New();
	mSliceRendererZ->setInputConnection(slicePort);
	mSliceRendererZ->setSlicePlane(PlaneType::PLANE_Z);
	mSliceRendererZ->setSliceToRender(0);
	mSliceRendererZ->setIntensityWindow(intensityWindow);
	mSliceRendererZ->setIntensityLevel(intensityLevel);

	view->addRenderer(mSliceRendererX);
	view->addRenderer(mSliceRendererY);
	view->addRenderer(mSliceRendererZ);

}

void CoronaryGUI::createGUI()
{
		
	// Create and add GUI elements

	// First create the menu layout
	QVBoxLayout* menuLayout = new QVBoxLayout;

	// Menu items should be aligned to the top
	menuLayout->setAlignment(Qt::AlignTop);

	// Title label
	QLabel* title = new QLabel;
	title->setText("Menu");
	QFont font;
	font.setPointSize(28);
	title->setFont(font);
	menuLayout->addWidget(title);

	// Quit button
	QPushButton* quitButton = new QPushButton;
	quitButton->setText("Quit");
	quitButton->setFixedWidth(200);
	menuLayout->addWidget(quitButton);

	// Connect the clicked signal of the quit button to the stop method for the window
	QObject::connect(quitButton, &QPushButton::clicked, boost::bind(&Window::stop, this));

	mSliceLabelX = new QLabel;
	mSliceLabelX->setText("Slice X: 0");
	menuLayout->addWidget(mSliceLabelX);

	QSlider* sliderX = new QSlider(Qt::Horizontal);
	sliderX->setMinimum(0);
	sliderX->setMaximum(sliceWidth);
	sliderX->setValue(0);
	sliderX->setFixedWidth(200);
	menuLayout->addWidget(sliderX);

	mSliceLabelY = new QLabel;
	mSliceLabelY->setText("Slice Y: 0");
	menuLayout->addWidget(mSliceLabelY);

	QSlider* sliderY = new QSlider(Qt::Horizontal);
	sliderY->setMinimum(0);
	sliderY->setMaximum(sliceHeight);
	sliderY->setValue(0);
	sliderY->setFixedWidth(200);
	menuLayout->addWidget(sliderY);

	mSliceLabelZ = new QLabel;
	mSliceLabelZ->setText("Slice Z: 0");
	menuLayout->addWidget(mSliceLabelZ);

	QSlider* sliderZ = new QSlider(Qt::Horizontal);
	sliderZ->setMinimum(0);
	sliderZ->setMaximum(sliceDepth);
	sliderZ->setValue(0);
	sliderZ->setFixedWidth(200);
	menuLayout->addWidget(sliderZ);

	// Connect the value changed signal of the slider to the updateSlice method
	QObject::connect(sliderX, &QSlider::valueChanged, boost::bind(&CoronaryGUI::updateSliceX, this, _1));
	QObject::connect(sliderY, &QSlider::valueChanged, boost::bind(&CoronaryGUI::updateSliceY, this, _1));
	QObject::connect(sliderZ, &QSlider::valueChanged, boost::bind(&CoronaryGUI::updateSliceZ, this, _1));

	// Add menu and view to main layout
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addLayout(menuLayout);
	layout->addWidget(view);

	mWidget->setLayout(layout);
}

void CoronaryGUI::importImage(std::string filename)
{
	// Import dataset
	std::cout << filename << std::endl;
	imageFileImporter->setFilename(filename);

	// Obtain width, height and depth of image
	imageFileImporter->update();
	Image::pointer importedImage = imageFileImporter->getOutputData<Image>();
	
	sliceWidth = importedImage->getWidth();
	sliceHeight = importedImage->getHeight();
	sliceDepth = importedImage->getDepth();


}

void CoronaryGUI::performMedianFilter()
{
	
	std::cout << "Perform Median Filter" << std::endl;

	std::string filename = Config::getTestDataPath() + "dataset" + currentDataset + "/image" + currentDataset + ".mhd";
	importImage(filename);

	// Convert to float
	ConvertHU::pointer convertHU = ConvertHU::New();
	convertHU->setInputConnection(imageFileImporter->getOutputPort());

	// Median filtering
	MedianFilter::pointer medianFilter = MedianFilter::New();
	medianFilter->setInputConnection(convertHU->getOutputPort());

	// Cache data
	if (useCache) {
		metaImageExporter->setFilename(Config::getTestDataPath() + "dataset" + currentDataset + "/median.mhd");
		metaImageExporter->setInputConnection(medianFilter->getOutputPort());
		metaImageExporter->update();
	}
	
	// Visualize slices
	slicePort = medianFilter->getOutputPort();
}

void CoronaryGUI::performImageGradient()
{

	std::cout << "Perform Image Gradient" << std::endl;

	// Import median
	std::string filename = Config::getTestDataPath() + "dataset" + currentDataset + "/median.mhd";
	importImage(filename);

	ImageGradient::pointer imageGradient = ImageGradient::New();
	imageGradient->setInputConnection(imageFileImporter->getOutputPort());


	// Cache data
	if (useCache) {
		std::cout << "Start caching" << std::endl;
		metaImageExporter->setFilename(Config::getTestDataPath() + "dataset" + currentDataset + "/imageGradient.mhd");
		metaImageExporter->setInputConnection(imageGradient->getOutputPort());
		metaImageExporter->update();
	}

	// Visualize slices
	slicePort = imageGradient->getOutputPort();

}

void CoronaryGUI::performGradientVectorFlow()
{

	std::cout << "Perform Gradient Vector Flow" << std::endl;

	// Import Image
	std::string filename = Config::getTestDataPath() + "dataset" + currentDataset + "/imageGradient.mhd";
	std::cout << filename << std::endl;
	importImage(filename);


	std::cout << "gvf here" << std::endl;

	// Gradient vector flow
	EulerGradientVectorFlow::pointer gradientVectorFlow = EulerGradientVectorFlow::New();
	gradientVectorFlow->setInputConnection(imageFileImporter->getOutputPort());
	gradientVectorFlow->setIterations(500);
	gradientVectorFlow->setMuConstant(5);


	// Cache data
	if (useCache) {
		std::cout << "start caching" << std::endl;
		metaImageExporter->setFilename(Config::getTestDataPath() + "dataset" + currentDataset + "/gradientVectorFlow.mhd");
		metaImageExporter->setInputConnection(gradientVectorFlow->getOutputPort());
		std::cout << "meta" << std::endl;
		metaImageExporter->update();
		std::cout << "update" << std::endl;
	}

	// Visualize
	slicePort = gradientVectorFlow->getOutputPort();
	

}

void CoronaryGUI::performHessian()
{

	// Import image gradient
	std::cout << "Perform Hessian" << std::endl;

	std::string inputFilename, outputFilename;

	inputFilename = folderPath + "imageGradient.mhd";
	importImage(inputFilename);


	std::cout << "hessian" << std::endl;

	//
	// Image Gradient
	//

	// Hessian analysis
	Hessian::pointer hessian = Hessian::New();
	hessian->setInputConnection(imageFileImporter->getOutputPort());

	// Cache data
	if (useCache) {

		// Cache eigenvalues
		std::cout << "Cache eigenvalues" << std::endl;
		outputFilename = folderPath + "eigenvaluesImageGradient.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getEigenvaluesOuptutPort());
		metaImageExporter->update();

		// Cache tangents
		std::cout << "Cache tangents" << std::endl;
		outputFilename = folderPath + "tangentsImageGradient.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getTangentsOutputPort());
		metaImageExporter->update();
	}

	// Visualize eigenvalues
	slicePort = hessian->getOutputPort(0);


	//
	// Gradient Vector Flow
	//

	inputFilename = folderPath + "gradientVectorFlow.mhd";
	importImage(inputFilename);

	if (useCache) {

		outputFilename = folderPath + "eigenvaluesGradientVectorFlow";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getEigenvaluesOuptutPort());
		metaImageExporter->update();

		outputFilename = folderPath + "tangentsGradientVectorFlow";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getTangentsOutputPort());
		metaImageExporter->update();
	}


	

}


void CoronaryGUI::performImageGradientTDF()
{
}

void CoronaryGUI::performGradientVectorFlowTDF()
{
}

void CoronaryGUI::performMaxTDF()
{
}

void CoronaryGUI::performRidgeTraversal()
{
}

void CoronaryGUI::performTreeReconstruction()
{
}

void CoronaryGUI::performOstiumDetection()
{
}

}
