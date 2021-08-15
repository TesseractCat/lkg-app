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

class GraphScene : public Scene
{
private:
    Shader lineShader;
    Material lineMaterial;

    Shader textShader;
    Material textMaterial;

    Mesh quadMesh;

    float PackColor(Vector4 color) {
       return floor(color.x * 128.0f + 0.5f)
        + floor(color.z * 128.0f + 0.5f) * 129.0f
        + floor(color.y * 128.0f + 0.5f) * 129.0f * 129.0f;
    }

    // LINE
    float LINE_WIDTH = 0.15f;
    float GRAPH_SEGMENT = 0.125f;
    Color LINE_COLOR = Color{255,255,255,255};//Color{38,182,128,255};
    //const Color LINE_COLOR = Color{38,182,128,255};
    void DrawLine(Vector3 start, Vector3 end, float width, Color color,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    void DrawCubeLines(float size,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    void DrawCircleLines(float radius, int segments,
            Matrix* transforms, Vector4* colors, int& instanceIdx);

    // TEXT
    void DrawChar(Matrix m, Color col, char c,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    void DrawText(std::string text, Color col, float scale,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
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
        float glow = 0.7f;
        float glowFalloff = 15.0f;
        SetShaderValue(lineShader, GetShaderLocation(lineShader, "glow"), &glow, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lineShader, GetShaderLocation(lineShader, "glowFalloff"), &glowFalloff, SHADER_UNIFORM_FLOAT);

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

        Matrix lineTransforms[1500];
        Vector4 lineColors[1500];
        int lineInstanceIdx = 0;

        Matrix textTransforms[1500];
        Vector4 textColors[1500];
        int textInstanceIdx = 0;

        rlPushMatrix();
            float space = 0.4f;
            rlTranslatef(0.8f, -1.5f, -2.0f * space);
            for (int i = -2; i <= 2; i++) {
                rlTranslatef(0, 0, space);
                //rlRotatef(((i+5)/10.0f) * 180.0f, 0, 1, 0);
                this->DrawCircleLines(0.6f + sin(gameTime + i * space) * 0.3f, 18,
                        lineTransforms, lineColors, lineInstanceIdx);
            }
        rlPopMatrix();

        rlPushMatrix();
            rlTranslatef(0, 1.25f, 0);
            for (float x = -1.8f; x <= 1.8f; x += GRAPH_SEGMENT) {
                float a = x*3.0f + gameTime;
                float b = (x + GRAPH_SEGMENT)*3.0f + gameTime;
                this->DrawLine(Vector3{x, sin(a), cos(a)},
                        Vector3{x + GRAPH_SEGMENT, sin(b), cos(b)}, LINE_WIDTH, LINE_COLOR,
                        lineTransforms, lineColors, lineInstanceIdx);
            }
        rlPopMatrix();
        rlPushMatrix();
            rlTranslatef(0.0f, -1.25f, 0);
            rlRotatef(gameTime * 5.0f, 1, 1, 1);
                this->DrawCubeLines(0.6f,
                        lineTransforms, lineColors, lineInstanceIdx);
        rlPopMatrix();
        rlPushMatrix();
            rlScalef(1.8f, 2.2f, 1.0f);
            this->DrawCubeLines(1.0f,
                    lineTransforms, lineColors, lineInstanceIdx);
        rlPopMatrix();

        rlPushMatrix();
            rlTranslatef(-1.35f, 1.0f, 0);
            this->DrawText("Hello world", LINE_COLOR, 0.7f,
                    textTransforms, textColors, textInstanceIdx);
        rlPopMatrix();

        // Lines
        //BeginBlendMode(BLEND_ADDITIVE);
        DrawMeshInstancedC(quadMesh, lineMaterial, lineTransforms, lineColors, lineInstanceIdx);
        DrawMeshInstancedC(quadMesh, textMaterial, textTransforms, textColors, textInstanceIdx);
    }

    Color GetClearColor() {
        //return Color{220,220,220,255};
        return Color{0,0,0,255};
    }
    std::pair<int, int> GetTileResolution() {
        //return std::pair<int, int>(168, 224);
        return std::pair<int, int>(252, 336);
        //return std::pair<int, int>(315, 420);
        //return std::pair<int, int>(420, 560);
    }
    std::pair<float, float> GetAngleDistance() { return std::pair<float, float>(30.0f, 20.0f); }
};

/* TEXT DRAWING FUNCTIONS */

void GraphScene::DrawChar(Matrix m, Color col, char c,
        Matrix* transforms, Vector4* colors, int& instanceIdx) {
    colors[instanceIdx] = ColorNormalize(Color{col.r,col.g,col.b,c-32});
    transforms[instanceIdx++] = MatrixMultiply(MatrixScale(0.5f, 1.0f, 1.0f), m);
};
void GraphScene::DrawText(std::string text, Color col, float scale,
        Matrix* transforms, Vector4* colors, int& instanceIdx) {
    rlPushMatrix();
        rlScalef(scale, scale, scale);
        for (char c : text) {
            if (c != 0) this->DrawChar(rlGetMatrixTransform(), col, c,
                    transforms, colors, instanceIdx);
            rlTranslatef(0.4f, 0, 0);
        }
    rlPopMatrix();
};

/* LINE DRAWING FUNCTIONS */

void GraphScene::DrawLine(Vector3 start, Vector3 end, float width, Color color,
        Matrix* transforms, Vector4* colors, int& instanceIdx) {
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
    colors[instanceIdx] = Vector4{screenDir.x,screenDir.y,width,this->PackColor(ColorNormalize(color))};
    transforms[instanceIdx++] = MatrixMultiply(matTransform, rlGetMatrixTransform());
}
void GraphScene::DrawCubeLines(float s, Matrix* transforms, Vector4* colors, int& instanceIdx) {
    this->DrawLine(Vector3{s, s, s}, Vector3{-s, s, s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, -s, s}, Vector3{-s, -s, s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, s, -s}, Vector3{-s, s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, -s, -s}, Vector3{-s, -s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, s, s}, Vector3{s, s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, -s, s}, Vector3{s, -s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, s}, Vector3{-s, s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, -s, s}, Vector3{-s, -s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, s, s}, Vector3{s, -s, s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, s, -s}, Vector3{s, -s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, s}, Vector3{-s, -s, s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, -s}, Vector3{-s, -s, -s}, LINE_WIDTH, LINE_COLOR,
            transforms, colors, instanceIdx);
}
void GraphScene::DrawCircleLines(float radius, int segments, Matrix* transforms, Vector4* colors, int& instanceIdx) {
    for (int i = 0; i < segments; i++) {
        float angle = ((float)i/(float)segments) * PI * 2.0f;
        float next_angle = ((float)(i+1)/(float)segments) * PI * 2.0f;
        auto p = Vector3{cos(angle), sin(angle), 0};
        auto n = Vector3{cos(next_angle), sin(next_angle), 0};
        this->DrawLine(Vector3Scale(p, radius), Vector3Scale(n, radius), LINE_WIDTH, LINE_COLOR,
                transforms, colors, instanceIdx);
    }
}
