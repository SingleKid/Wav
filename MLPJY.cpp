#include "MLPJY.h"

void malloc_weights_for(int front, int back, double **& out, const double ** table = NULL)
{
	if (!table) srand(time(0));//[0,RAND_MAX]
	out = new double*[front];
	for (int i = 0; i < front; i++)
	{
		out[i] = new double[back];
		for (int j = 0; j < back; j++)
		{
			if (!table)out[i][j] = ((rand() % 101) - 50) / 100.0; // make the random value with range[-0.5,0.5] and step 0.01
			else out[i][j] = table[i][j];
		}
	}
}
double normolizeFunction(double x)
{
	return 1.0 / (1 + exp(-x));
}

Layer::Layer(LayerType type, int amount)
{
	this->type = type;
	neural_amount = amount;
	
	value = new double[amount];
	value2 = new double[amount];
	memset(value, 0, sizeof(double) * amount);
	memset(value2, 0, sizeof(double) * amount);
}
void Layer::fillAsInput(double * data)
{
	memcpy(value2, data, neural_amount * sizeof(double));
}

void Layer::fillIn(const double* data)
{
	if (type == Input)
	{
		memcpy(value2, data, neural_amount * sizeof(double));
	}
	else {
		memcpy(value, data, neural_amount * sizeof(double));
		for (int i = 0; i < neural_amount; i++)
		{
			value2[i] = normolizeFunction(value[i]);
		}
	}
}
void Layer::clear()
{
	memset(value, 0, sizeof(double) * neural_amount);
	if (type != Input)
	{
		memset(value2, 0, sizeof(double) * neural_amount);
	}
}
Layer::Layer() {
	value = NULL;
	neural_amount = 0;
	type = Unknown;
}
Layer::~Layer()
{
	delete[] value;
	delete[] value2;
}


MLP::MLP(
	int input,
	int hidden_layer,
	int * hiddens,
	int output,
	const double * biases,
	const double *** table
)
{
	study_rate = 0.5;
	input_neural_amount = input;
	output_neural_amount = output;
	hidden_neural_amount = new int[hidden_layer];
	memcpy(hidden_neural_amount, hiddens, sizeof(int) * hidden_layer);
	hidden_layer_amount = hidden_layer;
	weight_layer_amount = hidden_layer + 1;
	this->biases = new double[weight_layer_amount];
	if (biases) memcpy(this->biases, biases, sizeof(double) * weight_layer_amount);
	else memset(this->biases, 0, sizeof(double) * weight_layer_amount);

	weights = new double**[weight_layer_amount];
	weights2 = new double**[weight_layer_amount];
	if (!table) {
		table = new const double**[weight_layer_amount];
		memset(table, 0, sizeof(double**)*weight_layer_amount);
	}
	malloc_weights_for(input_neural_amount, hiddens[0], weights[0], table[0]);
	malloc_weights_for(input_neural_amount, hiddens[0], weights2[0], table[0]);
	for (int i = 0; i < hidden_layer - 1; i++)
	{
		malloc_weights_for(hiddens[i], hiddens[i + 1], weights[i + 1], table[i + 1]);
		malloc_weights_for(hiddens[i], hiddens[i + 1], weights2[i + 1], table[i + 1]);
	}
	malloc_weights_for(hiddens[hidden_layer - 1], output_neural_amount, weights[hidden_layer], table[hidden_layer]);
	malloc_weights_for(hiddens[hidden_layer - 1], output_neural_amount, weights2[hidden_layer], table[hidden_layer]);

	input_layer = new Layer(Input, input_neural_amount);
	output_layer = new Layer(Output, output_neural_amount);
	hidden_layers = new Layer[hidden_layer];
	for (int i = 0; i < hidden_layer; i++)
	{
		hidden_layers[i] = Layer(Hidden, hiddens[i]);
	}

	sigmas = new double[output_neural_amount];
	memset(sigmas, 0, sizeof(double) * output_neural_amount);

	log_weights = fopen("log_weights.txt", "w");
	log_errors = fopen("log_errors.txt", "w");
}
void MLP::train(const double ** inputs, const double ** expected_outputs, int dataset_amount)
{
	double error = 0.0;
	for (int j = 0; j < 2000; j++)
	{
		for (int i = 0; i < dataset_amount; i++)
		{
			singleRun(inputs[i]);
			error = getError(expected_outputs[i]);
			calculateSigmas(expected_outputs[i]);
			lastWeightsCorrection();
			backRun();
			changeWeights2AndWeights();
			saveWeights();
			
			fprintf(log_errors, "%.5lf\t", error);
		}
		fprintf(log_errors, "\n");
	}
}
void MLP::saveWeights()
{
	for (int i = 0; i < input_neural_amount; i++)
	{
		for (int j = 0; j < hidden_neural_amount[0]; j++)
		{
			fprintf(log_weights, "%.2lf,", weights[0][i][j]);
		}
	}
	for (int i = 1; i < hidden_layer_amount; i++)
	{
		for (int j = 0; j < hidden_neural_amount[i]; j++)
		{
			for (int k = 0; k < hidden_neural_amount[i + 1]; k++)
			{
				fprintf(log_weights, "%.2lf,", weights[i][j][k]);
			}
		}
	}
	for (int i = 0; i < lastHidden()->neural_amount; i++)
	{
		for (int j = 0; j < output_neural_amount; j++)
		{
			fprintf(log_weights, "%.2lf,", weights[hidden_layer_amount][i][j]);
		}
	}
	fprintf(log_weights, "\n");
}
MLP::~MLP() {
	for (int i = 0; i < weight_layer_amount; i++)
	{
		delete[] weights[i];
		delete[] weights2[i];
	}
	delete[] weights;
	delete[] weights2;

	delete[] sigmas;

	input_layer->~Layer();
	output_layer->~Layer();
	for (int i = 0; i < hidden_layer_amount; i++)
	{
		hidden_layers[i].~Layer();
	}

	delete input_layer;
	delete output_layer;
	delete[] hidden_layers;
}

void MLP::changeWeights2AndWeights()
{
	double *** temp = weights2;
	weights2 = weights;
	weights = temp;
}
Layer * MLP::lastHidden()
{
	return hidden_layers + hidden_layer_amount - 1;
}
void MLP::lastWeightsCorrection()
{
	Layer * last = lastHidden();
	for (int i = 0; i < last->neural_amount; i++)
	{
		for (int j = 0; j < output_layer->neural_amount; j++)
		{
			weights2[hidden_layer_amount][i][j]
				= weights[hidden_layer_amount][i][j] - study_rate * sigmas[j] * last->value2[i];
		}
	}
}
void MLP::backRun()
{
	output_layer->fillAsInput(sigmas);
	executeLayer(output_layer, lastHidden(), weights[hidden_layer_amount], 0, true);

	for (int i = hidden_layer_amount - 2; i >= 0; i--)
	{
		for (int j = 0; j < hidden_layers[i + 1].neural_amount; j++)
		{
			for (int k = 0; k < hidden_layers[i].neural_amount; k++)
			{
				weights2[i + 1][k][j] =
					weights[i + 1][k][j] - study_rate * hidden_layers[i + 1].value2[j] * hidden_layers[i].value2[k];
			}
		}
		executeLayer(hidden_layers + i + 1, hidden_layers + i, weights[i + 1], 0, true);
	}
	for (int j = 0; j < input_layer->neural_amount; j++) {
		for (int k = 0; k < hidden_layers[0].neural_amount; k++)
		{
			weights2[0][j][k] =
				weights[0][j][k] - study_rate * hidden_layers[0].value2[k] * input_layer->value2[j];

		}
	}
	//executeLayer(hidden_layers, input_layer, weights[0], 0, true);

}
void MLP::calculateSigmas(const double * targets)
{
	double out = 0;
	double target = 0;
	double ob = 0;
	for (int i = 0; i < output_neural_amount; i++)
	{
		out = output_layer->value2[i];
		target = targets[i];
		sigmas[i] = (out - target) * out * (1 - out);
	}
}
void MLP::executeLayer(
	Layer * front, 
	Layer * back, 
	double ** weight, 
	double bias,
	bool inverse)
{
	if (!inverse)
	{
		back->clear();
		for (int i = 0; i < back->neural_amount; i++)
		{
			for (int j = 0; j < front->neural_amount; j++)
			{
				back->value[i] += front->value2[j] * weight[j][i];
			}
		}
		for (int i = 0; i < back->neural_amount; i++)
		{
			for (int j = 0; j < front->neural_amount; j++)
			{
				back->value2[i] = normolizeFunction(back->value[i]);
			}
		}
	}
	else {
		for (int i = 0; i < back->neural_amount; i++)
		{
			back->value[i] = back->value2[i] * (1 - back->value2[i]);
			back->value2[i] = 0;
			for (int j = 0; j < front->neural_amount; j++)
			{
				back->value2[i] += front->value2[j] * weight[i][j];
			}
			back->value2[i] *= back->value[i];
		}
	}
}
void MLP::singleRun(const double * input)
{
	input_layer->fillIn(input);
	executeLayer(
		input_layer,
		hidden_layers,
		weights[0],
		biases[0]
	);
	for (int i = 0; i < hidden_layer_amount - 1; i++)
	{
		executeLayer(
			hidden_layers + i,
			hidden_layers + i + 1,
			weights[i + 1],
			biases[i + 1]
		);
	}
	executeLayer(
		hidden_layers + hidden_layer_amount - 1,
		output_layer, weights[hidden_layer_amount],
		biases[hidden_layer_amount]
	);
}

