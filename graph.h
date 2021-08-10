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

float packColor(Vector4 color) {
   return floor(color.x * 128.0f + 0.5f)
   	+ floor(color.z * 128.0f + 0.5f) * 129.0f
   	+ floor(color.y * 128.0f + 0.5f) * 129.0f * 129.0f;
}

class GraphScene : public Scene
{
private:
    Shader lineShader;
    Material lineMaterial;

    Shader textShader;
    Material textMaterial;

    Mesh quadMesh;
public:
    GraphScene() {
        std::cout << "[INITIALIZING SCENE]: Graph" << std::endl;

        Vector3 shadowColor = Vector3{0.0f, 0.0f, 0.0f};
        Vector3 lightPos = Vector3{0.0f, -3.0f, 22.0f};
        float planeZ = -2.0f;
        
        // LINE SHADER ----------
        lineShader = LoadShaderSingleFile("./Shaders/line_instanced.shader");
        lineShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(lineShader, "matModel");
        lineShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(lineShader, "direction");
        lineShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(lineShader, "matView");
        lineShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(lineShader, "matProjection");

        lineMaterial = LoadMaterialDefault(); // Line material
        lineMaterial.shader = lineShader;

        rlDisableBackfaceCulling();

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
        quadMesh = GenMeshPlaneY(1.0f, 1.0f, 1, 1);
    }
    ~GraphScene() {
        UnloadShader(lineShader);
        UnloadShader(textShader);
    }
    void Update() {
        float deltaTime = GetFrameTime();
    }
    void Draw() {
        float gameTime = GetTime() * 2.0f;

        Matrix transforms[1500];
        Vector4 colors[1500];
        int instanceIdx = 0;

        Matrix textTransforms[1500];
        Vector4 textColors[1500];
        int textInstanceIdx = 0;

        auto drawLine = [&] (Vector3 start, Vector3 end, float width, Color color) {
            Vector3 midpoint = Vector3Lerp(start, end, 0.5f);
            Vector3 direction = Vector3Normalize(Vector3Subtract(end, start));
            float distance = Vector3Distance(end, start);

            // Matrix Transformation
            Matrix matTranslation = MatrixTranslate(midpoint.x, midpoint.y, midpoint.z);
            Matrix matScale = MatrixScale(/*width*/0.0f, distance, 1.0f);

            Quaternion fromToRotation = QuaternionFromVector3ToVector3(Vector3{0,1.0f,0}, direction);
            Matrix matRotation = QuaternionToMatrix(fromToRotation);
            Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

            // Line screen direction
            Matrix matModelView = MatrixMultiply(rlGetMatrixTransform(), rlGetMatrixModelview());
            Matrix mvp = MatrixMultiply(matModelView, rlGetMatrixProjection());

            Vector4 projStart = Vector4Transform(Vector4{start.x,start.y,start.z,1.0f}, mvp);
            Vector2 screenStart = Vector2Scale(Vector2{projStart.x, projStart.y}, 1.0f/projStart.w);
            screenStart.x *= 1536.0f/2048.0f;
            Vector4 projEnd = Vector4Transform(Vector4{end.x,end.y,end.z,1.0f}, mvp);
            Vector2 screenEnd = Vector2Scale(Vector2{projEnd.x, projEnd.y}, 1.0f/projEnd.w);
            screenEnd.x *= 1536.0f/2048.0f;
            Vector2 screenDir = Vector2Normalize(Vector2Subtract(screenEnd, screenStart));

            // Instance values
            colors[instanceIdx] = Vector4{screenDir.x,screenDir.y,width,packColor(ColorNormalize(color))};
            transforms[instanceIdx++] = MatrixMultiply(matTransform, rlGetMatrixTransform());
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

        const float LINE_WIDTH = 0.15f;
        const float GRAPH_SEGMENT = 0.1f;
        const Color LINE_COLOR = Color{48,192,178,255};
        for (float x = -2.1f; x <= 2.1f; x += GRAPH_SEGMENT) {
            float a = x*3.0f + gameTime;
            float b = (x + GRAPH_SEGMENT)*3.0f + gameTime;
            drawLine(Vector3{x, sin(a), cos(a)},
                    Vector3{x + GRAPH_SEGMENT, sin(b), cos(b)}, LINE_WIDTH, LINE_COLOR);
        }

        auto drawWireCube = [&] (float s) {
            drawLine(Vector3{s, s, s}, Vector3{-s, s, s}, LINE_WIDTH, LINE_COLOR); // -
            drawLine(Vector3{s, -s, s}, Vector3{-s, -s, s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{s, s, -s}, Vector3{-s, s, -s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{s, -s, -s}, Vector3{-s, -s, -s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{s, s, s}, Vector3{s, s, -s}, LINE_WIDTH, LINE_COLOR); // -
            drawLine(Vector3{s, -s, s}, Vector3{s, -s, -s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{-s, s, s}, Vector3{-s, s, -s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{-s, -s, s}, Vector3{-s, -s, -s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{s, s, s}, Vector3{s, -s, s}, LINE_WIDTH, LINE_COLOR); // -
            drawLine(Vector3{s, s, -s}, Vector3{s, -s, -s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{-s, s, s}, Vector3{-s, -s, s}, LINE_WIDTH, LINE_COLOR);
            drawLine(Vector3{-s, s, -s}, Vector3{-s, -s, -s}, LINE_WIDTH, LINE_COLOR);
        };

        rlPushMatrix();
        rlRotatef(gameTime * 5.0f, 1, 1, 1);
            drawWireCube(1.0f + sin(gameTime) * 0.5f);
            //drawWireCube(1.0f + sin(gameTime + 0.8f) * 0.3f);
            //drawWireCube(1.0f + sin(gameTime + 1.9f) * 0.4f);
        rlPopMatrix();

        // Lines
        DrawMeshInstancedC(quadMesh, lineMaterial, transforms, colors, instanceIdx);
    }

    Color GetClearColor() {
        return Color{0,0,0,255};
    }
    std::pair<int, int> GetTileResolution() {
        //return std::pair<int, int>(126, 168);
        return std::pair<int, int>(168, 224);
        //return std::pair<int, int>(315, 420);
        //return std::pair<int, int>(420, 560);
    }
    std::pair<float, float> GetAngleDistance() { return std::pair<float, float>(25.0f, 20.0f); }
};
