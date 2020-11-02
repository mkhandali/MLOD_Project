#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>


#define SQUARE_SIZE  50
#define NBR_SQUARE_X  10
#define NBR_SQUARE_Y  20

// Structures Definition
//----------------------------------------------------------------------------------
/*
typedef struct Snake {
    Vector2 position;
    Vector2 size;
    Vector2 speed;
    Color color;
} Snake;*/
//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 1200;
static const int screenHeight = 1200;
static const int gameWidth = SQUARE_SIZE*NBR_SQUARE_X;
static const int gameHeight = SQUARE_SIZE*NBR_SQUARE_Y;

static Vector2 offset = { 600,100 };

typedef struct{
    Vector2 position
}figure;


int main(int argc, char *argv[]){
    InitWindow(screenWidth, screenHeight, "DevWindow");
    SetTargetFPS(60);
    while(!WindowShouldClose())
        DrawGame();
    return 0;
}

void DrawGame(void)
{
    
    BeginDrawing();

        ClearBackground(RAYWHITE);
        
            // Draw grid lines
            for (int i = 0; i < NBR_SQUARE_X+1; i++)
            {
                DrawLineV((Vector2){SQUARE_SIZE*i + offset.x/2, offset.y/2}, (Vector2){SQUARE_SIZE*i+ offset.x/2, gameHeight+offset.y/2}, GRAY);
            }

            for (int i = 0; i < NBR_SQUARE_Y+1; i++)
            {
                DrawLineV((Vector2){offset.x/2, SQUARE_SIZE*i + offset.y/2}, (Vector2){gameWidth+ offset.x/2, SQUARE_SIZE*i+ offset.y/2}, GRAY);
            }


    EndDrawing();
    
}