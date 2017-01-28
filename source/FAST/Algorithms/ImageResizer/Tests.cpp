#include "FAST/Testing.hpp"
#include "ImageResizer.hpp"
#include "FAST/Importers/ImageFileImporter.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Visualization/ImageRenderer/ImageRenderer.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("ImageResizer 2D", "[fast][ImageResizer]") {
	ImageFileImporter::pointer importer = ImageFileImporter::New();
	importer->setFilename(Config::getTestDataPath() + "US/US-2D.jpg");
	ImageResizer::pointer resizer = ImageResizer::New();
	resizer->setInputConnection(importer->getOutputPort());
	resizer->setWidth(64);
	resizer->setHeight(64);
	resizer->update();

	Image::pointer result = resizer->getOutputData<Image>();
	CHECK(result->getWidth() == 64);
	CHECK(result->getHeight() == 64);
}
