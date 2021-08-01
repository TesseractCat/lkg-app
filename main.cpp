extern "C" {
    #include "raylib.h"
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

struct LKGConfig {
    float pitch;
    float slope;
    float center;
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
    int invView = 1;
    config_file.ignore(256, '=');
    config_file >> out.invView;
    float dpi = 324.0;
    config_file.ignore(256, '=');
    config_file >> out.dpi;
    
    return out;
}

void DrawCubeAndWires(Vector3 pos, float x, float y, float z, Color c) {
    DrawCube(pos, x, y, z, c);
    DrawCubeWires(pos, x, y, z, BLACK);
}

void DrawScene() {
    DrawCubeAndWires({0, 0, 0}, 1.0f, 1.0f, 1.0f, RED);
    DrawCubeAndWires({(float)sin(GetTime()), 1.5f, -2.5f}, 1.0f, 1.0f, 1.0f, BLUE);
    DrawCubeAndWires({0, -1.5f, 0.0f + (float)sin(GetTime())}, 1.0f, 1.0f, 1.0f, GREEN);
    DrawCubeAndWires({0, 1.5f + (float)sin(GetTime()), 0.5f}, 2.0f, 0.5f, 0.1f, DARKGRAY);
    
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
    float cameraSize = dist * tan((fov/2.0f) * (0.0174533f));
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

    //float aspect = (float)CORE.Window.currentFbo.width/(float)CORE.Window.currentFbo.height;

    // NOTE: zNear and zFar values are important when computing depth buffer values
    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Setup perspective projection
        double top = RL_CULL_DISTANCE_NEAR*tan(camera.fovy*0.5*DEG2RAD);
        double right = top*aspect;

        rlSetMatrixProjection(frustumMatrixOffAxis(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR, offset, aspect, camera.fovy, 20.0f));
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic projection
        double top = camera.fovy/2.0;
        double right = top*aspect;

        rlOrtho(-right, right, -top,top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }

    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)

    // Setup Camera view
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(matView));      // Multiply modelview matrix by view matrix (camera)

    rlEnableDepthTest();            // Enable DEPTH_TEST for 3D
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
    SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "LKG Quilt Viewer");
    
    // Fix Rectangle UVs (See: https://github.com/raysan5/raylib/issues/1730)
    SetShapesTexture(rlGetTextureDefault(), { 0.0f, 0.0f, 1.0f, 1.0f });
    
    // LKG Config
    Shader lkgFragment = LoadShader(0, "./quilt.fs");
    
    std::ifstream config_file("display.cfg");
    LKGConfig config = LoadConfig(config_file);
    
    int ri = 0;
    int bi = 2;
    
    float tile[2] = {(float)8,(float)6};
    
    // Initialize shader uniforms
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
    RenderTexture2D quiltRT = LoadRenderTexture(1536, 2048);
    for (int i = 0; i < 48; i++) {
        renderTextures.push_back(LoadRenderTexture(192, 341));
    }
    
    // Camera
    Camera3D camera = { 0 };
    camera.position = { 0, 0.0f, 20.0f };
    camera.target = { 0, 0, 0 };
    camera.up = { 0, 1.0f, 0 };
    camera.fovy = 17.0f;//17.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);               // Set our viewer to run at 60 frames-per-second
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
                
                    //ClearBackground({(unsigned char) (i*5), (unsigned char)i, 0, 255});
                    ClearBackground(RAYWHITE);
                    
                    float movementAmount = 10.0f;
                    float offset = -movementAmount + ((movementAmount * 2)/48.0) * i;
                    camera.position.x = offset;
                    camera.target.x = offset;
                    BeginMode3DLG(camera, 192.0f/341.0f, -offset);
                        DrawScene();
                    EndMode3D();
                
                EndTextureMode();
            }
            
            BeginTextureMode(quiltRT);
                for (int i = (int)renderTextures.size() - 1; i >= 0; i--) {
                    DrawTextureRec(renderTextures[i].texture,
                            {0, 0, (float)192, (float)341},
                            {(float) (i%8)*192, (float) (floor(i/8))*341}, WHITE);
                }
            EndTextureMode();
            
            BeginShaderMode(lkgFragment);
                DrawTextureRec(quiltRT.texture,
                        {0, 0, (float)1536, (float)-2048},
                        {0, 0}, WHITE);
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
    ClearDroppedFiles();
    CloseWindow();
    //--------------------------------------------------------------------------------------

    return 0;
}
