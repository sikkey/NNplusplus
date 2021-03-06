//
//  NeuralNet.cpp
//  Neural Net
//
//  Created by Gil Dekel on 8/19/16.
//  Last edited by Gil Dekel on 8/30/16.
//
#define  _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include<string.h>
#include <string>

#include "NeuralNet.hpp"

/*
 * Private members for reference
 *
 * size_t inNodes_;
 * size_t hiddNodes_;
 * size_t outNodes_;
 * size_t hiddLayers_;
 * double LR_;
 *
 * std::vector<Matrix> weights_;
 * std::vector<Matrix> outputs_;
 *
 */

/**********************************************************
 * Constructors
 **********************************************************/

NeuralNet::NeuralNet(size_t inputNodes, size_t hiddenNodes, size_t outputNodes, size_t hiddenLayers, double learningRate )
    : inNodes_{inputNodes}, hiddNodes_{hiddenNodes}, outNodes_{outputNodes}, hiddLayers_{hiddenLayers}, LR_{learningRate},
      weights_{std::vector<Matrix>()}, outputs_{std::vector<Matrix>()} {
    
    size_t weightsSize = 1+hiddLayers_;
    weights_.reserve(weightsSize);
          
    size_t outputsSize = 2+hiddLayers_;
    outputs_.reserve(outputsSize);
          
    for (size_t i = 0; i < weightsSize; ++i) {
        size_t currLayer = 0;
        size_t nextLayer = 0;
        
        if (i == 0) {
            currLayer = inNodes_;
            nextLayer = hiddNodes_;
        } else if (i == weightsSize-1) {
            currLayer = hiddNodes_;
            nextLayer = outNodes_;
        } else {
            currLayer = hiddNodes_;
            nextLayer = hiddNodes_;
        }
        
        weights_.push_back(initializeMatrix(nextLayer, currLayer));
    }
    
    for (size_t i = 0; i < outputsSize; ++i) {
        size_t numOfNodes = 0;
        
        if (i == 0) {
            numOfNodes = inNodes_;
        } else if (i == outputsSize-1) {
            numOfNodes = outNodes_;
        } else {
            numOfNodes = hiddNodes_;
        }
        
        outputs_.push_back(Matrix(numOfNodes, 1));
    }
}

NeuralNet::NeuralNet(const std::string &filename) {
    if (filename.substr(filename.length()-3).compare(".nn") != 0) {
        std::cout << "ERROR:: FILE MUST BE OF TYPE *.nn\n";
        exit(1);
    }
    
    std::ifstream in(filename);
    if (in.fail()) {
        std::cout << "ERROR:: CANNOT READ FROM FILE: '" << filename << "'\n";
        exit(1);
    }
    
    in >> inNodes_ >> hiddNodes_ >> outNodes_ >> hiddLayers_ >> LR_;
    
    weights_ = std::vector<Matrix>();
    size_t weightsSize = 1+hiddLayers_;
    weights_.reserve(weightsSize);
    
    outputs_ = std::vector<Matrix>();
    size_t outputsSize = 2+hiddLayers_;
    outputs_.reserve(outputsSize);
    
    size_t Mrows = 0, Ncols = 0;
    double nextVal = 0;
    
    for (size_t i = 0; i < weightsSize; ++i) {
        in >> Mrows >> Ncols;
        weights_.push_back(Matrix(Mrows, Ncols));
        
        for (size_t m = 0; m < Mrows; ++m) {
            for (size_t n = 0; n < Ncols; ++n) {
                in >> nextVal;
                weights_.back()(m,n) = nextVal;
            }
        }
    }
    
    for (size_t i = 0; i < outputsSize; ++i) {
        size_t numOfNodes = 0;
        
        if (i == 0) {
            numOfNodes = inNodes_;
        } else if (i == outputsSize-1) {
            numOfNodes = outNodes_;
        } else {
            numOfNodes = hiddNodes_;
        }
        
        outputs_.push_back(Matrix(numOfNodes, 1));
    }
}

/**********************************************************
 * Other Functions
 **********************************************************/

Matrix NeuralNet::queryNet(const Matrix &inputList) {
    Matrix finalOutput{inputList.T()};
    outputs_[0] = finalOutput;
    
    for (size_t i = 0; i < weights_.size(); ++i) {
        finalOutput = weights_[i].dot(finalOutput);
        
        for (size_t m = 0; m < finalOutput.getNumOfRows(); ++ m) {
            for (size_t n = 0; n < finalOutput.getNumOfCols(); ++n) {
                finalOutput(m,n) = activationFunction(finalOutput(m,n));
            }
        }
        outputs_[i+1] = finalOutput;
    }
    
    return finalOutput;
}

void NeuralNet::trainingCycle(const Matrix &inputList, const Matrix &targetOutput) {
    Matrix currOutput{queryNet(inputList)};                     // Returned transposed
    Matrix currTargetOut{targetOutput.T()};
    Matrix currLayerErrors{currTargetOut-currOutput};            // Calculate the final output layer's error
    
    // Update the weights going from the output nodes back
    for (long int i = weights_.size()-1; i >= 0; --i) {
        Matrix prevLayerErrors{weights_[i].T().dot(currLayerErrors)};
        Matrix prevHiddLayerOutsT{outputs_[i].T()};
        
        Matrix deltaWeights{currLayerErrors*currOutput};
        deltaWeights *= (1-currOutput);
        deltaWeights = deltaWeights.dot(prevHiddLayerOutsT);
        deltaWeights *= LR_;
        weights_[i] += deltaWeights;
        
        currLayerErrors = prevLayerErrors;
        currOutput = outputs_[i];
    }
}

void NeuralNet::saveNetwork(const std::string &name) const {
    std::string fileName;
    
	if (name.empty()) {
		fileName = (getCurrTime() + ".nn");
	}
	else {
		if (name.length() > 3 && name.substr(name.length() - 3).compare(".nn") != 0) {
			fileName = (name + ".nn");
		}
		else {
			fileName = name;
		}
	}
    
    std::ofstream out(fileName);
    if (out.fail()) {
        std::cout << "ERROR:: Fails writing to file " << (getCurrTime() + ".nn") << std::endl;
        exit(1);
    }
    
    out << inNodes_ << " " << hiddNodes_ << " " << outNodes_ << " " << hiddLayers_ << " " << LR_ << std::endl;
    
    for (size_t i = 0; i < weights_.size(); ++i) {
        out << weights_[i].getNumOfRows() << " " << weights_[i].getNumOfCols() << std::endl;
        for (size_t m = 0; m < weights_[i].getNumOfRows(); ++m) {
            for (size_t n = 0; n < weights_[i].getNumOfCols(); ++n) {
                out << weights_[i](m,n) << " ";
            }
            out << std::endl;
        }
    }
}

void NeuralNet::loadNetwork(const std::string &name) {
    *this = NeuralNet(name);
}

/**********************************************************
 * Private Functions
 **********************************************************/

Matrix NeuralNet::initializeMatrix(size_t rows, size_t cols) const {
    Matrix init(rows,cols);
    std::default_random_engine generator((std::random_device()()));
    std::normal_distribution<double> distribution(0.0, std::pow(rows, -0.5));
    
    for (size_t m = 0; m < rows; ++m) {
        for (size_t n = 0; n < cols; ++n) {
            init(m,n) = distribution(generator);
        }
    }
    return init;
}

// The activation function. Currently using Sigmoid function.
double NeuralNet::activationFunction(double x) const {
    return 1/(1+std::exp(-x));
}

std::string NeuralNet::getCurrTime() const {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    std::string currTime = std::to_string(now->tm_year + 1900) + '-' + std::to_string(now->tm_mon + 1) + '-' + std::to_string(now->tm_mday);
    currTime += "--" + ((now->tm_hour < 10) ? "0" + std::to_string(now->tm_hour) : std::to_string(now->tm_hour));
    currTime += "-" + ((now->tm_min < 10) ? "0" + std::to_string(now->tm_min) : std::to_string(now->tm_min));
    currTime += "-" + ((now->tm_sec < 10) ? "0" + std::to_string(now->tm_sec) : std::to_string(now->tm_sec));
    return currTime;
}
