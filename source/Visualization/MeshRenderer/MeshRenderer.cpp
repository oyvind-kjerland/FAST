#include <GL/glew.h>
#include "Image.hpp"
#include "HelperFunctions.hpp"
#include "DeviceManager.hpp"
#include "View.hpp"
#include "Utility.hpp"
#include <QCursor>

#include "MeshRenderer.hpp"
#include "SceneGraph.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace fast {


void MeshRenderer::setInput(MeshData::pointer mesh) {
    releaseInputAfterExecute(getNrOfInputData(), false);
    setInputData(getNrOfInputData(), mesh);
}

void MeshRenderer::addInput(MeshData::pointer mesh) {
    releaseInputAfterExecute(getNrOfInputData(), false);
    setInputData(getNrOfInputData(), mesh);
}

void MeshRenderer::addInput(MeshData::pointer mesh, Color color, float opacity) {
    addInput(mesh);
    mInputColors[mesh] = color;
    mInputOpacities[mesh] = opacity;
}


MeshRenderer::MeshRenderer() : Renderer() {
    mDevice = DeviceManager::getInstance().getDefaultVisualizationDevice();
    mDefaultOpacity = 1;
    mDefaultColor = Color::Green();
}

void MeshRenderer::execute() {
    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        MeshData::pointer input = getInputData(inputNr);
        if(input->isDynamicData()) {
            mMeshToRender[inputNr] = DynamicMesh::pointer(input)->getNextFrame();
        } else {
            mMeshToRender[inputNr] = input;
        }
    }
}

void MeshRenderer::draw() {

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);

    for(uint inputNr = 0; inputNr < getNrOfInputData(); inputNr++) {
        MeshData::pointer input = getInputData(inputNr);

        Mesh::pointer surfaceToRender = mMeshToRender[inputNr];
        // Draw the triangles in the VBO
        SceneGraph& graph = SceneGraph::getInstance();
        SceneGraphNode::pointer node = graph.getDataNode(surfaceToRender);
        LinearTransformation transform = graph.getLinearTransformationFromNode(node);

        glPushMatrix();
        glMultMatrixf(transform.getTransform().data());

        float opacity = mDefaultOpacity;
        Color color = mDefaultColor;
        if(mInputOpacities.count(input) > 0) {
            opacity = mInputOpacities[input];
        }
        if(mInputColors.count(input) > 0) {
            color = mInputColors[input];
        }

        // Set material properties
        if(opacity < 1) {
            // Enable transparency
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        GLfloat GLcolor[] = { color.getRedValue(), color.getGreenValue(), color.getBlueValue(), opacity };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, GLcolor);
        GLfloat specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specReflection);
        GLfloat shininess[] = { 16.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

        releaseOpenGLContext();
        VertexBufferObjectAccess access = surfaceToRender->getVertexBufferObjectAccess(ACCESS_READ, mDevice);
        setOpenGLContext(mDevice->getGLContext());
        GLuint* VBO_ID = access.get();

        // Normal Buffer
        glBindBuffer(GL_ARRAY_BUFFER, *VBO_ID);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, 24, BUFFER_OFFSET(0));
        glNormalPointer(GL_FLOAT, 24, BUFFER_OFFSET(12));

        glDrawArrays(GL_TRIANGLES, 0, surfaceToRender->getNrOfTriangles()*3);

        // Release buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        if(opacity < 1) {
            // Disable transparency
            glDisable(GL_BLEND);
        }
        glPopMatrix();
    }

    glDisable(GL_LIGHTING);
    glDisable(GL_NORMALIZE);
}

BoundingBox MeshRenderer::getBoundingBox() {
    std::vector<Vector3f> coordinates;
    for(uint i = 0; i < getNrOfInputData(); i++) {
        BoundingBox transformedBoundingBox = mMeshToRender[i]->getTransformedBoundingBox();
        MatrixXf corners = transformedBoundingBox.getCorners();
        for(uint j = 0; j < 8; j++) {
            coordinates.push_back((Vector3f)corners.row(j));
        }
    }
    return BoundingBox(coordinates);
}

void MeshRenderer::setDefaultColor(Color color) {
    mDefaultColor = color;
}

void MeshRenderer::setColor(MeshData::pointer mesh, Color color) {
    mInputColors[mesh] = color;
}

void MeshRenderer::setOpacity(MeshData::pointer mesh, float opacity) {
    mInputOpacities[mesh] = opacity;
}

void MeshRenderer::setDefaultOpacity(float opacity) {
    mDefaultOpacity = opacity;
    if(mDefaultOpacity > 1) {
        mDefaultOpacity = 1;
    } else if(mDefaultOpacity < 0) {
        mDefaultOpacity = 0;
    }
}

} // namespace fast