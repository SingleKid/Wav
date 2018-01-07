#include<stdio.h>
#include"WavManager.h"
//#include"Wav.h"
int main()
{
	const int modelAmount = 10;
	const char modelInfo[2][10][24] = {
		{
			"models/climb_1.wav",
			"models/climb_2.wav",
			"models/follow_1.wav",
			"models/follow_2.wav",
			"models/godown_1.wav",
			"models/godown_2.wav",
			"models/land_1.wav",
			"models/land_2.wav",
			"models/takeoff_1.wav",
			"models/takeoff_2.wav"
		},
		{
			"climb",
			"climb",
			"follow",
			"follow",
			"godown",
			"godown",
			"land",
			"land",
			"takeoff",
			"takeoff"
		}
	};

	const int sampleAmount = 5;
	const char sampleInfo[2][5][24] = {
		{
			"samples/climb_3.wav",
			"samples/follow_3.wav",
			"samples/godown_3.wav",
			"samples/land_3.wav",
			"samples/takeoff_3.wav"
		},
		{
			"climb",
			"follow",
			"godown",
			"land",
			"takeoff",
		}
	};

	WavManager manager = WavManager();
	int success_count = 0;
	for (int i = 0; i < modelAmount; i++)
	{
		manager.loadModel(modelInfo[0][i], modelInfo[1][i]);
		printf("model %d loaded, which is a %s\n", i + 1, modelInfo[1][i]);
	}
	
	for (int i = 0; i < sampleAmount; i++)
	{
		char recongnition_result[24];
		manager.loadSample(sampleInfo[0][i]);
		manager.currentSampleResult(recongnition_result);

		printf("sample %d : %s, result = %s\n",
			i + 1,
			sampleInfo[1][i],
			(strcmp(recongnition_result, sampleInfo[1][i]) == 0) ? 
			(success_count++, "success!") : (printf("Considered to be a %s, ", recongnition_result), "failed!")
		);

		for (int i = 0; i < manager.modelNum(); i++)
		{
			char lossOf[24];
			manager.getLossOf(modelInfo[1][i * 2], lossOf);
			printf("--model %d : %s\n", i + 1, lossOf);
		}
	}

	printf("total : %d, success : %d, fail : %d, ROS : %.2lf\n", sampleAmount, success_count, sampleAmount - success_count, success_count * 1.0 / sampleAmount);
	
	manager.MLPtest_modelLoad();
	for (int i = 0; i < sampleAmount; i++)
	{
		char recongnition_result[100] = "";
		manager.MLPtest_sampleLoad(sampleInfo[0][i], recongnition_result);
		printf("%s\n", recongnition_result);
	}

	
	

	//WavManager manager;
	//Wav sample = wavRead("testing3.wav");
	//sample.save("data1.txt");



	system("pause");
}