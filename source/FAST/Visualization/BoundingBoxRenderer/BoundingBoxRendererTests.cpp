#include "FAST/Testing.hpp"
#include "BoundingBoxRenderer.hpp"
#include "FAST/Importers/VTKMeshFileImporter.hpp"
#include "FAST/Visualization/SimpleWindow.hpp"

using namespace fast;

TEST_CASE("BoundingBox renderer", "[BoundingBoxRenderer][fast][visual]") {
    CHECK_NOTHROW(
        VTKMeshFileImporter::pointer importer = VTKMeshFileImporter::New();
        importer->setFilename(Config::getTestDataPath() + "Surface_LV.vtk");
        BoundingBoxRenderer::pointer renderer = BoundingBoxRenderer::New();
        renderer->addInputConnection(importer->getOutputPort());
        SimpleWindow::pointer window = SimpleWindow::New();
        window->addRenderer(renderer);
        window->setTimeout(1000);
        window->start();
    );
}
