//
//  WavManager.cpp
//  voice
//
//  Created by mac on 17/1/19.
//  Copyright © 2017年 mac. All rights reserved.
//

#include "WavManager.h"
double ** makeArray2D(double rows, double cols)
{
	double ** tot = new double*[rows];

	for (int i = 0; i < rows; i++)
	{
		tot[i] = new double[cols];
		memset(tot[i], 0, sizeof(double) * cols);
	}
	return tot;
}
WavManager::WavManager(){
    modelFeatures = std::map<string, WavFeature>();
    models = std::map<string, std::vector<Wav>>();
	modelDataAmount = 0;
}
int WavManager::modelNum()
{
    return modelFeatures.size();
}
void WavManager::getLossOf(const char * modelTitle,char * resultStr)
{
    map<string, WavFeature>::iterator  iter = modelFeatures.find(string(modelTitle));
    if(iter != modelFeatures.end())
    {
        distanceDescribeString(iter->second,sampleFeatures,30,resultStr);
    }
    else strcpy(resultStr, "INF+INF");
}
void WavManager::loadModel(const char * filename,const char * modelname){
    Wav temp = wavRead(filename);
    string modelname_string = string(modelname);
    map<string, WavFeature>::iterator iter = modelFeatures.find(modelname_string);
    if(iter != modelFeatures.end())
    {
        std::vector<Wav> & waves = models.find(modelname_string)->second;
        waves.push_back(temp);
        iter->second = getModelFeatures(waves);
		modelDataAmount++;
    }
    else
    {
        std::vector<Wav> wave = std::vector<Wav>();
        wave.push_back(temp);
        modelFeatures.insert(pair<string, WavFeature>(modelname_string, getModelFeatures(wave)));
        models.insert(pair<string,std::vector<Wav>>(modelname_string,wave));
		modelDataAmount++;
    }
}
void WavManager::loadSample(const char * filename)
{
    Wav temp = wavRead(filename);
    sampleFeatures = getSampleFeatures(temp);
}
void WavManager::currentSampleResult(char * result)
{
    map<string, WavFeature>::iterator  iter;
    std::vector<double> loss;
    std::vector<string> names;
    for(iter = modelFeatures.begin(); iter != modelFeatures.end(); iter++)
    {
        loss.push_back(distance(iter->second,sampleFeatures));
        names.push_back(iter->first);
    }
    auto smallest = std::min_element(std::begin(loss), std::end(loss));
    int i = std::distance(std::begin(loss), smallest);
    strcpy(result,names[i].c_str());
}
