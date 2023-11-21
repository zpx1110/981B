#include "randomforest.h"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <ctime>
#include <cstdlib>


using namespace std;

int maxTime;

randomForest::randomForest(int numTrees, int maxDepth) : numTrees(numTrees), maxDepth(maxDepth)
{
    srand(time(nullptr));
}

void randomForest::train(const vector<InputData>& data)
{
    //record max time
    maxTime = 0;
    for (const InputData& sample : data)
    {
        for (quint64 time : sample.input)
        {
            if (maxTime < time)
                maxTime = time;
        }
    }

    for (int i = 0; i < numTrees; ++i)
    {
        vector<InputData> subset = getRandomSubset(data, data.size() * 2 / 3);

        trees.push_back(buildDecisionTree(subset, maxDepth));
    }
}

bool randomForest::predict(const vector<quint64>& pressingTime)
{
    int positiveVotes = 0;
    for (const TreeNode* tree : trees)
    {
        const TreeNode* currentNode = tree;
        while (!currentNode->isLeaf)
        {
            if (pressingTime[currentNode->featureIndex] < currentNode->threshold)
            {
                currentNode = currentNode->left;
            }
            else
            {
                currentNode = currentNode->right;
            }
        }
        positiveVotes += currentNode->leafClass ? 1 : 0;
    }
    return positiveVotes > numTrees / 2;
}

//random subset
# if 1
vector<InputData> randomForest::getRandomSubset(const vector<InputData>& data, int subsetSize)
{
    vector<InputData> subset;
    for (int i = 0; i < subsetSize; i++)
    {
        int index = rand() % data.size();
        subset.push_back(data[index]);
    }
    return subset;
}

#else
vector<InputData> randomForest::getRandomSubset(const vector<InputData>& data)
{
    vector<InputData> subset;
    for (size_t i = 0; i < data.size(); i++)
    {
        int index = rand() % data.size();
        subset.push_back(data[index]);
    }
    return subset;
}
#endif

//class ratio
double randomForest::calculateClassRatio(const vector<InputData>& data)
{
    int positiveCount = 0;
    for (const InputData& sample : data)
    {
        if (sample.isMatch)
        {
            positiveCount++;
        }
    }

    return static_cast<double>(positiveCount) / data.size();
}

// gini index
double randomForest::calculateGiniIndex(const vector<InputData>& data)
{
    double classRatio = calculateClassRatio(data);
    return classRatio * (1 - classRatio);
}

//choose threshold
#if 1
void randomForest::findBestSplit(const vector<InputData>& data, int& bestFeatureIndex, double& bestThreshold)
{
    //random choose feature and threshold
    bestFeatureIndex = rand() % data[0].input.size();
    bestThreshold = (rand() % 100) / 100.0 * maxTime;
}
#else
//Traverse to find the feature that reduces gini the most as the best split
void randomForest::findBestSplit(const vector<InputData>& data, int& bestFeatureIndex, double& bestThreshold)
{
    double bestGini = 1.0;
    bestFeatureIndex = -1;
    bestThreshold = 0.0;

    for (size_t featureIndex = 0; featureIndex < data[0].input.size(); ++featureIndex)
    {
        vector<double> possibleThresholds;

        // get all possible threshold
        for (const auto& sample : data) {
            possibleThresholds.push_back(sample.input[featureIndex]);
        }
        sort(possibleThresholds.begin(), possibleThresholds.end());
        possibleThresholds.erase(unique(possibleThresholds.begin(), possibleThresholds.end()), possibleThresholds.end());

        // Traverse all possible thresholds and find the one with the smallest Gini index
        for (double threshold : possibleThresholds) {
            vector<InputData> leftSubset, rightSubset;
            for (const auto& sample : data) {
                if (sample.input[featureIndex] < threshold) {
                    leftSubset.push_back(sample);
                } else {
                    rightSubset.push_back(sample);
                }
            }
            double giniLeft = calculateGiniIndex(leftSubset);
            double giniRight = calculateGiniIndex(rightSubset);
            double weightedGini = (leftSubset.size() * giniLeft + rightSubset.size() * giniRight) / data.size();
            if (weightedGini < bestGini) {
                bestGini = weightedGini;
                bestFeatureIndex = featureIndex;
                bestThreshold = threshold;
            }
        }
    }
}
#endif

//build tree
TreeNode* randomForest::buildDecisionTree(const vector<InputData>& data, int depth)
{
    if (depth == 0 || calculateGiniIndex(data) == 0 || data.empty())
    {
        TreeNode* leaf = new TreeNode;
        leaf->isLeaf = true;
        leaf->leafClass = calculateClassRatio(data) > 0.5;
        return leaf;
    }

    int bestFeatureIndex;
    double bestThreshold;
    findBestSplit(data, bestFeatureIndex, bestThreshold);

    vector<InputData> leftSubset, rightSubset;
    for (const InputData& sample : data)
    {
        if (sample.input[bestFeatureIndex] < bestThreshold)
        {
            leftSubset.push_back(sample);
        }
        else
        {
            rightSubset.push_back(sample);
        }
    }

    TreeNode* node = new TreeNode;
    node->isLeaf = false;
    node->featureIndex = bestFeatureIndex;
    node->threshold = bestThreshold;
    node->left = buildDecisionTree(leftSubset, depth - 1);
    node->right = buildDecisionTree(rightSubset, depth - 1);

    return node;
}

// grid search
void randomForest::gridSearch(const vector<InputData>& data, const vector<int>& numTreesOptions, const vector<int>& maxDepthOptions)
{
    double bestAccuracy = 0.0;
    int bestNumTrees = 0;
    int bestMaxDepth = 0;

    for (int numTrees : numTreesOptions)
    {
        for (int maxDepth : maxDepthOptions)
        {
            double accuracy = crossValidate(data, numTrees, maxDepth);
            if (accuracy > bestAccuracy)
            {
                bestAccuracy = accuracy;
                bestNumTrees = numTrees;
                bestMaxDepth = maxDepth;
            }
        }
    }
    // Set global params for latter training process
    numTrees = bestNumTrees;
    maxDepth = bestMaxDepth;

    cout << "Best parameters: numTrees = " << bestNumTrees << ", maxDepth = " << bestMaxDepth << " with accuracy = " << bestAccuracy << endl;
}

// cross validate
double randomForest::crossValidate(const vector<InputData>& data, int numTrees, int maxDepth)
{

    int correctPredictions = 0;
    int totalPredictions = 0;

    // Suppose only do a simple training and validation
    vector<InputData> trainingData = getRandomSubset(data, data.size()); // use bootstrap
    vector<InputData> validationData = data; // A simple try, the trainingData part should actually be removed from the data

    randomForest rf(numTrees, maxDepth);
    rf.train(trainingData);

    for (const auto& sample : validationData)
    {
        bool prediction = rf.predict(sample.input);

        if (prediction == sample.isMatch)
        {
            correctPredictions++;
        }
        totalPredictions++;
    }

    return static_cast<double>(correctPredictions) / totalPredictions;
}


