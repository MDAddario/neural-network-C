#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <byteswap.h>
#define FILENAME "neural_network_parameters.txt"

/*
Reference: http://neuralnetworksanddeeplearning.com/chap1.html
*/

// Structure for neural network
typedef struct structNetwork{

	int num_layers;
	int* sizes;
	double** biases;
	double*** weights;

} Network;

// Generate uniform random float between -1 and 1
double randFloat(){

	double value = rand();
	return 2.0*value/(double)RAND_MAX - 1.0;
}

// Allocate memory and initialize random parameters in NN
Network* initNetwork(int num_layers, int sizes[]){

	// Allocate memory for struct
	Network* network = (Network*)malloc(sizeof(Network));

	// Set num layers
	network->num_layers = num_layers;

	// Allocate memory for size array, read values
	network->sizes = (int*)malloc(num_layers*sizeof(int));
	for(int i = 0; i < num_layers; i++)
		network->sizes[i] = sizes[i];

	// Allocate memory for biases, fill with random values
	network->biases = (double**)malloc(num_layers*sizeof(double*));
	for(int i = 1; i < num_layers; i++){
		network->biases[i] = (double*)malloc(sizes[i]*sizeof(double));
		for(int j = 0; j < sizes[i]; j++)
			network->biases[i][j] = randFloat();
	}

	// Allocate memory for weights, fill with random values
	network->weights = (double***)malloc(num_layers*sizeof(double**));
	for(int i = 0; i < num_layers-1; i++){
		network->weights[i] = (double**)malloc(sizes[i+1]*sizeof(double*));
		for(int j = 0; j < sizes[i+1]; j++){
			network->weights[i][j] = (double*)malloc(sizes[i]*sizeof(double));
			for(int k = 0; k < sizes[i]; k++)
				network->weights[i][j][k] = randFloat();
		}
	}
	return network;
}

// Save parameters of NN to file
void saveNetwork(Network* network, char* filename){

	// Open file
	FILE* fptr = fopen(filename, "w+");
	if (!fptr){
		printf("File %s not found.\n", filename);
		return;
	}

	// Save num_layers
	fprintf(fptr, "num_layers = %d\n\n", network->num_layers);

	// Save sizes
	fprintf(fptr, "sizes = ");
	for(int i = 0; i < network->num_layers; i++)
		fprintf(fptr, "%d ", network->sizes[i]);
	fprintf(fptr, "\n\n");

	// Save biases
	fprintf(fptr, "biases = \n\n");
	for(int i = 1; i < network->num_layers; i++){
		for(int j = 0; j < network->sizes[i]; j++)
			fprintf(fptr, "%lf ", network->biases[i][j]);
		fprintf(fptr, "\n\n");
	}

	// Save weights
	fprintf(fptr, "weights = \n\n");
	for(int i = 0; i < network->num_layers-1; i++){
		for(int j = 0; j < network->sizes[i+1]; j++){
			for(int k = 0; k < network->sizes[i]; k++)
				fprintf(fptr, "%lf ", network->weights[i][j][k]);
			fprintf(fptr, "\n");
		}
		fprintf(fptr, "\n");
	}

	fclose(fptr);
	return;
}

// Allocate memory and load parameters to NN
Network* loadNetwork(Network* network, char* filename){

	// Open file
	FILE* fptr = fopen(filename, "r");
	if (!fptr){
		printf("File %s not found.\n", filename);
		return NULL;
	}

	// Allocate memory for struct
	network = (Network*)malloc(sizeof(Network));

	// Load num_layers
	fscanf(fptr, "num_layers = %d\n\n", &(network->num_layers));

	// Allocate mem, load sizes
	network->sizes = (int*)malloc(network->num_layers*sizeof(int));
	fscanf(fptr, "sizes = ");
	for(int i = 0; i < network->num_layers; i++)
		fscanf(fptr, "%d ", &(network->sizes[i]));
	fscanf(fptr, "\n\n");

	// Allocate mem, load biases
	network->biases = (double**)malloc(network->num_layers*sizeof(double*));
	fscanf(fptr, "biases = \n\n");
	for(int i = 1; i < network->num_layers; i++){
		network->biases[i] = (double*)malloc(network->sizes[i]*sizeof(double));
		for(int j = 0; j < network->sizes[i]; j++)
			fscanf(fptr, "%lf ", &(network->biases[i][j]));
		fscanf(fptr, "\n\n");
	}

	// Allocate mem, load weights
	network->weights = (double***)malloc(network->num_layers*sizeof(double**));
	fscanf(fptr, "weights = \n\n");
	for(int i = 0; i < network->num_layers-1; i++){
		network->weights[i] = (double**)malloc(network->sizes[i+1]*sizeof(double*));
		for(int j = 0; j < network->sizes[i+1]; j++){
			network->weights[i][j] = (double*)malloc(network->sizes[i]*sizeof(double));
			for(int k = 0; k < network->sizes[i]; k++)
				fscanf(fptr, "%lf ", &(network->weights[i][j][k]));
			fscanf(fptr, "\n");
		}
		fscanf(fptr, "\n");
	}

	fclose(fptr);
	return network;
}

// Squishification function
double sigmoid(double z){

	return 1.0 / (1.0 + exp(-z));
}

// Forward propagate, retrieve output from input
double* feedForward(Network* network, double* input){

	int size_from, size_to;
	double* activation_from;
	double* activation_to;

	for(int i = 0; i < network->num_layers-1; i++){

		size_from = network->sizes[i];
		size_to = network->sizes[i+1];

		if (i == 0){
			activation_from = (double*)malloc(size_from*sizeof(double));
			memcpy(activation_from, input, size_from*sizeof(double));
			activation_to = (double*)malloc(size_to*sizeof(double));
		}
		else{
			activation_from = (double*)realloc(activation_from, size_from*sizeof(double));
			memcpy(activation_from, activation_to, size_from*sizeof(double));
			activation_to = (double*)realloc(activation_to, size_to*sizeof(double));
		}

		for(int j = 0; j < size_to; j++){
			activation_to[j] = 0;

			for(int k = 0; k < size_from; k++)
				activation_to[j] += network->weights[i][j][k] * activation_from[k];

			activation_to[j] += network->biases[i+1][j];
			activation_to[j] = sigmoid(activation_to[j]);
		}
	}
	free(activation_from);
	return activation_to;
}

// Determine the gradient of the NN
void backPropagation(Network* network, double* input, double* output,
					double*** delta_nabla_biases, double**** delta_nabla_weights){

	for(int i = 1; i < network->num_layers; i++)
		for(int j = 0; j < network->sizes[i]; j++)
			delta_nabla_biases[0][i][j] = 0;
	
	for(int i = 0; i < network->num_layers-1; i++)
		for(int j = 0; j < network->sizes[i+1]; j++)
			for(int k = 0; k < network->sizes[i]; k++)
				delta_nabla_weights[0][i][j][k] = 0;

	return;
}

/*
CONSIDER PREALLOCATING MEMORY FOR ALL THE NABLAS AND DELTAS IN THE STRUCT AS TO AVOID
HAVING TO CALLOC AND MALLOC AND FREE EVERY TIME
*/

// Applying SGD for the mini batch
void update_mini_batch(Network* network, double** mini_batch_input, 
					double** mini_batch_output, int mini_batch_size,
					double learning_rate){

	double** nabla_biases;
	double*** nabla_weights;
	double** delta_nabla_biases;
	double*** delta_nabla_weights;

	// Callocate memory
	nabla_biases = (double**)calloc(network->num_layers, sizeof(double*));
	for(int i = 1; i < network->num_layers; i++)
		nabla_biases[i] = (double*)calloc(network->sizes[i], sizeof(double));

	nabla_weights = (double***)calloc(network->num_layers, sizeof(double**));
	for(int i = 0; i < network->num_layers-1; i++){
		nabla_weights[i] = (double**)calloc(network->sizes[i+1], sizeof(double*));
		for(int j = 0; j < network->sizes[i+1]; j++)
			nabla_weights[i][j] = (double*)calloc(network->sizes[i], sizeof(double));
	}

	// Malloc memory
	delta_nabla_biases = (double**)malloc(network->num_layers*sizeof(double*));
	for(int i = 1; i < network->num_layers; i++)
		delta_nabla_biases[i] = (double*)malloc(network->sizes[i]*sizeof(double));

	delta_nabla_weights = (double***)malloc(network->num_layers*sizeof(double**));
	for(int i = 0; i < network->num_layers-1; i++){
		delta_nabla_weights[i] = (double**)malloc(network->sizes[i+1]*sizeof(double*));
		for(int j = 0; j < network->sizes[i+1]; j++)
			delta_nabla_weights[i][j] = (double*)malloc(network->sizes[i]*sizeof(double));
	}

	// Increment nabla for each vector in mini_batch
	for(int a = 0; a < mini_batch_size; a++){

		backPropagation(network, mini_batch_input[a], mini_batch_output[a], &delta_nabla_biases, &delta_nabla_weights);

		for(int i = 1; i < network->num_layers; i++)
			for(int j = 0; j < network->sizes[i]; j++)
				nabla_biases[i][j] += delta_nabla_biases[i][j];
		
		for(int i = 0; i < network->num_layers-1; i++)
			for(int j = 0; j < network->sizes[i+1]; j++)
				for(int k = 0; k < network->sizes[i]; k++)
					nabla_weights[i][j][k] += delta_nabla_weights[i][j][k];
	}

	// Update weights and biases from total nabla_biases and total nabla_weights
	for(int i = 1; i < network->num_layers; i++)
		for(int j = 0; j < network->sizes[i]; j++)
			network->biases[i][j] -= delta_nabla_biases[i][j] * learning_rate / mini_batch_size;
	
	for(int i = 0; i < network->num_layers-1; i++)
		for(int j = 0; j < network->sizes[i+1]; j++)
			for(int k = 0; k < network->sizes[i]; k++)
				network->weights[i][j][k] -= delta_nabla_weights[i][j][k] * learning_rate / mini_batch_size;

	// Free the memory
	for(int i = 1; i < network->num_layers; i++)
		free(nabla_biases[i]);
	free(nabla_biases);

	for(int i = 0; i < network->num_layers-1; i++){
		for(int j = 0; j < network->sizes[i+1]; j++)
			free(nabla_weights[i][j]);
		free(nabla_weights[i]);
	}
	free(nabla_weights);

	for(int i = 1; i < network->num_layers; i++)
		free(delta_nabla_biases[i]);
	free(delta_nabla_biases);

	for(int i = 0; i < network->num_layers-1; i++){
		for(int j = 0; j < network->sizes[i+1]; j++)
			free(delta_nabla_weights[i][j]);
		free(delta_nabla_weights[i]);
	}
	free(delta_nabla_weights);

	return;
}

/*
CONSIDER A REAL SHUFFLE FOR THE MINI BATCHES
IMPLEMENT OPTIONAL TEST DATA AT END OF EVERY EPOCH
*/

/*
MADE CODE MORE C LIKE
LESS LIKE PYTHON
PLEASE
*/

// Training algorithm
void stochasticGradientDescent(Network* network, double** training_input, 
					double** training_output, int training_size, int mini_batch_size, 
					int epochs, double learning_rate){

	if (training_size % mini_batch_size != 0){
		printf("Mini batch size must evenly divide the training set size.\n");
		return;
	}

	// Allocate mini_batch memory
	double** mini_batch_input = (double**)malloc(mini_batch_size*sizeof(double*));
	double** mini_batch_output = (double**)malloc(mini_batch_size*sizeof(double*));

	for(int i = 0; i < mini_batch_size; i++){
		mini_batch_input[i] = (double*)malloc(network->sizes[0]*sizeof(double));
		mini_batch_output[i] = (double*)malloc(network->sizes[network->num_layers-1]*sizeof(double));
	}

	// Repeat process epoch times
	for(int i = 0; i < epochs; i++){

		int index = rand() % training_size;

		// Number of minibatches per epoch
		for(int j = 0; j < training_size/mini_batch_size; j++){

			// Size of minibatch
			for(int k = 0; k < mini_batch_size; k++){

				if (index == training_size)
					index = 0;

				// Setup minibatches
				memcpy(mini_batch_input[k], training_input[index], network->sizes[0]*sizeof(double));
				memcpy(mini_batch_output[k], training_output[index], network->sizes[network->num_layers-1]*sizeof(double));
				index++;
				
				// Apply the SGD
				update_mini_batch(network, mini_batch_input, mini_batch_output, mini_batch_size, learning_rate);
			}
		}
		printf("Epoch %d complete.\n", i+1);
	}

	// Free mini_batch memory
	for(int i = 0; i < mini_batch_size; i++){
		free(mini_batch_input[i]);
		free(mini_batch_output[i]);
	}
	free(mini_batch_input);
	free(mini_batch_output);

	return;
}

// The main attraction
int main(){

	/*
	int num_layers = 3;
	int sizes[] = {4, 3, 2};

	Network* network = initNetwork(num_layers, sizes);
	saveNetwork(network, FILENAME);
	*/

	/*
	double input_vector[4] = {4.0, 3.0, 2.5, 3.4};
	double* output = feedForward(network, input_vector);

	printf("Final layer activation values:\n");
	for(int i = 0; i < network->sizes[network->num_layers-1]; i++)
		printf("%d: %lf\n", i+1, output[i]);

	free(output);
	*/

	srand(time(NULL));

	Network* network = loadNetwork(network, FILENAME);

	int training_size = 400;
	int mini_batch_size = 20;
	int epochs = 30;
	double learning_rate = 4.0;

	double** training_input = (double**)malloc(training_size*sizeof(double*));
	for(int i = 0; i < training_size; i++){
		training_input[i] = (double*)malloc(network->sizes[0]*sizeof(double));
		for(int j = 0; j < network->sizes[0]; j++)
			training_input[i][j] = i+1;
	}

	double** training_output = (double**)malloc(training_size*sizeof(double*));
	for(int i = 0; i < training_size; i++){
		training_output[i] = (double*)malloc(network->sizes[network->num_layers-1]*sizeof(double));
		for(int j = 0; j < network->sizes[network->num_layers-1]; j++)
			training_output[i][j] = i+1;
	}

	stochasticGradientDescent(network, training_input, training_output, 
				training_size, mini_batch_size, epochs, learning_rate);

	for(int i = 0; i < training_size; i++){
		free(training_input[i]);
		free(training_output[i]);
	}
	free(training_input);
	free(training_output);
	free(network);

	return 0;
}


// MY FUNCTIONS THAT AREN'T BEING USED

/*
// Elementwise sigmoid operation on (Ax1)
double* elementwiseSigmoid(double* z, int dim_A){

	double* output = (double*)malloc(dim_A*sizeof(double));

	for(int i = 0; i < dim_A; i++)
		output[i] = sigmoid(z[i]);
	
	return output;
}

// Matrix multiplication for (AxB) times (Bx1)
double* matVecProd(double** matrix, double* vector, int dim_A, int dim_B){

	double* product = (double*)malloc(dim_A*sizeof(double));

	for(int i = 0; i < dim_A; i++){
		product[i] = 0;

		for(int j = 0; j < dim_B; j++)
			product[i] += matrix[i][j] * vector[j];
	}
	return product;
}

// Vector sum (aX1)
double* vecVecSum(double* vector_1, double* vector_2, int dim_A){

	double* sum = (double*)malloc(dim_A*sizeof(double));

	for(int i = 0; i < dim_A; i++)
		sum[i] = vector_1[i] + vector_2[i];
	
	return sum;
}
*/


/*
FILE* images = fopen("train-images-idx3-ubyte.gz", "rb");
if (!images){
	perror("Could not open the images\n");
	return -1;
}

FILE* labels = fopen("train-labels-idx1-ubyte.gz", "rb");
if (!labels){
	perror("Could not open the labels\n");
	return -1;
}
*/