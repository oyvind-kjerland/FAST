/**
 * Examples/GUI/SimpleGUI/main.hpp
 *
 * If you edit this example, please also update the wiki and source code file in the repository.
 */
#include "CoronaryGUI.hpp"
#include <boost/program_options.hpp>

#include<iostream>
#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>



namespace po = boost::program_options;

using namespace fast;


int main(int argc, char* argv[]) {



	// Parse input arguments
	po::options_description desc("Allowed options");
	desc.add_options()
		// Generic
		("help", "show help message")

		// Settings
		("dataset", po::value<std::string>(), "dataset number to run the algorithm on, ex: 01, 02, 03")
		("show-slices", "Visualize slices")
		("show-reference", "Visualize reference centerlines")
		("no-cache", "prevents exports and caching result of the performed steps")

		// Methods
		("visualize-image", po::value<std::string>(), "visualize image")

		//
		// Algorithms
		//

		// Preprocessing
		("median", "perform median filtering")
		("lung-tissue-removal", "perform lung tissue removal")
		("image-gradient", "perform image gradient")
		("gradient-vector-flow", "perform gradient vector flow")

		// Tube detection filter
		("image-gradient-hessian", "perform Hessian analysis on image gradients to obtain the tangents and the eigenvalues")
		("gradient-vector-flow-hessian", "perform Hessian analysis on gradient vector flow to obtain the tangents and the eigenvalues")
		("image-gradient-tdf","perform Frangi tube detection filter on the image gradient")
		("gradient-vector-flow-tdf", "perform Frangi tube detection filter on the gradient vector flow")
		("max-tdf", "perform MaxTDF, taking the maximum of the TDF applied to the original image gradient and the gradient vector flow result")
		
		// Centerline extraction
		("ridge-candidate-selection", "perform ridge candidate selection")
		("ridge-traversal", "perform ridge traversal")
		("tree-reconstruction", "perform tree reconstruction")

		// Labeled data creation
		("tube-from-reference", "Create binary volume tubes from reference centerlines")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	// Print help
	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	// Create coronary GUI window
	CoronaryGUI::pointer window = CoronaryGUI::New();




	if (vm.count("dataset")) {
		window->setDataset(vm["dataset"].as<std::string>());
	}



	if (vm.count("no-export")) {
		window->disableCache();
	}




	if (vm.count("visualize-image")) {
		window->visualizeImage(vm["visualize-image"].as<std::string>());

	}

	if (vm.count("median")) {
		window->performMedianFilter();
	}

	if (vm.count("image-gradient")) {
		window->performImageGradient();
	}

	if (vm.count("gradient-vector-flow")) {
		window->performGradientVectorFlow();
	}

	if (vm.count("image-gradient-hessian")) {
		window->performImageGradientHessian();
	}

	if (vm.count("gradient-vector-flow-hessian")) {
		window->performGradientVectorFlowHessian();
	}
	if (vm.count("image-gradient-tdf")) {
		window->performImageGradientTDF();
	}

	if (vm.count("gradient-vector-flow-tdf")) {
		window->performGradientVectorFlowTDF();
	}

	if (vm.count("max-tdf")) {
		window->performMaxTDF();
	}

	if (vm.count("ridge-candidate-selection")) {
		window->performRidgeCandidateSelection();
	}

	if (vm.count("ridge-traversal")) {
		window->performRidgeTraversal();
	}

	if (vm.count("tree-reconstruction")) {
		window->performTreeReconstruction();
	}

	if (vm.count("lung-tissue-removal")) {
		window->performLungTissueRemoval();
	}

	if (vm.count("tube-from-reference")) {
		window->createTubeFromReference();
	}



	//if (vm.count("show-slices")) {
	window->showSlices();
	//}


	if (vm.count("show-reference")) {
		window->showReference();
	}





	window->start();

	return 0;
}


