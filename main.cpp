#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>
#include <ctime>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>

#include "config.h"
#include "raylib_extensions.h"

#include "scene.h"
#include "clock.h"
#include "pong.h"
#include "graph.h"
#include "tetris.h"

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    
    // Window Config
    const int screenWidth = 1536;
    const int screenHeight = 2048;
    
    // Window
    // SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "LKG Application");
    
    // Fix Rectangle UVs (See: https://github.com/raysan5/raylib/issues/1730)
    Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    SetShapesTexture(texture, Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });
    // SetShapesTexture(rlGetTextureDefault(), Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });
    
    //Load shaders
    Shader lkgFragment = LoadShaderSingleFile("./Shaders/quilt.shader"); // Quilt shader

    // Scene
    //Scene* scene = new PongScene();
    //Scene* scene = new ConsoleScene();
    //Scene* scene = new GraphScene();
    Scene* scene = new ClockScene();
    //Scene* scene = new TetrisScene();

    // LKG Config
    std::ifstream config_file("display.cfg");
    LKGConfig config(config_file);
    
    float ri = 0.0f;
    float bi = 2.0f;
    
    std::pair<float, float> angleDistance = scene->GetAngleDistance();
    std::pair<int, int> tiles = scene->GetTiles();
    std::pair<int, int> tileRes = scene->GetTileResolution();
    
    // Initialize shader uniforms
    int quiltTexLoc = GetShaderLocation(lkgFragment, "texture1");
    int pitchLoc = GetShaderLocation(lkgFragment, "pitch");
    SetShaderValue(lkgFragment, pitchLoc, &config.pitch, SHADER_UNIFORM_FLOAT);
    int slopeLoc = GetShaderLocation(lkgFragment, "slope");
    SetShaderValue(lkgFragment, slopeLoc, &config.slope, SHADER_UNIFORM_FLOAT);
    int centerLoc = GetShaderLocation(lkgFragment, "center");
    SetShaderValue(lkgFragment, centerLoc, &config.center, SHADER_UNIFORM_FLOAT);
    int dpiLoc = GetShaderLocation(lkgFragment, "dpi");
    SetShaderValue(lkgFragment, dpiLoc, &config.dpi, SHADER_UNIFORM_FLOAT);
    int riLoc = GetShaderLocation(lkgFragment, "ri");
    SetShaderValue(lkgFragment, riLoc, &ri, SHADER_UNIFORM_FLOAT);
    int biLoc = GetShaderLocation(lkgFragment, "bi");
    SetShaderValue(lkgFragment, biLoc, &bi, SHADER_UNIFORM_FLOAT);
    int tileLoc = GetShaderLocation(lkgFragment, "tile");
    float tile[2] = { tiles.first, tiles.second };
    SetShaderValue(lkgFragment, tileLoc, tile, SHADER_UNIFORM_VEC2);
    
    // Render textures 8x6 (420x560)
    RenderTexture2D quiltRT = LoadRenderTexture(tileRes.first * tiles.first, tileRes.second * tiles.second);
    const int TILE_WIDTH = tileRes.first;
    const int TILE_HEIGHT = tileRes.second;
    const int TILE_COUNT = tiles.first * tiles.second;
    
    // Camera
    Camera3D camera = { 0 };
    camera.position = { 0, 0, angleDistance.second };
    camera.target = { 0, 0, 0 };
    camera.up = { 0, 1.0f, 0 };
    camera.fovy = 17.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    //SetTargetFPS(30);               // Set our viewer to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
    scene->Update();
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(quiltRT);
            ClearBackground(scene->GetClearColor());
            for (int i = TILE_COUNT - 1; i >= 0; i--) {
                rlViewport((i%(int)tile[0])*TILE_WIDTH, (floor(i/(int)tile[0]))*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
                
                float movementAmount = tan((config.viewCone/2.0f) * DEG2RAD) * angleDistance.second;
                float offset = -movementAmount + ((movementAmount * 2)/TILE_COUNT) * i;
                camera.position.x = offset;
                camera.target.x = offset;
                
                BeginMode3DLG(camera, (float)TILE_WIDTH/(float)TILE_HEIGHT, -offset);
                //Rotate stand angle
                rlPushMatrix();
                rlRotatef(angleDistance.first, 1, 0, 0);
                    scene->Draw();
                rlPopMatrix();
                EndMode3D();
            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginShaderMode(lkgFragment);
                SetShaderValueTexture(lkgFragment, quiltTexLoc, quiltRT.texture);
                DrawRectangle(0,0, 1536, 2048, WHITE);
                //DrawTexture(quiltRT.texture, 0, 0, WHITE);
            EndShaderMode();

            if (scene->ShowFPS())
                DrawFPSSize(50, 50, 90);
            //std::cout << GetFPS() << std::endl;
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(quiltRT);
    UnloadShader(lkgFragment);

    ClearDroppedFiles();
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
