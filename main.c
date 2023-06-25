#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <conio.h>
#include <Windows.h>

#include "nn.h"

#define RENDERER_W 50
#define RENDERER_H 15
#define RENDERER_SIZE (RENDERER_W*RENDERER_H)
#define SLEEP_MS 100

#define SNAKE_MAX_SIZE 20
#define SNAKE_MIN_SIZE 5
#define SNAKE_SPEED 1

typedef struct {
    int x, y;
} Point;

typedef struct {
    char head_sym;
    char body_sym;
    int size;
    Point pos;
    Point* body;
    Point dir;
    bool dead;
} Snake;

typedef struct {
    char symbol;
    Point pos;
} Fruit;

enum {
    KEY_UP = 'w',
    KEY_LEFT = 'a',
    KEY_DOWN = 's',
    KEY_RIGHT = 'd'
};

enum {
    DIR_UP,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT
};

/*NN CONSTANTS*/

#define N_LAYERS 3

#define INPUT_VECTOR_SIZE 3
#define HIDDEN_LAYOUT 5
#define OUTPUT_VECTOR_SIZE 4

#define TEST_MODE 1

/*NN CONSTANTS*/

void init(char** renderer);
void clear(char* renderer);
void present(char* renderer);

void getKbInput(Snake* snake);
void getTrainingKbInput(Snake* snake, Fruit fruit);
void getNNInput(Snake* snake, Fruit fruit);
void getRandInput(Snake* snake);

void drawPoint(char* renderer, char symbol, int x, int y);
void drawLine(char* renderer, char symbol, int x1, int y1, int x2, int y2);
void drawCircle(char* renderer, char symbol, int xc, int yc, int r);

int inBounds(int x, int y);
void swap(int* a, int* b);
int cmpPoints(Point a, Point b);

Snake newSnake(char symbol, int maxsize, Point startP, Point startDir);
Fruit newFruit(char symbol, Point startP);
void renderSnake(char* renderer, Snake* snake);
void renderFruit(char* renderer, Fruit fruit);
void moveSnake(Snake* snake);
void feed(Snake* snake);
void relocateFruit(Fruit* fruit);

void loadTraining();
void saveTraining();
void setTrainingGameState(Snake snake, Fruit fruit, TrainingVector *tv);
void dir2OutputVector(int dir, TrainingVector *tv);
int outputVector2Dir(TrainingVector tv);

char* renderer;

/*NN Globals*/
int layers[N_LAYERS] = {INPUT_VECTOR_SIZE, HIDDEN_LAYOUT, OUTPUT_VECTOR_SIZE};
TrainingVector samples[MAX_TRAINING_SAMPLES];
int nsamples = 0;
/*NN Globals*/

int main()
{
    Snake snake = newSnake('#', SNAKE_MAX_SIZE, (Point){5,0}, (Point){1, 0});
    Fruit fruit = newFruit('+', (Point){15, 5});
    int i;
    
    init(&renderer);
    srand(time(NULL));

    if(initNN(layers, N_LAYERS) < 0){
        printf("Failed to init NN\n");
        system("Pause");
        return -1;
    }

#ifdef TEST_MODE
    loadTraining();
    printTrainingSamples(nsamples, samples);
    debugNN();
    system("pause");

    train(nsamples, samples);
    system("pause");
#endif /* TEST_MODE */

    while(1)
    {
        clear(renderer);

        if(!snake.dead)
        {   
        #ifdef TEST_MODE
            getNNInput(&snake, fruit);
        #else
            getTrainingKbInput(&snake, fruit);
        #endif /* TEST_MODE */

            //getKbInput(&snake);
            //getRandInput(&snake);

            for(i = 0; i < SNAKE_SPEED; i++)
            {
                moveSnake(&snake);

                if(cmpPoints(snake.pos, fruit.pos) == 0)
                {
                    feed(&snake);
                    relocateFruit(&fruit);
                }
            }
        }
        else
        {
        #ifndef TEST_MODE
            printTrainingSamples(nsamples, samples);
            system("pause");
            saveTraining();
        #endif /* NOT TEST_MODE */

            printf("**********************************DEAD**********************************");
            system("pause");
            break;
        }

        renderSnake(renderer, &snake);
        renderFruit(renderer, fruit);
        drawCircle(renderer, '*', snake.pos.x, snake.pos.y, 2);

        present(renderer);

    #ifdef TEST_MODE
        printLayer(*input, "IN");
        printLayer(*output, "OUT");
    #endif

        Sleep(SLEEP_MS);
    }

    return 0;
}

void init(char** renderer)
{ 
    *renderer = (char*)malloc(sizeof(char) * RENDERER_SIZE);
}

void clear(char* renderer)
{
    int i;

    for(i = 0; i < RENDERER_SIZE; i++)
    {
        renderer[i] = ' ';
    }
}

void drawPoint(char* renderer, char symbol, int x, int y)
{
    if(inBounds(x,y))
    {
        renderer[(y*RENDERER_W) + x] = symbol;
    }
}

void drawLine(char* renderer, char symbol, int x1, int y1, int x2, int y2)
{
    int x, y;
    int dx, dy, d = 0, step = 1;
    float m;

    m = (float)(y2-y1)/(x2-x1);
    dx = abs(x2-x1);
    dy = abs(y2-y1);

    if(m < 0) step = -1;

    if(fabsf(m) <= 1)
    {
        y = y1;
        if(x1 > x2){
            swap(&x1, &x2);
            y = y2;
        }

        for(x = x1; x <= x2; x++)
        {
            drawPoint(renderer, symbol, x, y);
            d += dy;
            if(d*2 >= dx){
                y+=step;
                d -= dx;
            }
        }
    }
    else
    {
        x = x1;
        if(y1 > y2){
            swap(&y1, &y2);
            x = x2;
        }

        for(y = y1; y <= y2; y++)
        {
            drawPoint(renderer, symbol, x, y);
            d += dx;
            if(d*2 >= dy){
                x+=step;
                d -= dy;
            }
        }
    }
}

void drawCircle(char* renderer, char symbol, int xc, int yc, int r)
{
    int x = r, y = 0;
    int e = 3 - 2*r;

    while(x >= y)
    {
        /*OCTANTS*/
        drawPoint(renderer, symbol, xc+x, yc+y);
        drawPoint(renderer, symbol, xc+x, yc-y);
        drawPoint(renderer, symbol, xc+y, yc+x);
        drawPoint(renderer, symbol, xc-y, yc+x);
        drawPoint(renderer, symbol, xc-x, yc+y);
        drawPoint(renderer, symbol, xc-x, yc-y);
        drawPoint(renderer, symbol, xc+y, yc-x);
        drawPoint(renderer, symbol, xc-y, yc-x);
        
        if(e > 0){
            e += 2*(5 - 2*x + 2*y);
            x--;
        }
        else{
            e += 2*(3 + 2*y);
        }

        y++;
    }
}


void present(char* renderer)
{
    int i;

    system("cls");
    for(i = 1; i <= RENDERER_SIZE; i++)
    {
        printf("%c", renderer[i-1]);
        if(i%RENDERER_W == 0) printf("\n");
    }
}

void getKbInput(Snake* snake)
{
    if(kbhit())
    {
        char key = getch();

        if(snake->dir.y == 0){
            switch((int)key){
                case KEY_UP:
                    snake->dir.x = 0;
                    snake->dir.y = -1;
                    break;
                case KEY_DOWN:
                    snake->dir.x = 0;
                    snake->dir.y = 1;
                    break;
            }
        }

        if(snake->dir.x == 0){
            switch((int)key){
                case KEY_RIGHT:
                    snake->dir.x = 1;
                    snake->dir.y = 0;
                    break;
                case KEY_LEFT:
                    snake->dir.x = -1;
                    snake->dir.y = 0;
                    break;
            }
        }
    }
}

void getTrainingKbInput(Snake* snake, Fruit fruit)
{
    int expected = -1;
    Snake recordSnake = *snake;

    if(kbhit())
    {
        char key = getch();

        if(snake->dir.y == 0){
            switch((int)key){
                case KEY_UP:
                    snake->dir.x = 0;
                    snake->dir.y = -1;
                    expected = DIR_UP;
                    break;
                case KEY_DOWN:
                    snake->dir.x = 0;
                    snake->dir.y = 1;
                    expected = DIR_DOWN;
                    break;
            }
        }
        else if(snake->dir.x == 0){
            switch((int)key){
                case KEY_RIGHT:
                    snake->dir.x = 1;
                    snake->dir.y = 0;
                    expected = DIR_RIGHT;
                    break;
                case KEY_LEFT:
                    snake->dir.x = -1;
                    snake->dir.y = 0;
                    expected = DIR_LEFT;
                    break;
            }
        }

        //FEED TRAINING DATA
        if(expected >= 0)
        {
            initTrainingVector(&samples[nsamples]);
            setTrainingGameState(recordSnake, fruit, &samples[nsamples]);
            dir2OutputVector(expected, &samples[nsamples]);
            nsamples++;
        }
    }
}

void getNNInput(Snake* snake, Fruit fruit)
{
    TrainingVector data;

    initTrainingVector(&data);
    setTrainingGameState(*snake, fruit, &data);

    int dirIndex = predict(data.inputs);

    if(snake->dir.y == 0){
        switch(dirIndex){
            case DIR_UP:
                snake->dir.x = 0;
                snake->dir.y = -1;
                break;
            case DIR_DOWN:
                snake->dir.x = 0;
                snake->dir.y = 1;
                break;
        }
    }
    else if(snake->dir.x == 0){
        switch(dirIndex){
            case DIR_RIGHT:
                snake->dir.x = 1;
                snake->dir.y = 0;
                break;
            case DIR_LEFT:
                snake->dir.x = -1;
                snake->dir.y = 0;
                break;
        }
    }

    freeTrainingVector(&data);
}

void getRandInput(Snake* snake)
{
    /*if(yspeed == 0 && (rand()%10 == 0)){
        xspeed = 0;
        yspeed = (rand()%2 == 0 ? -1 : 1) * SNAKE_SPEED;
    }
    else if(xspeed == 0 && (rand()%10 == 0)){
        yspeed = 0;
        xspeed = (rand()%2 == 0 ? -1 : 1) * SNAKE_SPEED;
    }

    if(snake.pos.x >= RENDERER_W){
        snake.pos.x--;
        xspeed = 0;
        yspeed = SNAKE_SPEED;
    }
    else if(snake.pos.y >= RENDERER_H){
        snake.pos.y--;
        xspeed = -SNAKE_SPEED;
        yspeed = 0;
    }
    else if(snake.pos.x < 0){
        snake.pos.x++;
        xspeed = 0;
        yspeed = -SNAKE_SPEED;
    }
    else if(snake.pos.y < 0){
        snake.pos.y++;
        xspeed = SNAKE_SPEED;
        yspeed = 0;
    }*/
}

int inBounds(int x, int y)
{
    return (x >= 0 && x < RENDERER_W) && (y >= 0 && y < RENDERER_H);
}

void swap(int* a, int* b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int cmpPoints(Point a, Point b)
{
    if(a.x == b.x && a.y == b.y) return 0;
    else return -1;
}

Snake newSnake(char symbol, int maxsize, Point startP, Point startDir)
{
    Snake snake;
    int i;

    snake = (Snake){symbol, '~', SNAKE_MIN_SIZE, startP, (Point*)malloc(maxsize*sizeof(Point)), startDir, false};

    for(i = 0; i < SNAKE_MIN_SIZE; i++){
        snake.body[i].x = startP.x - i;
        snake.body[i].y = startP.y;
    }

    return snake;
}

Fruit newFruit(char symbol, Point startP)
{
    if(startP.x < 0 || startP.x > RENDERER_W){
        startP.x = rand() % RENDERER_W;
    }

    if(startP.y < 0 || startP.y > RENDERER_H){
        startP.y = rand() % RENDERER_H;
    }

    return (Fruit){symbol, startP};
}

void renderSnake(char* renderer, Snake* snake)
{

    drawPoint(renderer, snake->head_sym, snake->pos.x, snake->pos.y);

    int i;
    for(i = 1; i < snake->size; i++)
    {
        if(snake->body[i].x == snake->pos.x && snake->body[i].y == snake->pos.y){
            //snake->dead = true;
        }

        drawPoint(renderer, snake->body_sym, snake->body[i].x, snake->body[i].y);
    }
}

void renderFruit(char* renderer, Fruit fruit)
{
    drawPoint(renderer, fruit.symbol, fruit.pos.x, fruit.pos.y);
}

Point getWallCollisionModifier(Snake snake)
{
    if(snake.pos.x > RENDERER_W)
        return (Point){0, snake.pos.y};

    if(snake.pos.x < 0)
        return (Point){RENDERER_W, snake.pos.y};

    if(snake.pos.y > RENDERER_H)
        return (Point){snake.pos.x, 0};

    if(snake.pos.y < 0)
        return (Point){snake.pos.x, RENDERER_H};

    return (Point){snake.pos.x, snake.pos.y};
}

void moveSnake(Snake* snake)
{
    Point wallCollisionModifier = getWallCollisionModifier(*snake);
    Point newPos = (Point){wallCollisionModifier.x+snake->dir.x, wallCollisionModifier.y+snake->dir.y};

    int i;
    for(i = snake->size-1; i > 0; i--){
        snake->body[i] = snake->body[i-1];
    }

    snake->pos = snake->body[0] = newPos;
}

void feed(Snake* snake)
{
    if(snake->size < SNAKE_MAX_SIZE-1)
    {
        snake->size++;
    }
}

void relocateFruit(Fruit* fruit)
{
    fruit->pos = (Point){rand()%RENDERER_W, rand()%RENDERER_H};
}

void loadTraining()
{
    FILE *tFile = fopen("training.data", "r");
    int i = 0, expected;
    bool eof = false;

    while(nsamples < MAX_TRAINING_SAMPLES && !eof){
        initTrainingVector(&samples[nsamples]);

        while(i < INPUT_VECTOR_SIZE && fscanf(tFile, "%g ", &samples[nsamples].inputs[i]) == 1){
            i++;
        }

        if(i == INPUT_VECTOR_SIZE){
            fscanf(tFile, "%i\n", &expected);
            dir2OutputVector(expected, &samples[nsamples]);
            nsamples++;
            i = 0;
        }
        else{
            eof = true;
        }
    }

    fclose(tFile);
}

void saveTraining()
{
    FILE *tFile = fopen("training.data", "a+");
    int i, j;

    for(i = 0; i < nsamples; i++){
        for(j = 0; j < INPUT_VECTOR_SIZE; j++){
            fprintf(tFile, "%g ", samples[i].inputs[j]);
        }
        fprintf(tFile, "%i\n", outputVector2Dir(samples[i]));
    }

    fclose(tFile);
}

void setTrainingGameState(Snake snake, Fruit fruit, TrainingVector *tv)
{
    float dx = (float)fruit.pos.x - (float)snake.pos.x;
    float dy = (float)fruit.pos.y - (float)snake.pos.y;
    float angle = atan2f(dy, dx);

    float s[] = {
        snake.dir.x, snake.dir.y, 
        angle
    };

    memcpy(tv->inputs, s, tv->inputSize*sizeof(float));
}

void dir2OutputVector(int dir, TrainingVector *tv)
{
    memset(tv->outputs, 0, tv->outputSize*sizeof(float));
    tv->outputs[dir] = 1;
}

int outputVector2Dir(TrainingVector tv)
{
    int i;

    for(i = 0; i < tv.outputSize; i++){
        if(tv.outputs[i] == 1) return i;
    }
}