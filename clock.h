#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>
#include <ctime>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>

#include "scene.h"
#include "raylib_extensions.h"

class ClockScene : public Scene
{
private:
    Shader litShader;
    Material litMaterial;

    Mesh cubeMesh;
public:
    ClockScene() {
        std::cout << "[INITIALIZING SCENE]: Clock" << std::endl;

        litShader = LoadShaderSingleFile("./Shaders/lit_instanced.shader"); // Lit shader
        litShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(litShader, "matModel");
        litShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(litShader, "colDiffuse");
        litShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(litShader, "matView");
        litShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(litShader, "matProjection");

        Vector3 shadowColor = Vector3{0.8f, 0.8f, 0.8f};
        SetShaderValue(litShader, GetShaderLocation(litShader, "shadowColor"), &shadowColor, SHADER_UNIFORM_VEC3);
        Vector3 lightPos = Vector3{0.0f, -3.0f, 22.0f};
        SetShaderValue(litShader, GetShaderLocation(litShader, "lightPos"), &lightPos, SHADER_UNIFORM_VEC3);
        float planeZ = -2.0f;
        SetShaderValue(litShader, GetShaderLocation(litShader, "planeZ"), &planeZ, SHADER_UNIFORM_FLOAT);

        litMaterial = LoadMaterialDefault(); // Lit material
        litMaterial.shader = litShader;

        cubeMesh = GenMeshCube(1.5f, 1.5f, 1.5f);
        //cubeMesh = GenMeshPlaneY(1.5f, 1.5f, 1, 1);
    }
    ~ClockScene() {
        UnloadShader(litShader);
    }
    void Update() { }
    void Draw() {
        float gameTime = GetTime();// * 0.25f;
        Vector3 position = {(float)sin(gameTime), (float)sin(gameTime * 2.0f) * 1.5f, -2.0f};
        Vector3 position2 = {(float)sin(gameTime * 3.0f), (float)sin(gameTime * 1.5f) * 1.5f, -0.5f};

        std::time_t now = time(nullptr);
        std::tm calender_time = *std::localtime( std::addressof(now) ) ;

        Matrix transforms[500];
        Vector4 colors[500];
        int instanceIdx = 0;

        auto drawCube = [&] (Matrix m, Color c) {
            c.a = 0;
            colors[instanceIdx] = ColorNormalize(c);
            transforms[instanceIdx++] = m;
            c.a = 255;
            colors[instanceIdx] = ColorNormalize(c);
            transforms[instanceIdx++] = m;
        };

        //Cube
        rlPushMatrix();
            rlScalef(0.5f, 0.5f, 0.5f);
            rlTranslatef(position.x * 2, position.y * 2, position.z);
            drawCube(rlGetMatrixTransform(), PINK);
        rlPopMatrix();

        rlPushMatrix();
            rlScalef(0.05f, 0.05f, 1.5f);
            rlTranslatef(0, 0, -0.75f);
            drawCube(rlGetMatrixTransform(), BLACK);
        rlPopMatrix();
        
        //Seconds
        rlPushMatrix();
            rlRotatef(fmod(calender_time.tm_sec, 60.0f)/60.0f * -360.0f, 0, 0, 1);
            rlScalef(0.05f, 1.1f, 0.05f);
            rlTranslatef(0, 0.6666f, 0);
            drawCube(rlGetMatrixTransform(), RED);
        rlPopMatrix();
        //Minutes
        rlPushMatrix();
            rlRotatef(fmod(calender_time.tm_min, 60.0f)/60.0f * -360.0f, 0, 0, 1);
            rlScalef(0.05f, 1.1f, 0.05f);
            rlTranslatef(0, 0.6666f, 1.5f);
            drawCube(rlGetMatrixTransform(), DARKGRAY);
        rlPopMatrix();
        //Hours
        rlPushMatrix();
            rlRotatef(fmod(calender_time.tm_hour % 12, 12.0f)/12.0f * -360.0f, 0, 0, 1);
            rlScalef(0.05f, 0.8f, 0.05f);
            rlTranslatef(0, 0.6666f, -1.5f);
            drawCube(rlGetMatrixTransform(), DARKGRAY);
        rlPopMatrix();

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
};
