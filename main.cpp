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

void DrawFPSSize(int posX, int posY, int fontSize)
{
    Color color = LIME; // good fps
    int fps = GetFPS();

    if (fps < 30 && fps >= 15) color = ORANGE;  // warning FPS
    else if (fps < 15) color = RED;    // bad FPS

    DrawText(TextFormat("%2i FPS", GetFPS()), posX, posY, fontSize, color);
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
    //rlDrawRenderBatchActive();      // Update and draw internal render batch

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
void EndMode3DLG() {
    rlDrawRenderBatchActive();      // Update and draw internal render batch

    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlPopMatrix();                  // Restore previous matrix (projection) from matrix stack

    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)
}

Shader litShader;
int litShaderShadowLoc;

Material litMaterial;
Mesh cubeMesh;

void DrawCubeAndWires(Vector3 pos, float x, float y, float z, Color c, Shader s) {
    BeginShaderMode(s);
        DrawCube(pos, x, y, z, c);
    EndShaderMode();
    //DrawCubeWires(pos, x, y, z, BLACK);
}
void DrawScene(bool shadow) {
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

    //Seconds
    litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    rlPushMatrix();
	rlRotatef(fmod(calender_time.tm_sec, 60.0f)/60.0f * -360.0f, 0, 0, 1);
        rlScalef(0.03f, 1.25f, shadow ? 1.0f : 0.03f);
        rlTranslatef(0, 0.6666f, 0);
        DrawMesh(cubeMesh, litMaterial, MatrixIdentity());
    rlPopMatrix();
    //Minutes
    litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    rlPushMatrix();
	rlRotatef(fmod(calender_time.tm_min, 60.0f)/60.0f * -360.0f, 0, 0, 1);
        rlScalef(0.03f, 1.25f, shadow ? 1.0f : 0.2f);
        rlTranslatef(0, 0.6666f, 0);
        DrawMesh(cubeMesh, litMaterial, MatrixIdentity());
    rlPopMatrix();
    //Hours
    litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    rlPushMatrix();
	rlRotatef(fmod(calender_time.tm_hour % 12, 12.0f)/12.0f * -360.0f, 0, 0, 1);
        rlScalef(0.03f, 1.25f, shadow ? 1.0f : 0.2f);
        rlTranslatef(0, 0.6666f, 0);
        DrawMesh(cubeMesh, litMaterial, MatrixIdentity());
    rlPopMatrix();

    for (int i = 0; i < 12; i++) {
        rlPushMatrix();
            rlScalef(0.1f, 0.1f, shadow ? 1.0f : 0.1f);
            rlRotatef((i/12.0f) * 360.0f, 0, 0, 1);
            rlTranslatef(20.0f, 0, 0);
            DrawMesh(cubeMesh, litMaterial, MatrixIdentity());
        rlPopMatrix();
    }
}

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    
    // Window Config
    const int screenWidth = 1536;
    const int screenHeight = 2048;
    
    // Window
    InitWindow(screenWidth, screenHeight, "LKG Quilt Viewer");
    
    // Fix Rectangle UVs (See: https://github.com/raysan5/raylib/issues/1730)
    Texture2D texture = { rlGetTextureIdDefault(), 1, 1, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    SetShapesTexture(texture, Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });
    // SetShapesTexture(rlGetTextureDefault(), Rectangle{ 0.0f, 0.0f, 1.0f, 1.0f });
    
    //Load shaders
    Shader lkgFragment = LoadShader("./Shaders/quilt.vs", "./Shaders/quilt.fs"); // Quilt shader

    litShader = LoadShader("./Shaders/lit.vs", "./Shaders/lit.fs"); // Lit shader
    litShaderShadowLoc = GetShaderLocation(litShader, "shadow");
    litShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(litShader, "matModel");
    litShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(litShader, "matView");
    litShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(litShader, "matProjection");

    Vector3 lightPos = Vector3{0.0f, -3.0f, 22.0f};
    SetShaderValue(litShader, GetShaderLocation(litShader, "lightPos"), &lightPos, SHADER_UNIFORM_VEC3);
    float planeZ = -3.5f;
    SetShaderValue(litShader, GetShaderLocation(litShader, "planeZ"), &planeZ, SHADER_UNIFORM_FLOAT);
    int zero = 0;
    int one = 1;
    SetShaderValue(litShader, litShaderShadowLoc, &zero, SHADER_UNIFORM_INT);

    litMaterial = LoadMaterialDefault();
    litMaterial.shader = litShader;
    litMaterial.maps[MATERIAL_MAP_DIFFUSE].color = RED;

    //Load meshes
    cubeMesh = GenMeshCube(1.5f, 1.5f, 1.5f);
    
    // LKG Config
    std::ifstream config_file("display.cfg");
    LKGConfig config = LoadConfig(config_file);
    
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
    
    //SetTargetFPS(30);               // Set our viewer to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        if (IsKeyPressed(KEY_S)) {
            TakeScreenshot("./screenshot.jpg");
        }
        
        //camera.position.x = sin(GetTime());

        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(quiltRT);
            ClearBackground(RAYWHITE);
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
		    SetShaderValue(litShader, litShaderShadowLoc, &zero, SHADER_UNIFORM_INT);
	    	    BeginShaderMode(litShader);
            	        DrawScene(false);
	            EndShaderMode();

		    SetShaderValue(litShader, litShaderShadowLoc, &one, SHADER_UNIFORM_INT);
	    	    BeginShaderMode(litShader);
            	        DrawScene(true);
	            EndShaderMode();
            	rlPopMatrix();
                EndMode3D();
            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(RAYWHITE);
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
