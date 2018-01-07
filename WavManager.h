//
//  WavManager.hpp
//  voice
//
//  Created by mac on 17/1/19.
//  Copyright © 2017年 mac. All rights reserved.
//
#pragma warning(disable:4996)
#ifndef WavManager_hpp
#define WavManager_hpp

#include <stdio.h>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include "Wav.h"
#include "MLPJY.h"
using std::map;
using std::string;
using std::pair;

double ** makeArray2D(double rows, double cols);
class WavManager{
private:
    WavFeature sampleFeatures;
    std::map<string,WavFeature> modelFeatures;
    std::map<string,std::vector<Wav>> models;
	MLP * mlp;
	int modelDataAmount;
public:
    WavManager();
    void loadModel(const char * filename,const char * modelname);
    void loadSample(const char * filename);
    void currentSampleResult(char * result);
	void MLPtest_modelLoad()
	{
		map<string, std::vector<Wav>>::iterator iter;
		int hiddens[] = { 60};
		mlp = new MLP(60, 1, hiddens, modelNum());
		double ** input = makeArray2D(modelDataAmount, 60);
		double ** output = makeArray2D(modelDataAmount, 5);
		int counter = 0;
		int j = 0;
		for (iter = models.begin(); iter != models.end(); iter++)
		{
			for (int i = 0; i < iter->second.size(); i++)
			{
				memcpy(
					input[counter],
					getSampleFeatures(
						iter->second[i]
					).dynamic_zero_rate,
					sizeof(double) * 30
				);
				memcpy(
					input[counter] + 30,
					getSampleFeatures(
						iter->second[i]
					).envelope,
					sizeof(double) * 30
				);
				output[counter][j] = 1;
				counter++;
			}
			j++;
		}

		mlp->train((const double**)input, (const double**)output, modelDataAmount);
	}
	void MLPtest_sampleLoad(const char * filename, char * result)
	{
		Wav temp = wavRead(filename);
		sampleFeatures = getSampleFeatures(temp);
		double * out = new double[modelNum()];
		double * in = new double[60];
		memcpy(
			in,
			sampleFeatures.dynamic_zero_rate,
			sizeof(double) * 30
		);
		memcpy(
			in + 30,
			sampleFeatures.envelope,
			sizeof(double) * 30
		);
		mlp->work((const double*)in, out);

		for (int i = 0; i < modelNum(); i++)
		{
			char temp[20];
			sprintf(temp, "%.8lf, ", out[i]);
			strcat(result, temp);
		}
		fclose(mlp->log_errors);
		fclose(mlp->log_weights);
	}
    int modelNum();
    void getLossOf(const char * modelTitle,char * resultStr);
};

#endif /* WavManager_hpp */
