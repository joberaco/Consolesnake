#define MAX_CHARS 20

#define MAX_TRAINING_SAMPLES 2000
#define LEARNING_RATE (0.008)
#define EPOCHS 500000

#define MIN_NN_LAYERS 2

typedef struct {
    float **weightGradients;
    float *biasGradients;
    float *localGradients;
    float (*activationDerivative)(float x);
} BackpropagationData;

typedef struct {
    int size;
    int upstreamSize;
    float **weights;
    float *biases;
    float *in;
    float *out;
    float (*activation)(float x);
    BackpropagationData bpData;
} Layer;

typedef struct {
    float *inputs;
    int inputSize;
    float *outputs;
    int outputSize;
} TrainingVector;

Layer *nn, *input, *output;
int nlayers;

float relu(float x){
    return x > 0 ? x : 0;
}

float reluDerivative(float x){
    return x > 0 ? 1 : 0;
}

float step(float x){
    return x > 0 ? 1 : 0;
}

int sign(int x){
    if(x > 0) return 1;
    else if(x < 0) return -1;
    else return 0;
}

int signF(float x){
    if(x > 0) return 1;
    else if(x < 0) return -1;
    else return 0;
}

float sigmoid(float x){
    return 1 / (1 + expf(-x));
}

float sigmoidDerivative(float x){
    return sigmoid(x) * (1 - sigmoid(x));
}

float tanH(float x){
    return (expf(x) - expf(-x)) / (expf(x) + expf(-x));
}

float tanHDerivative(float x){
    return 1 - (tanH(x)*tanH(x));
}

int maxOutput(){
    int i, m = 0;

    for(i = 0; i < output->size; i++){
        m = output->out[i] >= output->out[m] ? i : m;
    }

    return m;
}

void initLayer(Layer *layer, int size, int upstreamSize, float (*activation)(float x), float (*activationDerivative)(float x))
{
    int i;

    layer->size = size;

    layer->out = (float*)malloc(size * sizeof(float));
    memset(layer->out, 0, size * sizeof(float));

    if(activation != NULL) /*IF WEIGHTED LAYER (NON-INPUT)*/
    {
        layer->upstreamSize = upstreamSize;

        layer->weights = (float**)malloc(size * sizeof(float*));
        layer->bpData.weightGradients = (float**)malloc(size * sizeof(float*));

        for(i = 0; i < size; i++){
            layer->weights[i] = (float*)malloc(upstreamSize * sizeof(float));
            layer->bpData.weightGradients[i] = (float*)malloc(upstreamSize * sizeof(float));

            memset(layer->weights[i], 0, upstreamSize * sizeof(float));
            memset(layer->bpData.weightGradients[i], 0, upstreamSize * sizeof(float));
        }

        layer->biases = (float*)malloc(size * sizeof(float));
        layer->bpData.biasGradients = (float*)malloc(size * sizeof(float));

        memset(layer->biases, 0, size * sizeof(float));
        memset(layer->bpData.biasGradients, 0, size * sizeof(float));

        layer->in = (float*)malloc(size * sizeof(float));
        memset(layer->in, 0, size * sizeof(float));

        layer->activation = activation;
        layer->bpData.activationDerivative = activationDerivative;

        layer->bpData.localGradients = (float*)malloc(size * sizeof(float));
        memset(layer->bpData.localGradients, 0, size * sizeof(float));
    }
}

void initTrainingVector(TrainingVector *tvector)
{
    tvector->inputSize = input->size;
    tvector->outputSize = output->size;

    tvector->inputs = malloc(sizeof(float) * tvector->inputSize);
    tvector->outputs = malloc(sizeof(float) * tvector->outputSize);
}

void freeTrainingVector(TrainingVector *tvector)
{
    free(tvector->inputs);
    free(tvector->outputs);
    tvector->inputs = NULL;
    tvector->outputs = NULL;
}

void randomizeParameters(Layer *layer)
{
    int i, j;

    for(i = 0; i < layer->size; i++){
        for(j = 0; j < layer->upstreamSize; j++){
            layer->weights[i][j] = (float)rand()/(float)(RAND_MAX);
        }
    }

    for(i = 0; i < layer->size; i++){
        layer->biases[i] =  (float)rand()/(float)(RAND_MAX);
    }
}

int initNN(int *layers, int size)
{
    int i, j;

    nlayers = size;
    if(nlayers < MIN_NN_LAYERS) return -1;

    nn = (Layer*)malloc(sizeof(Layer) * nlayers);

    /*INPUT*/
    initLayer(&nn[0], layers[0], 0, NULL, NULL);
    input = &nn[0];

    /*HIDDEN*/
    for(i = 1; i < nlayers-1; i++){
        initLayer(&nn[i], layers[i], layers[i-1], sigmoid, sigmoidDerivative);
        randomizeParameters(&nn[i]);
    }

    /*OUTPUT*/
    initLayer(&nn[nlayers-1], layers[nlayers-1], layers[nlayers-2], sigmoid, sigmoidDerivative);
    randomizeParameters(&nn[nlayers-1]);
    output = &nn[nlayers-1];

    return 0;
}

void printLayer(Layer layer, const char label[MAX_CHARS])
{
    int i, j;

    for(i = 0; i < layer.size; i++)
    {
        printf("%s NODE %d (a%d=%0.2f) ", label, i, i, layer.out[i]);

        if(layer.upstreamSize > 0){
            printf("= { ");
            for(int j = 0; j < layer.upstreamSize; j++){
                printf("w%d=%0.1f\t", j, layer.weights[i][j]);
            }

            printf("b=%0.1f ", layer.biases[i]);
            printf("}");
        }
        printf("\n");
    }
    printf("\n");
}

void debugNN()
{
    int i, j;

    printLayer(*input, "INPUT");

    for(i = 1; i < nlayers-1; i++){
        printLayer(nn[i], "HIDDEN");
    }

    printLayer(*output, "OUTPUT");
}

void debugNNGraph()
{
    char render[15][50];
    int i, l, x, y;

    for(y = 0; y < 15; y++)
        for(x = 0; x < 50; x++)
            render[y][x] = ' ';

    for(i = 0, y = 0, x = 0; i < input->size; i++){
        render[y+i+1][x] = 'I';
    }
    
    for(l = 1, x = 3; l < nlayers-1; l++, x+=2){
        for(i = 0, y = 0; i < nn[l].size; i++){
            render[y+i+1][x+1] = 'H';
        }
    }

    for(i = 0, y = 0, x+=3; i < output->size; i++){
        render[y+i+1][x] = 'O';
    }

    for(x = 0; x < 15; x++){
        for(y = 0; y < 50; y++){
            printf("%c", render[x][y]);
        }
        printf("\n");
    }

}

void printTrainingSamples(int nSamples, TrainingVector *samples)
{
    int i, j;

    printf("Training Samples [%d] =\n", nSamples);

    for(i = 0; i < nSamples; i++){
        printf("[%d]{ ", i);
        for(j = 0; j < samples[i].inputSize; j++){
            printf("%0.3f ", samples[i].inputs[j]);
        }
        printf("}, { ");
        for(j = 0; j < samples[i].outputSize; j++){
            printf("%0.0f ", samples[i].outputs[j]);
        }
        printf("},\n");
    }

    printf("}\n");
}

void feedForward(Layer from, Layer *to)
{
    int i, j;

    /*weightedSum[rows] = x[cols] * w[rows][cols]*/
    for(i = 0; i < to->size; i++){
        to->in[i] = 0;
        for(j = 0; j < from.size; j++){
            to->in[i] += from.out[j] * to->weights[i][j];
        }

        to->in[i] += to->biases[i];
        to->out[i] = to->activation(to->in[i]);
    }
}

int predict(float *data)
{
    int i, max;

    memcpy(input->out, data, input->size * sizeof(float));

    for(i = 0; i < nlayers-1; i++){
        feedForward(nn[i], &nn[i+1]);
    }

    max = maxOutput();
    return max;
}

float nodeCost(float node, float expected)
{
    float err = node - expected;
    return err * err;
}

float nodeCostDerivative(float node, float expected)
{
    return 2 * (node - expected);
}

float cost(float *input, float *expected)
{
    int i;
    float costSum = 0;

    /*feed forward*/
    predict(input);

    /*cost sum*/
    for(i = 0; i < output->size; i++){
        costSum += nodeCost(output->out[i], expected[i]);

        /*calculate cost derivative in advance, fully updated in updateGradients()*/
        output->bpData.localGradients[i] = 
            nodeCostDerivative(output->out[i], expected[i]) * output->bpData.activationDerivative(output->in[i]);
    }

    return costSum;
}

void applyGradients(Layer *layer)
{
    int i, j;

    for(i = 0; i < layer->size; i++)
    {
        layer->biases[i] -= layer->bpData.biasGradients[i] * LEARNING_RATE;
        layer->bpData.biasGradients[i] = 0;
        for(j = 0; j < layer->upstreamSize; j++)
        {
            layer->weights[i][j] -= layer->bpData.weightGradients[i][j] * LEARNING_RATE;
            layer->bpData.weightGradients[i][j] = 0;
        }
    }
}

void updateGradients(Layer *layer, Layer upstream)
{
    int i, j;
    
    for(i = 0; i < layer->size; i++)
    {
        for(j = 0; j < layer->upstreamSize; j++)
        {
            layer->bpData.weightGradients[i][j] += upstream.out[j] * layer->bpData.localGradients[i];
        }

        layer->bpData.biasGradients[i] += 1 * layer->bpData.localGradients[i];
    }    
}

void backPropagate()
{
    int i, j, l;
    
    /*Output gradients update*/
    updateGradients(output, nn[nlayers-2]);

    Layer downstream = *output;
    /*Hidden layers local gradient & weight gradients update*/
    for(l = nlayers-2; l >= 1 ; l--)
    {
        for(i = 0; i < nn[l].size; i++)
        {
            for(j = 0; j < downstream.size; j++)
            {
                nn[l].bpData.localGradients[i] += downstream.weights[j][i] * downstream.bpData.localGradients[j];
            }

            nn[l].bpData.localGradients[i] *= nn[l].bpData.activationDerivative(nn[l].in[i]);
        }

        updateGradients(&nn[l], nn[l-1]);
        downstream = nn[l];
    }
}

void train(int nSamples, TrainingVector* samples)
{
    int i, e, dir;
    float costSum, acc;
    
    for(e = 0; e < EPOCHS; e++){
        for(i = 0, costSum = 0; i < nSamples; i++)
        {
            costSum += cost(samples[i].inputs, samples[i].outputs);
            backPropagate();
        }

        for(i = 1; i < nlayers; i++){
            applyGradients(&nn[i]);
        }

        printf("\tTRAINING COST [%i] = %0.2f\n", e, (costSum/nSamples));
    }
    
    printf("\tTRAINING COST [%i] = %0.2f\n", e, (costSum/nSamples));
    for(i = 0, acc = 0; i < nSamples; i++)
    {
        dir = predict(samples[i].inputs);

        /*debugNN();
        system("pause");*/

        output->out = samples[i].outputs;
        if(dir == maxOutput()) acc++;

    }

    printf("\n***TEST ACCURACY = %0.2f%% (%0.0f/%i)\n", (acc/nSamples)*100, acc, nSamples);
}