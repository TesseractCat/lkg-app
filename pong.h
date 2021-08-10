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

#define BALL_SPEED 3.5f

class PongScene : public Scene
{
private:
    float paddle1X = 0.0f;
    float paddle2X = 0.0f;
    Vector3 pongPosition = Vector3{0,0,0};
    Vector3 pongVelocity = Vector3{-0.9f,-BALL_SPEED,0};
    int player1Score = 0;
    int player2Score = 0;

    float roundStartTime;

    Shader litShader;
    Material litMaterial;
    Mesh cubeMesh;

    Shader textShader;
    Material textMaterial;
    Mesh quadMesh;
public:
    PongScene() {
        std::cout << "[INITIALIZING SCENE]: Pong" << std::endl;

        Vector3 shadowColor = Vector3{0.0f, 0.0f, 0.0f};
        Vector3 lightPos = Vector3{0.0f, -3.0f, 22.0f};
        float planeZ = -2.0f;

        // LIT SHADER ----------
        litShader = LoadShaderSingleFile("./Shaders/lit_instanced.shader");
        litShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(litShader, "matModel");
        litShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(litShader, "colDiffuse");
        litShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(litShader, "matView");
        litShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(litShader, "matProjection");

        SetShaderValue(litShader, GetShaderLocation(litShader, "shadowColor"), &shadowColor, SHADER_UNIFORM_VEC3);
        SetShaderValue(litShader, GetShaderLocation(litShader, "lightPos"), &lightPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(litShader, GetShaderLocation(litShader, "planeZ"), &planeZ, SHADER_UNIFORM_FLOAT);

        litMaterial = LoadMaterialDefault(); // Lit material
        litMaterial.shader = litShader;

        // TEXT SHADER ----------
        textShader = LoadShaderSingleFile("./Shaders/text.shader");
        textShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(textShader, "matModel");
        textShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(textShader, "colDiffuse");
        textShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(textShader, "matView");
        textShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(textShader, "matProjection");

        Vector2 atlasSize = Vector2{15, 8};
        SetShaderValue(textShader, GetShaderLocation(textShader, "atlasSize"), &atlasSize, SHADER_UNIFORM_VEC2);

        textShader.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(textShader, "texture1");

        MaterialMap fontAtlasMap = { 0 };
        fontAtlasMap.texture = LoadTexture("./Textures/FontAtlas.png");
        fontAtlasMap.color = WHITE;

        textMaterial = LoadMaterialDefault(); // Text material
        textMaterial.shader = textShader;
        textMaterial.maps[0] = fontAtlasMap;

        // MESHES ----------
        cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
        quadMesh = GenMeshPlaneY(0.5f, 1.0f, 1, 1);
        
        // MISC ----------
        roundStartTime = GetTime();
    }
    ~PongScene() {
        UnloadShader(litShader);
    }
    void Update() {
        float deltaTime = GetFrameTime();
        if (IsKeyDown(KEY_A)) {
           paddle1X -= deltaTime * 2.0f;
        }
        if (IsKeyDown(KEY_D)) {
           paddle1X += deltaTime * 2.0f;
        }
        if (IsKeyDown(KEY_J)) {
           paddle2X -= deltaTime * 2.0f;
        }
        if (IsKeyDown(KEY_L)) {
           paddle2X += deltaTime * 2.0f;
        }
        if (GetTime() - roundStartTime > 1.0f)
            pongPosition = Vector3Add(pongPosition, Vector3Scale(pongVelocity, deltaTime));
        //Paddle bouncing
        if (pongPosition.y < -2.25f && pongPosition.x > paddle1X - 0.625f && pongPosition.x < paddle1X + 0.625f) {
            pongVelocity.y = -pongVelocity.y;
        }
        if (pongPosition.y > 2.25f && pongPosition.x > paddle2X - 0.625f && pongPosition.x < paddle2X + 0.625f) {
            pongVelocity.y = -pongVelocity.y;
        }
        //Off screen
        if (pongPosition.y < -4.0f || pongPosition.y > 4.0f) {
            if (pongPosition.y > 4.0f)
                player1Score++;
            if (pongPosition.y < -4.0f)
                player2Score++;
            pongPosition = Vector3{0,0,0};
            pongVelocity = Vector3{GetRandomFloat(-1.0f, 1.0f),GetRandomValue(0, 1) == 0 ? BALL_SPEED : -BALL_SPEED, 0};
            paddle1X = 0;
            paddle2X = 0;
            roundStartTime = GetTime();
        }
        //Wall bouncing
        if (pongPosition.x < -2.0f || pongPosition.x > 2.0f) pongVelocity.x = -pongVelocity.x;
    }
    void Draw() {
        rlTranslatef(0, 0, 0.25f);
        float gameTime = GetTime();// * 0.25f;

        std::time_t now = time(nullptr);
        std::tm calender_time = *std::localtime( std::addressof(now) ) ;

        Matrix transforms[10];
        Vector4 colors[10];
        int instanceIdx = 0;

        Matrix textTransforms[20];
        Vector4 textColors[20];
        int textInstanceIdx = 0;

        auto drawCube = [&] (Matrix m, Color c) {
            c.a = 0;
            colors[instanceIdx] = ColorNormalize(c);
            transforms[instanceIdx++] = m;
            c.a = 255;
            colors[instanceIdx] = ColorNormalize(c);
            transforms[instanceIdx++] = m;
        };
        auto drawChar = [&] (Matrix m, Color col, char c) {
            textColors[textInstanceIdx] = ColorNormalize(Color{col.r,col.g,col.b,c-32});
            textTransforms[textInstanceIdx++] = m;
        };
        auto drawText = [&] (std::string text, Color col, float scale) {
            rlPushMatrix();
                rlScalef(scale, scale, scale);
                for (char c : text) {
                    if (c != 0) drawChar(rlGetMatrixTransform(), col, c);
                    rlTranslatef(0.4f, 0, 0);
                }
            rlPopMatrix();
        };

        //Paddle 1
        rlPushMatrix();
            rlScalef(1.25f, 0.25f, 0.5f);
            rlTranslatef(paddle1X * (1.0f/1.25f), -2.5f * (1.0f/0.25f), 0.0f);
            drawCube(rlGetMatrixTransform(), RAYWHITE);
        rlPopMatrix();
        //Paddle 2
        rlPushMatrix();
            rlScalef(1.25f, 0.25f, 0.5f);
            rlTranslatef(paddle2X * (1.0f/1.25f), 2.5f * (1.0f/0.25f), 0.0f);
            drawCube(rlGetMatrixTransform(), RAYWHITE);
        rlPopMatrix();
    
        //Pong ball
        rlPushMatrix();
            rlScalef(0.25f, 0.25f, 0.25f);
            rlTranslatef(pongPosition.x * (1.0f/0.25f),pongPosition.y * (1.0f/0.25f),pongPosition.z * (1.0f/0.25f));
            drawCube(rlGetMatrixTransform(), RAYWHITE);
        rlPopMatrix();

        DrawMeshInstancedC(cubeMesh, litMaterial, transforms, colors, instanceIdx);

        //Score
        drawText("HeLo ThEr", WHITE, 0.4f);

        //Player 1
        rlPushMatrix();
        rlTranslatef(-1.6f, 0.5f, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player2Score);
        rlTranslatef(0, 0, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player2Score);
        rlTranslatef(0, 0, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player2Score);
        rlTranslatef(0, 0, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player2Score);
        rlPopMatrix();
        //Player 2
        rlPushMatrix();
        rlTranslatef(1.6f, -0.5f, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player1Score);
        rlTranslatef(0, 0, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player1Score);
        rlTranslatef(0, 0, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player1Score);
        rlTranslatef(0, 0, -0.5f);
            drawChar(rlGetMatrixTransform(), WHITE, '0' + player1Score);
        rlPopMatrix();

        DrawMeshInstancedC(quadMesh, textMaterial, textTransforms, textColors, textInstanceIdx);
    }

    Color GetClearColor() {
        return Color{20,20,20,255};
    }
    std::pair<int, int> GetTileResolution() {
        //return std::pair<int, int>(126, 168);
        return std::pair<int, int>(168, 224);
        //return std::pair<int, int>(315, 420);
        //return std::pair<int, int>(420, 560);
    }
};
