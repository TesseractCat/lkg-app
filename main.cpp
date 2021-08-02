extern "C" {
    #include "raylib.h"
    #include "raymath.h"
    #include "rlgl.h"
}
#include <cmath>
#include <stdio.h>
#include <assert.h>

#include <string>
#include <regex>
#include <iostream>
#include <fstream>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#define LKG_DISTANCE 20.0f
#define LKG_ANGLE 25

struct LKGConfig {
    float pitch;
    float slope;
    float center;
    float viewCone;
    float dpi;
    int invView;
};

LKGConfig LoadConfig(std::istream& config_file) {
    LKGConfig out;
    
    float pitch = 52.56658151511047;
    config_file.ignore(256, '=');
    config_file >> out.pitch;
    float slope = -7.145505158882189;
    config_file.ignore(256, '=');
    config_file >> out.slope;
    float center = 0.9997089252844671;
    config_file.ignore(256, '=');
    config_file >> out.center;
    float viewCone = 40.0f;
    config_file.ignore(256, '=');
    config_file >> out.viewCone;
    int invView = 1;
    config_file.ignore(256, '=');
    config_file >> out.invView;
    float dpi = 324.0;
    config_file.ignore(256, '=');
    config_file >> out.dpi;
    
    return out;
}

Matrix frustumMatrixOffAxis(double left, double right, double bottom, double top, double znear, double zfar,
        double offset, double aspect, double fov, double dist)
{
    Matrix matFrustum = { 0 };
    
    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(zfar - znear);

    matFrustum.m0 = ((float) znear*2.0f)/rl;
    matFrustum.m1 = 0.0f;
    matFrustum.m2 = 0.0f;
    matFrustum.m3 = 0.0f;

    matFrustum.m4 = 0.0f;
    matFrustum.m5 = ((float) znear*2.0f)/tb;
    matFrustum.m6 = 0.0f;
    matFrustum.m7 = 0.0f;

    matFrustum.m8 = ((float)right + (float)left)/rl;
    
    //See: https://docs.lookingglassfactory.com/keyconcepts/camera
    float cameraSize = dist * tan((fov/2.0f) * DEG2RAD);
    matFrustum.m8 += offset/(cameraSize * aspect);
    
    matFrustum.m9 = ((float)top + (float)bottom)/tb;
    matFrustum.m10 = -((float)zfar + (float)znear)/fn;
    matFrustum.m11 = -1.0f;

    matFrustum.m12 = 0.0f;
    matFrustum.m13 = 0.0f;
    matFrustum.m14 = -((float)zfar*(float)znear*2.0f)/fn;
    matFrustum.m15 = 0.0f;
    
    return matFrustum;
}

void BeginMode3DLG(Camera3D camera, float aspect, float offset)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlPushMatrix();                 // Save previous matrix, which contains the settings for the 2d ortho projection
    rlLoadIdentity();               // Reset current matrix (projection)

    // NOTE: zNear and zFar values are important when computing depth buffer values
    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Setup perspective projection
        double top = RL_CULL_DISTANCE_NEAR*tan(camera.fovy*0.5*DEG2RAD);
        double right = top*aspect;

        rlSetMatrixProjection(frustumMatrixOffAxis(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR, offset, aspect, camera.fovy, camera.position.z));
    }

    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)

    // Setup Camera view
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(matView));      // Multiply modelview matrix by view matrix (camera)

    rlEnableDepthTest();            // Enable DEPTH_TEST for 3D
}

Shader gradientShader;
Shader shadowShader;

void DrawCubeAndWires(Vector3 pos, float x, float y, float z, Color c, Shader s) {
    BeginShaderMode(s);
        DrawCube(pos, x, y, z, c);
    EndShaderMode();
    DrawCubeWires(pos, x, y, z, BLACK);
}
void DrawScene() {
    const float BACKDROP_DIST = -4.5f;
    
    float time = GetTime();// * 0.25f;
    Vector3 position = {(float)sin(time), (float)sin(time * 2.0f) * 1.5f, 0.0f};
    DrawCubeAndWires(position, 1.5f, 1.5f, 1.5f, WHITE, gradientShader);
    BeginShaderMode(shadowShader);
        DrawCube(position, 1.5f, 1.5f, 1.5f, WHITE);
    EndShaderMode();
    
    rlPushMatrix();
    rlTranslatef(0, 0, BACKDROP_DIST);
    rlRotatef(90 - LKG_ANGLE, 1, 0, 0);
        DrawPlane({0, 0, 0}, {50.0f, 50.0f}, Color{220, 220, 220, 255});
    rlPopMatrix();
}

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    
    // Window Config
    const int screenWidth = 400;
    const int screenHeight = 400;
    bool fullscreen = false;
    
    // Window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "LKG Quilt Viewer");
    
    // Fix Rectangle UVs (See: https://github.com/raysan5/raylib/issues/1730)
    SetShapesTexture(rlGetTextureDefault(), { 0.0f, 0.0f, 1.0f, 1.0f });
    
    //Load shaders
    Shader lkgFragment = LoadShader(0, "./Shaders/quilt.fs");
    gradientShader = LoadShader("./Shaders/gradient.vs", "./Shaders/gradient.fs");
    SetShaderValue(gradientShader, GetShaderLocation(gradientShader, "startColor"), &(Vector4{ 0.6f, 0, 0, 1.0f }), SHADER_UNIFORM_VEC4);
    SetShaderValue(gradientShader, GetShaderLocation(gradientShader, "endColor"), &(Vector4{ 0.8f, 0, 0, 1.0f }), SHADER_UNIFORM_VEC4);
    
    shadowShader = LoadShader("./Shaders/projection_shadow.vs", "./Shaders/projection_shadow.fs");
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "lightPos"), &(Vector3{ 0.0f, -3.0f, 20.0f }), SHADER_UNIFORM_VEC3);
    float planeZ = -4.0f;
    SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "planeZ"), &planeZ, SHADER_UNIFORM_FLOAT);
    
    // LKG Config
    std::ifstream config_file("display.cfg");
    LKGConfig config = LoadConfig(config_file);
    
    int ri = 0;
    int bi = 2;
    
    float tile[2] = {(float)8,(float)6};
    
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
    std::vector<RenderTexture2D> renderTextures;
    //RenderTexture2D quiltRT = LoadRenderTexture(672, 672);
    //const int TILE_WIDTH = 84;
    //const int TILE_HEIGHT = 112;
    RenderTexture2D quiltRT = LoadRenderTexture(1344, 1344);
    const int TILE_WIDTH = 168;
    const int TILE_HEIGHT = 224;
    //RenderTexture2D quiltRT = LoadRenderTexture(3360, 3360);
    //const int TILE_WIDTH = 420;
    //const int TILE_HEIGHT = 560;
    for (int i = 0; i < 48; i++) {
        renderTextures.push_back(LoadRenderTexture(TILE_WIDTH, TILE_HEIGHT));
    }
    
    // Camera
    Camera3D camera = { 0 };
    camera.position = { 0, 0, LKG_DISTANCE };
    camera.target = { 0, 0, 0 };
    camera.up = { 0, 1.0f, 0 };
    camera.fovy = 17.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(45);               // Set our viewer to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        if (IsKeyPressed(KEY_F)) {
            fullscreen = !fullscreen;
            if (fullscreen) {
                SetWindowState(FLAG_WINDOW_UNDECORATED);
                SetWindowState(FLAG_WINDOW_TOPMOST);
                MaximizeWindow();
                SetWindowSize(1536, 2048);
            } else {
                ClearWindowState(FLAG_WINDOW_UNDECORATED);
                ClearWindowState(FLAG_WINDOW_TOPMOST);
                RestoreWindow();
                SetWindowSize(400, 400);
            }
        }
        if (IsKeyPressed(KEY_S)) {
            TakeScreenshot("./screenshot.jpg");
        }
        
        //camera.position.x = sin(GetTime());

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            for (int i = 0; i < (int)renderTextures.size(); i++) {
                BeginTextureMode(renderTextures[i]);
                
                    ClearBackground(RAYWHITE);
                    
                    float movementAmount = tan((config.viewCone/2.0f) * DEG2RAD) * LKG_DISTANCE;
                    float offset = -movementAmount + ((movementAmount * 2)/(tile[0] * tile[1])) * i;
                    camera.position.x = offset;
                    camera.target.x = offset;
                    
                    BeginMode3DLG(camera, (float)TILE_WIDTH/(float)TILE_HEIGHT, -offset);
                        //Rotate stand angle
                        rlPushMatrix();
                        rlRotatef(LKG_ANGLE, 1, 0, 0);
                            DrawScene();
                        rlPopMatrix();
                    EndMode3D();
                
                EndTextureMode();
            }
            
            BeginTextureMode(quiltRT);
                for (int i = (int)renderTextures.size() - 1; i >= 0; i--) {
                    DrawTextureRec(renderTextures[i].texture,
                            {0, 0, (float)TILE_WIDTH, (float)TILE_HEIGHT},
                            {(float) (i%(int)tile[0])*TILE_WIDTH, (float) (floor(i/(int)tile[0]))*TILE_HEIGHT}, WHITE);
                }
            EndTextureMode();
            
            BeginShaderMode(lkgFragment);
                //DrawTextureRec(quiltRT.texture,
                //        {0, 0, (float)1536, (float)-2048},
                //        {0, 0}, WHITE);
                SetShaderValueTexture(lkgFragment, quiltTexLoc, quiltRT.texture);
                DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(), WHITE);
            EndShaderMode();
                
            DrawFPS(50, 50);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    for (auto rt : renderTextures) {
        UnloadRenderTexture(rt);
    }
    UnloadRenderTexture(quiltRT);
    
    UnloadShader(lkgFragment);
    UnloadShader(gradientShader);
    
    ClearDroppedFiles();
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
