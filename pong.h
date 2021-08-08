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

class PongScene : public Scene
{
private:
    float paddle1X = 0.0f;
    float paddle2X = 0.0f;

    Shader litShader;
    Material litMaterial;

    Mesh cubeMesh;
public:
    PongScene() {
        std::cout << "[INITIALIZING SCENE]: Pong" << std::endl;

        litShader = LoadShaderSingleFile("./Shaders/lit_instanced.shader"); // Lit shader
        litShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(litShader, "matModel");
        litShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(litShader, "colDiffuse");
        litShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(litShader, "matView");
        litShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(litShader, "matProjection");

        Vector3 shadowColor = Vector3{0.0f, 0.0f, 0.0f};
        SetShaderValue(litShader, GetShaderLocation(litShader, "shadowColor"), &shadowColor, SHADER_UNIFORM_VEC3);
        Vector3 lightPos = Vector3{0.0f, -3.0f, 22.0f};
        SetShaderValue(litShader, GetShaderLocation(litShader, "lightPos"), &lightPos, SHADER_UNIFORM_VEC3);
        float planeZ = -2.0f;
        SetShaderValue(litShader, GetShaderLocation(litShader, "planeZ"), &planeZ, SHADER_UNIFORM_FLOAT);

        litMaterial = LoadMaterialDefault(); // Lit material
        litMaterial.shader = litShader;

        cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    }
    ~PongScene() {
        UnloadShader(litShader);
    }
    void Update() {
        if (IsKeyDown(KEY_A)) {
           paddle1X -= GetFrameTime() * 2.0f;
	}
        if (IsKeyDown(KEY_D)) {
           paddle1X += GetFrameTime() * 2.0f;
	}
        if (IsKeyDown(KEY_J)) {
           paddle2X -= GetFrameTime() * 2.0f;
	}
        if (IsKeyDown(KEY_L)) {
           paddle2X += GetFrameTime() * 2.0f;
	}
    }
    void Draw() {
	BeginShaderMode(litShader);

        float game_time = GetTime();// * 0.25f;
        Vector3 position = {(float)sin(game_time), (float)sin(game_time * 2.0f) * 1.5f, -1.0f};
        Vector3 position2 = {(float)sin(game_time * 3.0f), (float)sin(game_time * 1.5f) * 1.5f, -0.5f};

        std::time_t now = time(nullptr);
        std::tm calender_time = *std::localtime( std::addressof(now) ) ;

        Matrix transforms[20];
        Color colors[20];
        int instanceIdx = 0;

        auto drawCube = [&] (Matrix m, Color c) {
            c.a = 0;
            colors[instanceIdx] = c;
            transforms[instanceIdx++] = m;
            c.a = 255;
            colors[instanceIdx] = c;
            transforms[instanceIdx++] = m;
        };

        //Paddle 1
        rlPushMatrix();
            rlScalef(1.25f, 0.25f, 0.25f);
            rlTranslatef(paddle1X * (1.0f/1.25f), -2.5f * (1.0f/0.25f), 0.0f);
            drawCube(rlGetMatrixTransform(), RAYWHITE);
        rlPopMatrix();
        //Paddle 2
        rlPushMatrix();
            rlScalef(1.25f, 0.25f, 0.25f);
            rlTranslatef(paddle2X * (1.0f/1.25f), 2.5f * (1.0f/0.25f), 0.0f);
            drawCube(rlGetMatrixTransform(), RAYWHITE);
        rlPopMatrix();

        DrawMeshInstancedC(cubeMesh, litMaterial, transforms, colors, instanceIdx);

	EndShaderMode();
    }

    Color GetClearColor() {
        return Color{20,20,20,255};
    }
};
