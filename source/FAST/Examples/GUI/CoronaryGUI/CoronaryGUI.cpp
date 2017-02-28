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
#include "FAST/Algorithms/CoronarySegmentation/MaxTDF.hpp"
#include "FAST/Algorithms/CoronarySegmentation/RidgeCandidateSelection.hpp"
#include "FAST/Algorithms/CoronarySegmentation/RidgeTraversal.hpp"

#include "FAST/Algorithms/BinaryThresholding/BinaryThresholding.hpp"

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


	std::cout << "max intensity: " << max << ", min intenisty: " << min << std::endl;
	intensityWindow = max - min;
	if (intensityWindow == 0) {
		intensityWindow = 0.1f;
	}
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

	// Normalize Vector Field
	NormalizeVectorField::pointer normalize = NormalizeVectorField::New();
	normalize->setInputConnection(imageGradient->getOutputPort());
	normalize->setUseMaxLength(true);
	normalize->setMaxLength(100);

	// Cache data
	if (useCache) {
		std::cout << "Start caching" << std::endl;
		metaImageExporter->setFilename(Config::getTestDataPath() + "dataset" + currentDataset + "/imageGradient.mhd");
		metaImageExporter->setInputConnection(normalize->getOutputPort());
		metaImageExporter->update();
	}

	// Visualize slices
	slicePort = normalize->getOutputPort();

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


	// Normalize vector field
	NormalizeVectorField::pointer normalize = NormalizeVectorField::New();
	normalize->setInputConnection(gradientVectorFlow->getOutputPort());
	normalize->setUseMaxLength(false);

	// Cache data
	if (useCache) {
		std::cout << "start caching" << std::endl;
		metaImageExporter->setFilename(Config::getTestDataPath() + "dataset" + currentDataset + "/gradientVectorFlow.mhd");
		metaImageExporter->setInputConnection(normalize->getOutputPort());
		metaImageExporter->update();
	}

	// Visualize
	slicePort = normalize->getOutputPort();
	

}

void CoronaryGUI::performImageGradientHessian()
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
		//outputFilename = folderPath + "eigenvaluesImageGradient.mhd";
		//metaImageExporter->setFilename(outputFilename);
		//metaImageExporter->setInputConnection(hessian->getEigenvaluesOuptutPort());
		//metaImageExporter->update();

		outputFilename = folderPath + "eigenvaluesImageGradient.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getEigenvaluesOuptutPort());
		metaImageExporter->update();

		std::cout << "saving" << std::endl;
		// Cache tangents
		outputFilename = folderPath + "tangentsImageGradient.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getTangentsOutputPort());
		metaImageExporter->update();
	}

	// Visualize eigenvalues
	slicePort = hessian->getEigenvaluesOuptutPort();
}

void CoronaryGUI::performGradientVectorFlowHessian()
{

	// Import image gradient
	std::cout << "Perform Hessian" << std::endl;

	std::string inputFilename, outputFilename;

	inputFilename = folderPath + "gradientVectorFlow.mhd";
	importImage(inputFilename);


	// Hessian analysis
	Hessian::pointer hessian = Hessian::New();
	hessian->setInputConnection(imageFileImporter->getOutputPort());

	if (useCache) {

		outputFilename = folderPath + "eigenvaluesGradientVectorFlow.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getEigenvaluesOuptutPort());
		metaImageExporter->update();


		outputFilename = folderPath + "tangentsGradientVectorFlow.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(hessian->getTangentsOutputPort());
		metaImageExporter->update();
	}

	// Visualize eigenvalues
	slicePort = hessian->getEigenvaluesOuptutPort();
}

void CoronaryGUI::performImageGradientTDF()
{

	std::cout << "Perform Image Gradient TDF" << std::endl;

	std::string inputFilename, outputFilename;

	inputFilename = folderPath + "eigenvaluesImageGradient.mhd";
	outputFilename = folderPath + "imageGradientTDF.mhd";

	// Import image
	importImage(inputFilename);

	// Create TDF
	FrangiTDF::pointer tdf = FrangiTDF::New();
	tdf->setInputConnection(imageFileImporter->getOutputPort());
	tdf->setTubeConstants(0.5f, 0.5f, 100.0f);
	tdf->update();
	slicePort = tdf->getOutputPort();
	std::cout << "Done Perform Image Gradient TDF" << std::endl;

	if (useCache) {
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(tdf->getOutputPort());
		metaImageExporter->update();
	}
}

void CoronaryGUI::performGradientVectorFlowTDF()
{

	std::cout << "Perform Gradient Vector Flow TDF" << std::endl;

	std::string inputFilename, outputFilename;

	inputFilename = folderPath + "eigenvaluesGradientVectorFlow.mhd";
	outputFilename = folderPath + "gradientVectorFlowTDF.mhd";

	// Import image
	importImage(inputFilename);

	// Create TDF
	FrangiTDF::pointer tdf = FrangiTDF::New();
	tdf->setInputConnection(imageFileImporter->getOutputPort());
	tdf->setTubeConstants(0.5f, 0.5f, 100.0f);
	tdf->update();
	if (useCache) {
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(tdf->getOutputPort());
		metaImageExporter->update();
	}

	slicePort = tdf->getOutputPort();

}

void CoronaryGUI::performMaxTDF()
{
	std::cout << "Perform Max TDF" << std::endl;

	std::string inputFilename, outputFilename;

	// Import imageGradientTDF
	ImageFileImporter::pointer imageGradientTDF = ImageFileImporter::New();
	imageGradientTDF->setFilename(folderPath + "imageGradientTDF.mhd");

	// Import gradientVectorFlowTDF
	ImageFileImporter::pointer gradientVectorFlowTDF = ImageFileImporter::New();
	gradientVectorFlowTDF->setFilename(folderPath +  "gradientVectorFlowTDF.mhd");

	// Import tangents
	ImageFileImporter::pointer imageGradientTangents = ImageFileImporter::New();
	imageGradientTangents->setFilename(folderPath + "tangentsImageGradient.mhd");

	ImageFileImporter::pointer gradientVectorFlowTangents = ImageFileImporter::New();
	gradientVectorFlowTangents->setFilename(folderPath + "tangentsGradientVectorFlow.mhd");

	MaxTDF::pointer maxTDF = MaxTDF::New();
	maxTDF->setTDFInputConnection(0, imageGradientTDF->getOutputPort());
	maxTDF->setTDFInputConnection(1, gradientVectorFlowTDF->getOutputPort());
	maxTDF->setTangentsInputConnection(0, imageGradientTangents->getOutputPort());
	maxTDF->setTangentsInputConnection(1, gradientVectorFlowTangents->getOutputPort());

	slicePort = maxTDF->getOutputPort();

	if (useCache) {
		outputFilename = folderPath + "maxTDF.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(maxTDF->getOutputPort(0));
		try {
			metaImageExporter->update();
		} catch (cl::Error& e) {
			std::cout << "here" << std::endl;
			std::cout << e.err() << std::endl;
		}


		// Save tangents
		outputFilename = folderPath + "maxTangents.mhd";
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(maxTDF->getOutputPort(1));
		metaImageExporter->update();
	}

}

void CoronaryGUI::performLungTissueRemoval() {
	// Import median
	std::cout << "Perform Lung tissue removal" << std::endl;

	std::string medianFilename = folderPath + "median.mhd";
	std::string inputFilename = folderPath + "maxTDF.mhd";

	std::string outputFilename = folderPath + "lungTDF.mhd";

	ImageFileImporter::pointer median = ImageFileImporter::New();
	median->setFilename(medianFilename);
	median->update();

	ImageFileImporter::pointer input =  ImageFileImporter::New();
	input->setFilename(inputFilename);


	// Perform Binary thresholding
	BinaryThresholding::pointer binaryThresholding = BinaryThresholding::New();
	binaryThresholding->setInputConnection(median->getOutputPort());
	binaryThresholding->setLowerThreshold(324);

	slicePort = binaryThresholding->getOutputPort();



}

void CoronaryGUI::performRidgeCandidateSelection() {

	std::cout << "Perform Ridge Candidate Selection" << std::endl;

	std::string inputFilename = folderPath + "maxTDF.mhd";
	std::string outputFilename = folderPath + "ridgeCandidates.mhd";

	importImage(inputFilename);

	std::cout << "Before" << std::endl;
	RidgeCandidateSelection::pointer ridgeCandidateSelection = RidgeCandidateSelection::New();
	std::cout << "after" << std::endl;
	ridgeCandidateSelection->setTHigh(0.5f);
	ridgeCandidateSelection->setTLow(0.1f);
	ridgeCandidateSelection->setInputConnection(imageFileImporter->getOutputPort());
	std::cout << "Before update" << std::endl;
	ridgeCandidateSelection->update();
	std::cout << "After update" << std::endl;
	slicePort = ridgeCandidateSelection->getCandidatesOuptutPort();

	if (useCache) {
		metaImageExporter->setFilename(outputFilename);
		metaImageExporter->setInputConnection(ridgeCandidateSelection->getCandidatesOuptutPort());
		metaImageExporter->update();
	}
}

void CoronaryGUI::performRidgeTraversal()
{
	std::cout << "Perform Ridge Traversal" << std::endl;

	std::string tdfFilename = folderPath + "maxTDF.mhd";
	std::string tangentsFilename = folderPath + "maxTangents.mhd";
	std::string ridgeCandidatesFilename = folderPath + "ridgeCandidates.mhd";


	// Create image file importers
	ImageFileImporter::pointer tdf = ImageFileImporter::New();
	ImageFileImporter::pointer tangents = ImageFileImporter::New();
	ImageFileImporter::pointer ridgeCandidates = ImageFileImporter::New();

	tdf->setFilename(tdfFilename);
	tangents->setFilename(tangentsFilename);
	ridgeCandidates->setFilename(ridgeCandidatesFilename);


	// Create RidgeTraversal
	RidgeTraversal::pointer ridgeTraversal = RidgeTraversal::New();
	ridgeTraversal->setTDFInputConnection(tdf->getOutputPort());
	ridgeTraversal->setTangentsInputConnection(tangents->getOutputPort());
	ridgeTraversal->setRidgeCandidatesInputConnection(ridgeCandidates->getOutputPort());
	ridgeTraversal->setTLow(0.1f);
	ridgeTraversal->setLMin(10);

	ridgeTraversal->update();
	slicePort = ridgeTraversal->getNeighborsOutputPort();


}

void CoronaryGUI::performTreeReconstruction()
{
}

void CoronaryGUI::performOstiumDetection()
{
}

}
