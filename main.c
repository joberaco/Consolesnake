#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <Windows.h>

#define RENDERER_W 100
#define RENDERER_H 25
#define RENDERER_SIZE RENDERER_W*RENDERER_H
#define SLEEP_MS 10

void init(char** renderer);
void clear(char* renderer);
void present(char* renderer);

void drawPoint(char* renderer, char symbol, int x, int y);
void drawLine(char* renderer, char symbol, int x1, int y1, int x2, int y2);
void drawCircle(char* renderer, char symbol, int xc, int yc, int r);

int inBounds(int x, int y);
void swap(int* a, int* b);

char* renderer;

int main()
{
    init(&renderer);
    int period = 0, t = 0, r, x, y;

    srand(time(NULL));

    while(1)
    {
        clear(renderer);

        /*drawLine(renderer, '*', 20, 5, 30, 5);
        drawLine(renderer, '*', 20, 5, 23, 10);
        drawLine(renderer, '*', 20, 5, 10, 5);
        drawLine(renderer, '*', 20, 5, 10, 10);*/

        r = 6, x = 20, y = 10;

        drawLine(renderer, '*', x, y, x + r*cos(((2*3.14)/20)*t), y + r*sin(((2*3.14)/20)*t));
        r = 7;
        drawPoint(renderer, '#', x + r*cos(((2*3.14)/20)*t), y + r*sin(((2*3.14)/20)*t));
        r = 8;
        drawCircle(renderer, '*', x, y, r);

        r = 5, x = 50, y = 9;

        drawLine(renderer, '*', x, y, x + r*cos(((2*3.14)/30)*t), y - r*sin(((2*3.14)/30)*t));
        drawCircle(renderer, '*', x, y, r);

        r = 5, x = 70, y = 15;

        drawLine(renderer, '*', x, y, x + r*cos(((2*3.14)/20)*t), y + r*sin(((2*3.14)/20)*t));
        drawCircle(renderer, '*', x, y, r);

        t++;

        /*for(x = 0; x < RENDERER_W; x++, period++)
        {
            //drawPoint(renderer, '-', x, RENDERER_H/2);

            //drawPoint(renderer, '.', x, -pow(((x-50)/3),2)+10 + RENDERER_H/2);
            //drawPoint(renderer, '.', x, sin((x+period)/3.14)*3 + RENDERER_H/2);
            //drawPoint(renderer, '.', x, -sin((x+period)/3.14)*3 + RENDERER_H/2);
            //drawPoint(renderer, '.', x, -tan(((x+period)*5)/(3.14/2))*2 + RENDERER_H/2);
            //drawPoint(renderer, '.', x, (1/sin((x+period)/(3.14)))*3 + RENDERER_H/2);
        }*/

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