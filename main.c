#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <Windows.h>

#define RENDERER_W 100
#define RENDERER_H 25
#define RENDERER_SIZE RENDERER_W*RENDERER_H
#define SLEEP_MS 10

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
    bool dead;
} Snake;

void init(char** renderer);
void clear(char* renderer);
void present(char* renderer);

//char getKey();

void drawPoint(char* renderer, char symbol, int x, int y);
void drawLine(char* renderer, char symbol, int x1, int y1, int x2, int y2);
void drawCircle(char* renderer, char symbol, int xc, int yc, int r);

int inBounds(int x, int y);
void swap(int* a, int* b);

Snake newSnake(char symbol, int maxsize, Point startP);
void renderSnake(char* renderer, Snake* snake);
void moveSnake(Snake* snake, Point pos);
void feed(Snake* snake);

char* renderer;

int main()
{
    Snake snake = newSnake('#', SNAKE_MAX_SIZE, (Point){50,5});
    int xspeed = SNAKE_SPEED, yspeed = 0;
    
    init(&renderer);
    srand(time(NULL));

    while(1)
    {
        clear(renderer);

        if(!snake.dead)
        {
            if(yspeed == 0 && (rand()%10 == 0)){
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
            }

            if(rand() % 2 == 0){
                feed(&snake);
            }

            moveSnake(&snake, (Point){snake.pos.x+xspeed, snake.pos.y+yspeed});
        }
        else
        {
            printf("**********************************DEAD**********************************");
            system("pause");
        }

        renderSnake(renderer, &snake);

        present(renderer);
        
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

Snake newSnake(char symbol, int maxsize, Point startP)
{
    Snake snake;
    int i;

    snake = (Snake){symbol, '~', SNAKE_MIN_SIZE, startP, (Point*)malloc(maxsize*sizeof(Point)), false};

    for(i = 0; i < SNAKE_MIN_SIZE; i++){
        snake.body[i].x = startP.x - i;
        snake.body[i].y = startP.y;
    }

    return snake;
}

void renderSnake(char* renderer, Snake* snake)
{

    drawPoint(renderer, snake->head_sym, snake->pos.x, snake->pos.y);

    int i;
    for(i = 1; i < snake->size; i++)
    {
        if(snake->body[i].x == snake->pos.x && snake->body[i].y == snake->pos.y){
            snake->dead = true;
        }

        drawPoint(renderer, snake->body_sym, snake->body[i].x, snake->body[i].y);
    }
}

void moveSnake(Snake* snake, Point pos)
{
    int i;
    for(i = snake->size-1; i > 0; i--){
        snake->body[i] = snake->body[i-1];
    }

    snake->pos = snake->body[0] = pos;
}

void feed(Snake* snake)
{
    if(snake->size < SNAKE_MAX_SIZE-1)
    {
        snake->size++;
    }
}