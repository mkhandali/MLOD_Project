#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>


#define SQUARE_SIZE  50
#define MAP_X  10
#define MAP_Y  20
#define MODULE_SIZE  4

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
static Vector2 offset = { 600,100 };
static Vector2 blocSize = { SQUARE_SIZE,SQUARE_SIZE };





// Enum
typedef enum bloc { VIDE, MOBILE, FIXE} bloc;

typedef struct{
    bloc etat;
    Color color;
}blocMap;

// Matrices
static blocMap map [MAP_X][MAP_Y];
static bloc module[4][4];
static bloc moduleSuivant[4][4];

Vector2 posModule;
Color moduleColor;
Color moduleSuivantColor;
int moduleType;
int moduleTypeS;
int moduleCase;
int score;
bool gameOver;
int speed;
bool gameOver = false;
bool moduleActivity;
bool moduleEnPlace;
bool enRotation;

//////////////////////////// FCT DECLA
static void moduleRandomizer();
static void setModule();
static void moduleRandomizer();
void deplacementHorizontal();
void rotation();
void ligneComplete();
void detectGameOver();

void InitGame(void)
{
    speed=6;
    posModule.x=0;
    posModule.y=0;
    SetTargetFPS(speed);
    moduleColor=RAYWHITE;
    moduleSuivantColor=RAYWHITE;
    score=0;
    enRotation=false;

    for (int i = 0; i < MAP_X; i++)
    {
        for (int j = 0; j < MAP_Y; j++)
        {
            map[i][j].etat = VIDE;
            map[i][j].color = RAYWHITE;
        }
    }

    moduleRandomizer();
}

void updateMap(){
    
    if (!moduleActivity)
    {
        setModule();
    }
    else 
    {
        detectCollision();
        deplacementVertical();
        deplacementHorizontal();
        rotation();
        ligneComplete();
        detectGameOver();
    }
    

    
}
void detectGameOver(){
    
    for (int i = 0; i < MAP_X; i++)
    {
        if(map[i][0].etat == FIXE)
        gameOver=true;
    }
    
}
void deplacementVertical()
{
    if (!enRotation){
        if (moduleEnPlace)
        {
            for (int j = MAP_Y - 1; j >= 0; j--)
            {
                for (int i = 0; i < MAP_X; i++)
                {
                    if (map[i][j].etat == MOBILE)
                    {
                        map[i][j].etat = FIXE;
                        moduleEnPlace = false;
                        moduleActivity = false;
                    }
                }
            }
        }
        else
        {
            for (int j = MAP_Y - 1; j >= 0; j--)
            {
                for (int i = 0; i < MAP_X; i++)
                {
                    if (map[i][j].etat == MOBILE)
                    {
                        map[i][j+1].etat = MOBILE;
                        map[i][j+1].color = map[i][j].color;
                        map[i][j].etat = VIDE;
                        map[i][j].color= RAYWHITE;
                    }
                }
            }
            
            posModule.y++;
        }
    }}

void deplacementHorizontal()
{
    bool collision = false;

    if (IsKeyDown(KEY_LEFT))   
    {
        
        for (int j = MAP_Y - 1; j >= 0; j--)
        {
            for (int i = 0; i < MAP_X; i++)
            {
                if (map[i][j].etat == MOBILE)
                {
                    if ((i-1< 0) || (map[i-1][j].etat == FIXE)) collision = true;
                }
            }
        }
        
        if (!collision)
        {
            for (int j = MAP_Y-1; j >= 0; j--)
            {
                for (int i = 0; i < MAP_X; i++)     
                {
                    if (map[i][j].etat == MOBILE)
                    {
                        map[i-1][j].etat = MOBILE;
                        map[i-1][j].color=map[i][j].color;
                        map[i][j].etat = VIDE;
                        map[i][j].color=RAYWHITE;
                    }
                }
            }

            posModule.x--;
        }
    }
    else if (IsKeyDown(KEY_RIGHT))
    {
        for (int j = MAP_Y-1; j >= 0; j--)
        {
            for (int i = 0; i < MAP_X; i++)
            {
                if (map[i][j].etat == MOBILE)
                {
                    if ((i+1> MAP_X-1) || (map[i+1][j].etat == FIXE))
                    {
                        collision = true;
                    }
                }
            }
        }
        

        if (!collision)
        {
            for (int j = MAP_Y-1; j >= 0; j--)
            {
                for (int i = MAP_X - 1; i >= 0; i--)
                {
                    if (map[i][j].etat == MOBILE)
                    {
                        map[i+1][j].etat = MOBILE;
                        map[i+1][j].color=map[i][j].color;
                        map[i][j].etat = VIDE;
                        map[i][j].color=RAYWHITE;
                    }
                }
            }

            posModule.x++;
        }
    }
}


void rotation()
{
    bool collision = false;

    if (IsKeyDown(KEY_UP) && ((int)posModule.x+1>0)&&((int)posModule.x<MAP_X-3)&&((int)posModule.y<MAP_Y-3)){
        
        enRotation=true;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                module[i][j] = VIDE;
            }
        }
        
        switch (moduleType)
        {
        case 0: { break;}      
        case 1: { 
                switch (moduleCase){
                case 0: { module[0][1]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[2][0]= MOBILE; moduleCase=1;} break;
                case 1: { module[1][0]= MOBILE; module[1][1]= MOBILE; module[1][2]= MOBILE; module[0][0]= MOBILE; moduleCase=2;} break;
                case 2: { module[0][1]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[0][2]= MOBILE; moduleCase=3;} break;
                case 3: { module[1][0]= MOBILE; module[1][1]= MOBILE; module[1][2]= MOBILE; module[2][2]= MOBILE; moduleCase=0;} break;
                }break;};     
        case 2: { 
                switch (moduleCase){
                case 0: { module[0][1]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[2][2]= MOBILE; moduleCase=1;} break;
                case 1: { module[1][0]= MOBILE; module[1][1]= MOBILE; module[1][2]= MOBILE; module[2][0]= MOBILE; moduleCase=2;} break;
                case 2: { module[0][1]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[0][0]= MOBILE; moduleCase=3;} break;
                case 3: { module[1][2]= MOBILE; module[2][0]= MOBILE; module[2][1]= MOBILE; module[2][2]= MOBILE; moduleCase=0;} break;
                }break;};     
        case 3: { 
                switch (moduleCase){
                case 0: { module[1][0]= MOBILE; module[1][1]= MOBILE; module[1][2]= MOBILE; module[1][3]= MOBILE; moduleCase=1;} break;
                case 1: { module[0][1]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[3][1]= MOBILE; moduleCase=0;} break;
                }break;};     
        case 4: { 
                switch (moduleCase){
                case 0: { module[0][1]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[1][2]= MOBILE; moduleCase=1;} break;
                case 1: { module[1][0]= MOBILE; module[1][1]= MOBILE; module[1][2]= MOBILE; module[2][1]= MOBILE; moduleCase=2;} break;
                case 2: { module[0][1]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[1][0]= MOBILE; moduleCase=3;} break;
                case 3: { module[1][0]= MOBILE; module[1][1]= MOBILE; module[1][2]= MOBILE; module[0][1]= MOBILE; moduleCase=0;} break;
                }break;};   
        case 5: { 
                switch (moduleCase){
                case 0: { module[2][0]= MOBILE; module[2][1]= MOBILE; module[1][1]= MOBILE; module[1][2]= MOBILE; moduleCase=1;} break;
                case 1: { module[1][1]= MOBILE; module[2][1]= MOBILE; module[2][2]= MOBILE; module[3][2]= MOBILE; moduleCase=0;} break;
                }break;};   
        case 6: { 
                switch (moduleCase){
                case 0: { module[1][0]= MOBILE; module[1][1]= MOBILE; module[2][1]= MOBILE; module[2][2]= MOBILE; moduleCase=1;} break;
                case 1: { module[1][2]= MOBILE; module[2][2]= MOBILE; module[2][1]= MOBILE; module[3][1]= MOBILE; moduleCase=0;} break;
                }break;};  
        }
        
        moduleToMap();
    }
    else
    enRotation=false;
    
}
void detectCollision()
{
    for (int j = MAP_Y-1; j >= 0; j--)
    {
        for (int i = 0; i < MAP_X; i++)
        {
            if ((map[i][j].etat == MOBILE) && ((map[i][j+1].etat == FIXE) || (j>MAP_Y-2)))
            moduleEnPlace = true;
        }
    }
}

void setModule(){
    moduleActivity=1;
    posModule.y=0;
    posModule.x=0;
    moduleCase=0;
    moduleType=moduleTypeS;
    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            module[i][j]= moduleSuivant[i][j];
            moduleColor=moduleSuivantColor;
        }
    }
    
    moduleRandomizer();
    moduleToMap();

}

void moduleToMap(){
    
    
    for (int j = MAP_Y-1; j >= 0; j--)
    {
        for (int i = 0; i < MAP_X; i++)
        {
            if (map[i][j].etat == MOBILE)
            {
                map[i][j].etat = VIDE;
                map[i][j].color=RAYWHITE;
            }
        }
    }
    
    for (int i = (int)posModule.x; i < (int)posModule.x + 4; i++)
    {
        for (int j = (int)posModule.y; j < (int)posModule.y + 4; j++)
        {
            if (module[i - (int)posModule.x][j - (int)posModule.y] == MOBILE)
            {
                map[i][j].etat = MOBILE;
                map[i][j].color=moduleColor;
            }
        }
    }
}

static void moduleRandomizer()
{
    int random = GetRandomValue(0, 6);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            moduleSuivant[i][j] = VIDE;
            moduleSuivantColor = RAYWHITE;
        }
    }

    switch (random)
    {
    case 0: { moduleSuivant[1][1]= MOBILE; moduleSuivant[2][1]= MOBILE; moduleSuivant[1][2]= MOBILE; moduleSuivant[2][2]= MOBILE; moduleSuivantColor=RED;moduleTypeS=0;} break;    //Carre
    case 1: { moduleSuivant[1][0]= MOBILE; moduleSuivant[1][1]= MOBILE; moduleSuivant[1][2]= MOBILE; moduleSuivant[2][2]= MOBILE; moduleSuivantColor=BLUE;;moduleTypeS=1;} break;    //L
    case 2: { moduleSuivant[1][2]= MOBILE; moduleSuivant[2][0]= MOBILE; moduleSuivant[2][1]= MOBILE; moduleSuivant[2][2]= MOBILE; moduleSuivantColor=ORANGE;;moduleTypeS=2;} break;    //L inverse
    case 3: { moduleSuivant[0][1]= MOBILE; moduleSuivant[1][1]= MOBILE; moduleSuivant[2][1]= MOBILE; moduleSuivant[3][1]= MOBILE; moduleSuivantColor=GRAY;;moduleTypeS=3;} break;    //Barre
    case 4: { moduleSuivant[1][0]= MOBILE; moduleSuivant[1][1]= MOBILE; moduleSuivant[1][2]= MOBILE; moduleSuivant[2][1]= MOBILE; moduleSuivantColor=PURPLE;;moduleTypeS=4;} break;    //T
    case 5: { moduleSuivant[1][1]= MOBILE; moduleSuivant[2][1]= MOBILE; moduleSuivant[2][2]= MOBILE; moduleSuivant[3][2]= MOBILE; moduleSuivantColor=YELLOW;;moduleTypeS=5;} break;    //S
    case 6: { moduleSuivant[1][2]= MOBILE; moduleSuivant[2][2]= MOBILE; moduleSuivant[2][1]= MOBILE; moduleSuivant[3][1]= MOBILE; moduleSuivantColor=GREEN;;moduleTypeS=6;} break;    //S INVERSE
    }
}

void ligneComplete()
{
    int compteLigne = 0;

    for (int j = MAP_Y - 1; j > 0; j--)
    {
        compteLigne = 0;
        for (int i = 0; i < MAP_X; i++)
        {
            if (map[i][j].etat == FIXE)
            compteLigne++;
        }
        if (compteLigne == MAP_X)
        {
            compteLigne = 0;
            score++;
            
            for (int k = j; k > 0; k--)
            {
                for (int i = 0; i < MAP_X; i++)
                {
                    map[i][k].etat = map[i][k-1].etat;
                    map[i][k].color= map[i][k-1].color;
                }
            }
            
            for (int i = 0; i < MAP_X; i++)
            {
                map[i][0].etat = VIDE;
                map[i][0].color= RAYWHITE;
            }
            
        }
    }
}

void draw(void){
    BeginDrawing();
    
    ClearBackground(RAYWHITE);
    
    if (!gameOver){
        
        updateMap();
        for(int i=0;i<MAP_X;i++){
            for(int j=0;j<MAP_Y;j++){
                DrawRectangleV((Vector2){i*SQUARE_SIZE+offset.x/2,j*SQUARE_SIZE+offset.y/2},blocSize,map[i][j].color);
                DrawRectangleLines(i*SQUARE_SIZE+offset.x/2,j*SQUARE_SIZE+offset.y/2,blocSize.x,blocSize.y,GRAY);                    
            }
        }
        
        for(int i=0;i<MODULE_SIZE;i++){
            for(int j=0;j<MODULE_SIZE;j++){
                DrawRectangleLines(i*SQUARE_SIZE+900,j*SQUARE_SIZE+300,blocSize.x,blocSize.y,GRAY);
                if(moduleSuivant[i][j]==MOBILE){
                    DrawRectangle(i*SQUARE_SIZE+900,j*SQUARE_SIZE+300,blocSize.x,blocSize.y,moduleSuivantColor);
                }
            }
        }
        DrawText(TextFormat("SCORE: %04i",score), 900, 500, 20, BLACK);
    }
    else{
        DrawText(TextFormat("GAME OVER"), 300, 500, 100, BLACK);
    }
    

    EndDrawing();

}
int main(int argc, char *argv[]){
    InitWindow(screenWidth, screenHeight, "TETRIS COLOR");
    InitGame();
    setModule();
    
    
    while(!WindowShouldClose()){
        
        draw();
    }
    
    return 0;
}
