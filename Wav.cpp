//
//  Wav.cpp
//  voice
//
//  Created by mac on 17/1/19.
//  Copyright © 2017年 mac. All rights reserved.
//
#pragma warning(disable : 4996)
#include "Wav.h"



//#pragma once


unsigned char DATA_FLAG[] = { 0x64,0x61,0x74,0x61 };
void save_array(double * arr,int num,char * filename)
{
    FILE * fp = fopen(filename, "w");
    for (int i = 0;i < num;i++)
    {
        fprintf(fp, "%d\t%lf\n", i, arr[i]);
    }
    fclose(fp);
}
Wav::Wav(int fre, int len) :frequency(fre), length(len) { data = new double[length]; }
Wav::Wav() :frequency(0),length(0),data(NULL){}
void Wav::set_data(double * arr)
{
    memcpy(data, arr, sizeof(double) * length);
}
void Wav::set_frequency(int freq)
{
    frequency = freq;
}
void Wav::set_length(int len)
{
    if (data)delete[] data;
    data = new double[len];
    length = len;
}
void Wav::save(char * filename)
{
    FILE * fp = fopen(filename, "w");
    for (int i = 0;i < length;i++)
    {
        fprintf(fp,"%2.6lf\n", data[i]);
    }
    fprintf(fp,"frequency = %d , length = %d", frequency, length);
    fclose(fp);
}
void Wav::console_display()
{
    for (int i = 0;i < length;i++)
    {
        printf("%2.6lf\n", data[i]);
    }
    printf("frequency = %d , length = %d", frequency, length);
}
Wav Wav::down_sampling(int step)
{
    Wav tot(frequency / (double)step, length / (double)step);
    for (int i = 0;i < tot.length;i++)
    {
        double sum = 0;
        int frist_index = i * step;
        for (int j = 0;j < step;j++)
        {
            sum += data[frist_index + j];
        }
        tot.data[i] = sum / step;
    }
    return tot;
}
Wav Wav::subwav(int first, int last)
{
    Wav tot(frequency, last - first + 1);
    tot.set_data(data + first);
    return tot;
}
Wav Wav::get_waving_area(double threshold1,double threshold2,double threshold3)
{
    int first = -1, last = -1;
    
    //bool in = false;
    int judge_num = threshold2 * frequency;
    int i = 0;
    for (;i < length;i++)
        if (data[i] > threshold1)
        {
            int counter = 0;
            int j = 0;
            for(;j<judge_num;j++)
                if (data[j + i] > threshold1)
                {
                    counter++;
                }
            if (counter > judge_num * threshold3)goto headdone;
            if(counter < 10)i += j;
        }
headdone:;
    first = i;
    i = length - 1;
    for (;i > first;i--)
    {
        if (data[i] > threshold1)
        {
            int counter = 0;
            int j = 0;
            for (;j<judge_num;j++)
                if (data[i - j] > threshold1)
                {
                    counter++;
                }
            if (counter > judge_num * threshold3)goto taildone;
            if (counter < 10)i -= j;
        }
    }
taildone:;
    last = i;
    return subwav(first, last);
}
void Wav::get_envelope(double * output,double sample_num)
{
    //Wav tot(sample_num, length / (double)step);
    double step = length / sample_num;
    for (int i = 0;i < sample_num;i++)
    {
        int first_index = i * step + 0.5;
        double best = data[first_index];
        for (int j = 0;j < step;j++)
        {
            if (best < data[first_index + j])
                best = data[first_index + j];
        }
        output[i] = best;
    }
}
void Wav::get_dynamic_zero_rate(double * out ,int n)
{
    if (out == NULL)out = new double[n];
    int window_n = length / n;
    for (int i = 0;i < n;i++) {
        double num = 0;
        for (int j = 0;j < window_n - 1;j++) {
            if (data[window_n * i + j] * data[window_n * i + j + 1] < 0)num += 1;
        }
        out[i] = num / window_n;
    }
}

void parseData(FILE * fp, int offset, void * ptr, int size)
{
    fseek(fp, offset, SEEK_SET);
    fread(ptr, size, 1, fp);
}
WavFeature getSampleFeatures(Wav & wav){
    WavFeature tot;
    Wav wa = wav.get_waving_area();
    wa.get_envelope(tot.envelope,30);
    wa.get_dynamic_zero_rate(tot.dynamic_zero_rate,30);
    return tot;
}
void array_multi(double * arr1,double * arr2,int n)
{
    for(int i = 0;i<n;i++)
    {
        arr1[i] += arr2[i];
    }
}
void array_divide(double * arr1,double who,int n)
{
    for(int i = 0;i<n;i++)
    {
        arr1[i] /= who;
    }
}
WavFeature getModelFeatures(std::vector<Wav> & waves)
{
    WavFeature tot;
    for(int i = 0;i<waves.size();i++)
    {
        double env[30],dyzr[30];
        Wav wa = waves[i].get_waving_area();
        wa.get_envelope(env,30);
        wa.get_dynamic_zero_rate(dyzr,30);
        array_multi(tot.envelope,env,30);
        array_multi(tot.dynamic_zero_rate,dyzr,30);
    }
    array_divide(tot.dynamic_zero_rate, waves.size(), 30);
    array_divide(tot.envelope, waves.size(), 30);
    return tot;
}
void distanceDescribeString(WavFeature & wf1,WavFeature & wf2,int n,char * out)
{
    double loss1 = 0;
    double loss2 = 0;
    double * arr1 = wf1.envelope;
    double * arr2 = wf2.envelope;
    for(int i = 0;i<n;i++)
    {
        loss1 += pow((arr1[i] - arr2[i]) * 100,2);
        //printf("%d std : %lf ,real : %lf\n",i,arr1[i],arr2[i]);
    }
    arr1 = wf1.dynamic_zero_rate;
    arr2 = wf2.dynamic_zero_rate;
    for(int i = 0;i<n;i++)
    {
        loss2 += pow((arr1[i] - arr2[i]) * 100,2);
        //printf("%d std : %lf ,real : %lf\n",i,arr1[i],arr2[i]);
    }
    sprintf(out,"%lf+%lf",loss1,loss2);
    //return loss2;
}
double distance(WavFeature & wf1,WavFeature & wf2,int n)
{
    double loss = 0;
    double * arr1 = wf1.envelope;
    double * arr2 = wf2.envelope;
    for(int i = 0;i<n;i++)
    {
        loss += pow((arr1[i] - arr2[i]) * 100,2);
        //printf("%d std : %lf ,real : %lf\n",i,arr1[i],arr2[i]);
    }
    arr1 = wf1.dynamic_zero_rate;
    arr2 = wf2.dynamic_zero_rate;
    for(int i = 0;i<n;i++)
    {
        loss += pow((arr1[i] - arr2[i]) * 100,2);
        //printf("%d std : %lf ,real : %lf\n",i,arr1[i],arr2[i]);
    }
    return loss;
}
Wav wavRead(const char * filename)
{
    Wav wav;
    
    unsigned long file_size;        //文件大小
    unsigned short channel;            //通道数
//    unsigned long frequency;        //采样频率
    unsigned short sample_num_bit;    //一个样本的位数
    int first_index;
    FILE * fp = fopen(filename, "rb");
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
	parseData(fp, 0x14, &channel, sizeof(channel));
//    parseData(fp, 0x18, &frequency, sizeof(frequency));
    parseData(fp, 0x22, &sample_num_bit, sizeof(sample_num_bit));
    
    int left_size = file_size - 0x22 - sizeof(sample_num_bit);
    unsigned char * raw = new unsigned char[left_size];
    
    fread(raw, left_size, 1, fp);
    for (int i = 0;i < left_size;i++)
    {
        if (memcmp(raw + i, DATA_FLAG, 4) == 0)
        {
            unsigned long raw_length;
            fseek(fp, 0x22 + sizeof(sample_num_bit) + i, SEEK_SET);
            parseData(fp, 0x22 + sizeof(sample_num_bit) + i + 4,&raw_length,sizeof(raw_length));
            switch (sample_num_bit)
            {
                case 16:
                    wav = Wav(44100, raw_length / 2);
                    first_index = 0x22 + sizeof(sample_num_bit) + i + 4 + sizeof(raw_length);
                    for (int j = 0;j < raw_length;j += 2)
                    {
                        unsigned long data_low = raw[j + first_index];
                        unsigned long data_high = raw[j + first_index + 1];
                        double data_true = data_high * 256 + data_low;
                        long data_complement = data_true;
                        if ((int)(data_high / 128) == 1)
                            data_complement = data_true - 65536;
                        wav.data[j / 2] = (double)(data_complement / (double)32768);
                    }
                    return wav;
                case 8:
                    wav = Wav(44100, raw_length);
                    first_index = 0x22 + sizeof(sample_num_bit) + i + 4 + sizeof(raw_length);
                    for (int j = 0;j < raw_length;j += 2)
                    {
                        double data_true = raw[j + first_index];
                        long data_complement = data_true;
                        if ((int)(data_true / 128) == 1)
                            data_complement = data_true - 65536;
                        wav.data[j / 2] = (double)(data_complement / (double)32768);
                    }
                    return wav;
            }
        }
    }
    return Wav(0, 0);
}
