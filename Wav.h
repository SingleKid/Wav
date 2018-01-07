
#pragma once
#include <vector>
#include <math.h>

//#pragma warning(disable : 4996)
#include<string.h>
#include<stdio.h>

void save_array(double * arr,int num,char * filename);
struct WavFeature{
	double envelope[30];
	double dynamic_zero_rate[30];
	//double short_fft_varilation[30]; 
	WavFeature()
	{
		for(int i = 0;i<30;i++)
		{
			envelope[i] = 0;
			dynamic_zero_rate[i] = 0;
		}
	}
};
class Wav {
private:
	int frequency;
	int length;
public:
	double * data;
    Wav(int fre, int len);
    Wav() ;
    void set_data(double * arr);
    void set_frequency(int freq);
    void set_length(int len);
    void save(char * filename);
    void console_display();
    Wav down_sampling(int step);
    Wav subwav(int first, int last);
    Wav get_waving_area(double threshold1 = 0.02,double threshold2 = 0.0454,double threshold3 = 0.1);
    void get_envelope(double * output,double sample_num = 20);
    void get_dynamic_zero_rate(double * out ,int n = 20);
};
void parseData(FILE * fp, int offset, void * ptr, int size);
WavFeature getSampleFeatures(Wav & wav);
void array_multi(double * arr1,double * arr2,int n);
void array_divide(double * arr1,double who,int n);
WavFeature getModelFeatures(std::vector<Wav> & waves);
void distanceDescribeString(WavFeature & wf1,WavFeature & wf2,int n,char * out);
double distance(WavFeature & wf1,WavFeature & wf2,int n = 30);
Wav wavRead(const char * filename);


