#pragma once
#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

enum LayerType {
	Input = 0, Output = 1, Hidden = 2, Unknown = 3,
};
struct Layer {
	double * value;
	double * value2;
	int neural_amount;
	LayerType type; // 0 for input, 1 for output, 2 for hidden

	Layer(LayerType type, int amount);
	void fillAsInput(double * data);
	void fillIn(const double* data);
	void clear();
	void print()
	{
		printf("layer type : %d, length = %d\n value : ", type, neural_amount);
		for (int i = 0; i < neural_amount; i++)
		{
			printf("%.8lf, ", value[i]);
		}
		printf("\n value2: ");
		for (int i = 0; i < neural_amount; i++)
		{
			printf("%.8lf, ", value2[i]);
		}
		printf("\n");
	}
	Layer();
	~Layer();
};
struct MLP {
	int hidden_layer_amount;
	int input_neural_amount;
	int output_neural_amount;
	int weight_layer_amount;
	int * hidden_neural_amount;

	Layer * input_layer;
	Layer * output_layer;
	Layer * hidden_layers;

	double *** weights;
	double *** weights2;
	double * sigmas;
	double * biases;

	FILE * log_weights;
	FILE * log_errors;

	double study_rate;
	MLP() = default;
	MLP(
		int input,
		int hidden_layer,
		int * hiddens,
		int output,
		const double * biases = NULL,
		const double *** table = NULL
	);
	void train(const double ** inputs, const double ** expected_outputs, int dataset_amount);
	~MLP();
	double getError(const double * targets)
	{
		double tot = 0.0;
		for (int i = 0; i < output_neural_amount; i++)
		{
			tot += (output_layer->value2[i] - targets[i]) * (output_layer->value2[i] - targets[i]);
		}
		return tot / 2;
	}
	void work(const double * input, double * out)
	{
		singleRun(input);
		memcpy(out, output_layer->value2, sizeof(double) * output_neural_amount);
	}
private:
	void saveWeights();
	void changeWeights2AndWeights();
	Layer * lastHidden();
	void lastWeightsCorrection();
	void backRun();
	void calculateSigmas(const double * targets);
	void executeLayer(
		Layer * front, 
		Layer * back, 
		double ** weight, 
		double bias, 
		bool inverse = false
	);
	void singleRun(const double * input);
};