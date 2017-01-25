/**
* Examples/GUI/SimpleGUI/SimpleGUI.hpp
*
* If you edit this example, please also update the wiki and source code file in the repository.
*/
#ifndef SIMPLE_GUI_HPP_
#define SIMPLE_GUI_HPP_

#include "FAST/Visualization/Window.hpp"
#include "FAST/Visualization/SliceRenderer/SliceRenderer.hpp"
#include "FAST/Visualization/LineRenderer/LineRenderer.hpp"
#include "FAST/Importers/RCAALineSetImporter.hpp"
#include <QLabel>
#include <FAST/Importers/ImageFileImporter.hpp>
#include "FAST/Exporters/MetaImageExporter.hpp"

namespace fast {

	class CoronaryGUI : public Window {
		FAST_OBJECT(CoronaryGUI)
	public:
		void updateSliceX(int value);
		void updateSliceY(int value);
		void updateSliceZ(int value);

		void setDataset(std::string dataset);

		void showSlices();
		void showReference();
		void disableCache();

		//Visuaslize image
		void visualizeImage(std::string filename);

		// Separate steps are cached
		void performMedianFilter();
		void performImageGradient();
		void performGradientVectorFlow();
		void performHessian();
		void performImageGradientTDF();
		void performGradientVectorFlowTDF();
		void performMaxTDF();
		void performRidgeTraversal();
		void performTreeReconstruction();
		void performOstiumDetection();


	private:
		CoronaryGUI();
		
		void createLineSetImporters(std::string datasetPath, Vector3f spacing);

		RCAALineSetImporter::pointer line0Importer, line1Importer, line2Importer, line3Importer;

		
		// View
		View* view;


		// GUI
		QLabel* mSliceLabelX;
		QLabel* mSliceLabelY;
		QLabel* mSliceLabelZ;

		ProcessObjectPort slicePort;


		// Current dataset path
		std::string currentDataset;
		std::string folderPath;

		// Perform caching
		bool useCache = true;

		// Visualization
		LineRenderer::pointer lineRenderer;
		SliceRenderer::pointer mSliceRendererX, mSliceRendererY, mSliceRendererZ;
		// todo these should be obtained by the image
		int sliceWidth = 512;
		int sliceHeight = 512;
		int sliceDepth = 278;
		Vector3f spacing;

		float intensityWindow = -1;
		float intensityLevel =  -1;


		// Process elements
		ImageFileImporter::pointer imageFileImporter;
		MetaImageExporter::pointer metaImageExporter;

		void initView();
		void createGUI();

		void importImage(std::string filename);



	};

} // end namespace fast

#endif
