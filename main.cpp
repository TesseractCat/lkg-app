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

#define LKG_DISTANCE 20.0f
#define LKG_ANGLE 25

Shader litShader;

Material litMaterial;
Mesh cubeMesh;
Model testModel;

void DrawScene() {
    const float BACKDROP_DIST = -4.5f;
    
    float game_time = GetTime();// * 0.25f;
    Vector3 position = {(float)sin(game_time), (float)sin(game_time * 2.0f) * 1.5f, 0.5f};
    Vector3 position2 = {(float)sin(game_time * 3.0f), (float)sin(game_time * 1.5f) * 1.5f, -0.5f};

    //litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    //rlPushMatrix();
    //    rlTranslatef(position.x, position.y, shadow ? 0 : position.z);
    //    DrawMesh(cubeMesh, litMaterial, MatrixIdentity());
    //    rlScalef(1.5f, 1.0f, shadow ? 1.0f : 0.5f);
    //    DrawMesh(cubeMesh, litMaterial, MatrixIdentity());
    //    rlScalef(0.5f, 1.5f, shadow ? 1.0f : 0.5f);
    //    DrawMesh(cubeMesh, litMaterial, MatrixIdentity());
    //rlPopMatrix();

    std::time_t now = time(nullptr);
    std::tm calender_time = *std::localtime( std::addressof(now) ) ;

    /*rlPushMatrix();
        rlTranslatef(0.5f, sin(GetTime()), -0.5f);
	rlRotatef(90, 1, 0, 0);
        DrawModel(testModel, Vector3{0,0,0}, 0.15f, WHITE);
    rlPopMatrix();
    rlPushMatrix();
        rlTranslatef(1.5f, sin(GetTime()), 0.5f);
	rlRotatef(90, 1, 0, 0);
        DrawModel(testModel, Vector3{0,0,0}, 0.15f, WHITE);
    rlPopMatrix();*/

    Matrix transforms[1500];
    Color colors[1500];
    int instanceIdx = 0;

    auto drawCube = [&] (Matrix m, Color c) {
        c.a = 0;
	colors[instanceIdx] = c;
        transforms[instanceIdx++] = m;
        c.a = 255;
	colors[instanceIdx] = c;
        transforms[instanceIdx++] = m;
    };
    
    //Seconds
    litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    rlPushMatrix();
	rlRotatef(fmod(calender_time.tm_sec, 60.0f)/60.0f * -360.0f, 0, 0, 1);
        rlScalef(0.05f, 1.1f, 0.05f);
        rlTranslatef(0, 0.6666f, 0);
	drawCube(rlGetMatrixTransform(), RED);
    rlPopMatrix();
    //Minutes
    //litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    rlPushMatrix();
	rlRotatef(fmod(calender_time.tm_min, 60.0f)/60.0f * -360.0f, 0, 0, 1);
        rlScalef(0.05f, 1.1f, 0.05f);
        rlTranslatef(0, 0.6666f, 1.5f);
	drawCube(rlGetMatrixTransform(), DARKGRAY);
    rlPopMatrix();
    //Hours
    //litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    rlPushMatrix();
	rlRotatef(fmod(calender_time.tm_hour % 12, 12.0f)/12.0f * -360.0f, 0, 0, 1);
        rlScalef(0.05f, 0.8f, 0.05f);
        rlTranslatef(0, 0.6666f, -1.5f);
	drawCube(rlGetMatrixTransform(), DARKGRAY);
    rlPopMatrix();

    /*for (int i = 0; i < 250; i++) {
        rlPushMatrix();
            rlScalef(0.1f, 0.1f, 0.1f);
            rlRotatef((i/50.0f) * 360.0f, 0, 0, 1);
            rlTranslatef(18.0f + sin(GetTime() * 3.0f + i) * 3.0f, 0, (i/50) * -3.0f + 7.5f);
	    drawCube(rlGetMatrixTransform(), BLUE);
        rlPopMatrix();
    }*/
    for (int i = 0; i < 12; i++) {
        rlPushMatrix();
            rlScalef(0.1f, 0.1f, 0.1f);
            rlRotatef((i/12.0f) * 360.0f, 0, 0, 1);
            rlTranslatef(19.0f + sin(GetTime() * 3.0f + i), 0, 0);
	    drawCube(rlGetMatrixTransform(), DARKGRAY);
        rlPopMatrix();
    }
    DrawMeshInstancedC(cubeMesh, litMaterial, transforms, colors, instanceIdx);
}

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    
    // Window Config
    const int screenWidth = 1536;
    const int screenHeight = 2048;
    
    // Window
    // SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "LKG Quilt Viewer");
    
    // Fix Rectangle UVs (See: https://github.com/raysan5/raylib/issues/1730)
    Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    SetShapesTexture(texture, Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });
    // SetShapesTexture(rlGetTextureDefault(), Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });
    
    //Load shaders
    Shader lkgFragment = LoadShaderSingleFile("./Shaders/quilt.shader"); // Quilt shader

    litShader = LoadShaderSingleFile("./Shaders/lit_instanced.shader"); // Lit shader
    //litShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(litShader, "matModel");
    litShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(litShader, "matModel");
    litShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(litShader, "colDiffuse");
    litShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(litShader, "matView");
    litShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(litShader, "matProjection");

    Vector3 lightPos = Vector3{0.0f, -3.0f, 22.0f};
    SetShaderValue(litShader, GetShaderLocation(litShader, "lightPos"), &lightPos, SHADER_UNIFORM_VEC3);
    float planeZ = -2.0f;
    SetShaderValue(litShader, GetShaderLocation(litShader, "planeZ"), &planeZ, SHADER_UNIFORM_FLOAT);

    litMaterial = LoadMaterialDefault(); // Lit material
    litMaterial.shader = litShader;
    litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;

    //Load meshes
    cubeMesh = GenMeshCube(1.5f, 1.5f, 1.5f);
    testModel = LoadModel("./Models/gameboy.gltf");
    for (int i = 0; i < testModel.materialCount; i++) {
        testModel.materials[i].shader = litShader;
    }
    
    // LKG Config
    std::ifstream config_file("display.cfg");
    LKGConfig config(config_file);
    
    int ri = 0;
    int bi = 2;
    
    float tile[2] = {(float)8,(float)6};
    //float tile[2] = {(float)8,(float)4};
    
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
    SetShaderValue(lkgFragment, riLoc, &ri, SHADER_UNIFORM_INT);
    int biLoc = GetShaderLocation(lkgFragment, "bi");
    SetShaderValue(lkgFragment, biLoc, &bi, SHADER_UNIFORM_INT);
    int tileLoc = GetShaderLocation(lkgFragment, "tile");
    SetShaderValue(lkgFragment, tileLoc, tile, SHADER_UNIFORM_VEC2);
    
    // Render textures 8x6 (420x560)
    //RenderTexture2D quiltRT = LoadRenderTexture(3072, 2048);
    //const int TILE_WIDTH = 384;
    //const int TILE_HEIGHT = 512;
    //RenderTexture2D quiltRT = LoadRenderTexture(1008, 1008);
    //const int TILE_WIDTH = 126;
    //const int TILE_HEIGHT = 168;
    RenderTexture2D quiltRT = LoadRenderTexture(1344, 1344);
    const int TILE_WIDTH = 168;
    const int TILE_HEIGHT = 224;
    //RenderTexture2D quiltRT = LoadRenderTexture(3360, 3360);
    //const int TILE_WIDTH = 420;
    //const int TILE_HEIGHT = 560;

    int tileCount = tile[0] * tile[1];
    
    // Camera
    Camera3D camera = { 0 };
    camera.position = { 0, 0, LKG_DISTANCE };
    camera.target = { 0, 0, 0 };
    camera.up = { 0, 1.0f, 0 };
    camera.fovy = 17.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    //std::vector<Matrix> projectionMatrices(tileCount, { 0 });
    
    //SetTargetFPS(30);               // Set our viewer to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        if (IsKeyPressed(KEY_S)) {
            TakeScreenshot("./screenshot.jpg");
        }
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(quiltRT);
            ClearBackground(Color{225,225,225,255});
            for (int i = tileCount - 1; i >= 0; i--) {
                rlViewport((i%(int)tile[0])*TILE_WIDTH, (floor(i/(int)tile[0]))*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
                
                float movementAmount = tan((config.viewCone/2.0f) * DEG2RAD) * LKG_DISTANCE;
                float offset = -movementAmount + ((movementAmount * 2)/tileCount) * i;
                camera.position.x = offset;
                camera.target.x = offset;
                
                BeginMode3DLG(camera, (float)TILE_WIDTH/(float)TILE_HEIGHT, -offset);
            	//Rotate stand angle
            	rlPushMatrix();
            	rlRotatef(LKG_ANGLE, 1, 0, 0);
	    	    BeginShaderMode(litShader);
            	        DrawScene();
	            EndShaderMode();
            	rlPopMatrix();
                EndMode3D();
            }
        EndTextureMode();

        BeginDrawing();
            //ClearBackground(RAYWHITE);
            BeginShaderMode(lkgFragment);
                SetShaderValueTexture(lkgFragment, quiltTexLoc, quiltRT.texture);
                DrawRectangle(0,0, 1536, 2048, WHITE);
            EndShaderMode();

            DrawFPSSize(50, 50, 90);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(quiltRT);
    
    UnloadShader(litShader);
    UnloadShader(lkgFragment);
    
    ClearDroppedFiles();
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
