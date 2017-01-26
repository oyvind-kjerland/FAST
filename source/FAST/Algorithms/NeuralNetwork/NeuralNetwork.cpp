#include "NeuralNetwork.hpp"
#include "FAST/Data/Image.hpp"
#include "FAST/Algorithms/ImageResizer/ImageResizer.hpp"

#include <tensorflow/core/framework/step_stats.pb.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/framework/types.pb.h>
#include <tensorflow/core/lib/strings/stringprintf.h>
#include <tensorflow/core/platform/env.h>
#include <tensorflow/core/platform/logging.h>
#include <tensorflow/core/platform/mutex.h>
#include <tensorflow/core/platform/types.h>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/graph/default_device.h>

namespace fast {

// See here for reference: https://github.com/tensorflow/tensorflow/blob/86f5ab7474825da756838b34e1b4eac93f5fc68a/tensorflow/contrib/android/jni/tensorflow_inference_jni.cc

void NeuralNetwork::load(std::string networkFilename) {

	tensorflow::SessionOptions options;
	tensorflow::ConfigProto &config = options.config;
	mSession.reset(tensorflow::NewSession(options));
	tensorflow::GraphDef tensorflow_graph;

	{
		tensorflow::Status s = ReadBinaryProto(tensorflow::Env::Default(), networkFilename, &tensorflow_graph);
		if (!s.ok()) {
			throw Exception("Could not read TensorFlow graph file " + networkFilename);
		}
	}

	// Assume first node is input node
	mInputName = tensorflow_graph.node(0).name();
    //auto attributes = tensorflow_graph.node(0).attr();
	//std::cout << attributes["shape"].shape() << std::endl;
	/*
    for(int i = 0; i < tensorflow_graph.node_size(); ++i) {
		tensorflow::NodeDef node = tensorflow_graph.node(i);
		reportInfo() << "Node " << i << " with name " << node.name() << reportEnd();
        reportInfo() << "Op name " << node.op() << reportEnd();
		reportInfo() << "inputs: " << node.input_size() << reportEnd();
	}
	 */

	reportInfo() << "Creating session." << reportEnd();
	tensorflow::Status s = mSession->Create(tensorflow_graph);
	if (!s.ok()) {
		throw Exception("Could not create TensorFlow Graph");
	}

	//tensorflow::graph::SetDefaultDevice("/gpu:0", &tensorflow_graph);

	// Clear the proto to save memory space.
	tensorflow_graph.Clear();
	reportInfo() << "TensorFlow graph loaded from: " << networkFilename << reportEnd();

	mModelLoaded = true;
}

void NeuralNetwork::setScaleFactor(float factor) {
    mScaleFactor = factor;
}

NeuralNetwork::NeuralNetwork() {
	createInputPort<Image>(0, true, INPUT_STATIC_OR_DYNAMIC, true);
	mModelLoaded = false;
	mInputName = "";
	mWidth = -1;
	mHeight = -1;
	mScaleFactor = 1.0f;
	createOpenCLProgram(Config::getKernelSourcePath() + "Algorithms/NeuralNetwork/NeuralNetwork.cl");
}

void NeuralNetwork::execute() {


    Image::pointer image = getStaticInputData<Image>();
	std::vector<Image::pointer> images = {image};//getMultipleStaticInputData<Image>();

	if(mWidth < 0 || mHeight < 0)
		throw Exception("Network input layer width and height has to be specified before running the network");

    images = resizeImages(images);

	executeNetwork(images);
}


void NeuralNetwork::setInputSize(int width, int height) {
	mWidth = width;
	mHeight = height;
}
void NeuralNetwork::setOutputParameters(std::vector<std::string> outputNodeNames) {
    mOutputNames = outputNodeNames;
}

std::vector<std::vector<float> > NeuralNetwork::getNetworkOutput() {
    if(mOutputData.size() != 1)
		throw Exception("If network has more than 1 output can't return network output without name.");

	return mOutputData[mOutputNames[0]];
}

std::vector<std::vector<float> > NeuralNetwork::getNetworkOutput(std::string name) {
	return mOutputData.at(name);
}

void NeuralNetwork::executeNetwork(const std::vector<Image::pointer>& images) {
    if(!mModelLoaded)
		throw Exception("Network and weights must be loaded in NeuralNetwork before execution.");
	if(mInputName == "")
		throw Exception("An input name must ge given to the NeuralNetwork before execution");
	if(mOutputNames.size() == 0)
		throw Exception("An output name must ge given to the NeuralNetwork before execution");

    int batchSize = images.size();
	if(batchSize == 0)
		throw Exception("Need at least one image to execute network.");

	// Create input tensor
	tensorflow::Tensor input_tensor(
			tensorflow::DT_FLOAT,
			tensorflow::TensorShape({batchSize, mHeight, mWidth, 1})
	);

	auto input_tensor_mapped = input_tensor.tensor<float, 4>();

	mRuntimeManager->startRegularTimer("input_data_copy");
	reportInfo() << "TensorFlow: Copying Data." << reportEnd();
	for(int n = 0; n < batchSize; ++n) {
		Image::pointer image = images[n];
		if (image->getWidth() != mWidth || image->getHeight() != mHeight)
			throw Exception("Input image sent to executeNetwork was of incrorrect size");

		ImageAccess::pointer access = image->getImageAccess(ACCESS_READ);
		for (int i = 0; i < mHeight; ++i) { // y
			for (int j = 0; j < mWidth; ++j) { // x
				input_tensor_mapped(n, i, j, 0) = access->getScalar(Vector2i(j, i))*mScaleFactor;
			}
		}
	}
	mRuntimeManager->stopRegularTimer("input_data_copy");

    // TODO Need to know names of inputs and outputs in advance
	// Input: Only single for now
	// Output: Can be multiple

	// Create a scalar tensor which tells the system we are NOT doing training
	tensorflow::Tensor input_tensor2(
			tensorflow::DT_BOOL,
			tensorflow::TensorShape() // Scalar
	);
	auto input_tensor_mapped2 = input_tensor2.tensor<bool, 0>();
	input_tensor_mapped2(0) = false;

	std::vector <std::pair<std::string, tensorflow::Tensor>> input_tensors(
			{{mInputName, input_tensor}, {"keras_learning_phase", input_tensor2}});

	std::vector <tensorflow::Tensor> output_tensors;

	tensorflow::Status s;
	mRuntimeManager->startRegularTimer("network_execution");
	s = mSession->Run(input_tensors, mOutputNames, {}, &output_tensors);
	mRuntimeManager->stopRegularTimer("network_execution");

	if (!s.ok()) {
		throw Exception("Error during inference: " + s.ToString());
	}
	reportInfo() << "Finished executing network" << reportEnd();

    for(int j = 0; j < mOutputNames.size(); ++j) {
		tensorflow::Tensor *output = &output_tensors[j];
        std::string outputName = mOutputNames[j];

		//const auto outputData = output->flat<float>(); // This is some sort of Eigen tensor type
        auto output_tensor_mapped = output->tensor<float, 2>();
		std::vector<std::vector<float>> resultData;
		for(int n = 0; n < batchSize; ++n) {
            std::vector<float> outputValues;
			for (int i = 0; i < output_tensor_mapped.dimension(1); ++i) {
				outputValues.push_back(output_tensor_mapped(0, i));
			}
			resultData.push_back(outputValues);
		}

		mOutputData[outputName] = resultData;
	}
	reportInfo() << "Finished parsing output" << reportEnd();

}

std::vector<SharedPointer<Image>> NeuralNetwork::resizeImages(const std::vector<SharedPointer<Image>> &images) {
	reportInfo() << "Resizing images.." << reportEnd();
    std::vector<Image::pointer> resizedImages;
	for(Image::pointer image : images) {
		// Resize image to fit input layer
		if(mWidth != image->getWidth() || mHeight != image->getHeight()) {
			// Only resize if needed
            ImageResizer::pointer resizer = ImageResizer::New();
            resizer->setWidth(mWidth);
            resizer->setHeight(mHeight);
            resizer->setInputData(image);
            resizer->update();
            Image::pointer resizedImage = resizer->getOutputData<Image>();
            resizedImages.push_back(resizedImage);
		} else {
			resizedImages.push_back(image);
		}
	}

	return resizedImages;
}


};
