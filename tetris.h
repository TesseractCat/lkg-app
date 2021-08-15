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

enum class Tetromino : unsigned char {
    Shape_O,
    Shape_I,
    Shape_S,
    Shape_Z,
    Shape_L,
    Shape_J,
    Shape_T
};

struct Cell {
    bool empty = true;
    Color color;
};

struct Dropped {
    Tetromino tetromino = Tetromino::Shape_I;

    int posX = 5;
    int posY = 11;
    int rotation = 0;

    constexpr Color GetColor() {
        switch (tetromino) {
            case Tetromino::Shape_O:
                return Color{255, 255, 190, 255};
            case Tetromino::Shape_I:
                return Color{190, 190, 255, 255};
            case Tetromino::Shape_S:
                return Color{255, 190, 190, 255};
            case Tetromino::Shape_Z:
                return Color{190, 255, 190, 255};
            case Tetromino::Shape_L:
                return Color{255, 220, 190, 255};
            case Tetromino::Shape_J:
                return Color{255, 190, 220, 255};
            case Tetromino::Shape_T:
                return Color{225, 225, 225, 255};
        }
    }
    std::array<Vector2, 4> GetCells(int r) {
        std::array<Vector2, 4> cells;
        switch (tetromino) {
            case Tetromino::Shape_O:
                cells = {{Vector2{0,0}, Vector2{1,0}, Vector2{0,-1}, Vector2{1,-1}}};
                break;
            case Tetromino::Shape_I:
                cells = {{Vector2{0,1}, Vector2{0,0}, Vector2{0,-1}, Vector2{0,-2}}};
                break;
            case Tetromino::Shape_S:
                cells = {{Vector2{-1,-1}, Vector2{0,-1}, Vector2{0,0}, Vector2{1,0}}};
                break;
            case Tetromino::Shape_Z:
                cells = {{Vector2{1,-1}, Vector2{0,-1}, Vector2{0,0}, Vector2{-1,0}}};
                break;
            case Tetromino::Shape_L:
                cells = {{Vector2{0,1}, Vector2{0,0}, Vector2{0,-1}, Vector2{1,-1}}};
                break;
            case Tetromino::Shape_J:
                cells = {{Vector2{0,1}, Vector2{0,0}, Vector2{0,-1}, Vector2{-1,-1}}};
                break;
            case Tetromino::Shape_T:
                cells = {{Vector2{-1,0}, Vector2{0,-1}, Vector2{0,0}, Vector2{1,0}}};
                break;
        }
        // Apply rotation
        for (int i = 0; i < 4; i++) {
            // For each 90 degree increment
            for (int j = 0; j < r; j += 90) {
                //Rotate 90 degrees to the right
                cells[i] = Vector2{cells[i].y, -cells[i].x};
            }
        }
        return cells;
    }
    bool CanMove(Cell (&cells)[10][12], Vector2 direction) {
        for (Vector2 cell : GetCells(rotation)) {
            Vector2 newPos = Vector2Add(Vector2Add(cell, direction), Vector2{posX, posY});
            if (newPos.y > (12 - 1))
                continue;
            if (!cells[(int)(newPos.x)][(int)(newPos.y)].empty
                    || newPos.y < 0 || newPos.x < 0 || newPos.x > 9)
                return false;
        }
        return true;
    }

    int NewRotation(bool right) {
        int newRotation = rotation;
        if (right) {
            newRotation += 90;
        } else {
            newRotation -= 90;
        }
        if (newRotation < 0)
            newRotation = 360;
        if (newRotation > 360)
            newRotation = 0;
        return newRotation;
    }
    bool CanRotate(Cell (&cells)[10][12], bool right) {
        for (Vector2 cell : GetCells(NewRotation(right))) {
            Vector2 newPos = Vector2Add(cell, Vector2{posX, posY});
            if (!cells[(int)(newPos.x)][(int)(newPos.y)].empty
                    || newPos.y < 0 || newPos.x < 0 || newPos.x > 9)
                return false;
        }
        return true;
    }
    void Rotate(bool right) {
        rotation = NewRotation(right);
    }

    void Apply(Cell (&cells)[10][12]) {
        for (Vector2 cell : GetCells(rotation)) {
            Vector2 newPos = Vector2Add(cell, Vector2{posX, posY});
            cells[(int)round(newPos.x)][(int)round(newPos.y)] = Cell{ false, GetColor() };
        }
    }
};

class TetrisScene : public Scene
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
    void DrawCubeLines(float size, Color color,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    void DrawBGLines(float size, Color color,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    void DrawSquareLines(float size, Color color,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    void DrawCircleLines(float radius, int segments,
            Matrix* transforms, Vector4* colors, int& instanceIdx);

    // TEXT
    void DrawChar(Matrix m, Color col, char c,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    void DrawText(std::string text, Color col, float scale, float charWidth,
            Matrix* transforms, Vector4* colors, int& instanceIdx);
    float TextWidth(std::string text, float scale);

    // Tetris
    Cell cells[10][12];
    Dropped dropped;
    float dropTime;
    int score = 0;
    Tetromino nextTetromino;
public:
    TetrisScene() {
        std::cout << "[INITIALIZING SCENE]: Tetris" << std::endl;

        Vector3 shadowColor = Vector3{0.0f, 0.0f, 0.0f};
        Vector3 lightPos = Vector3{0.0f, -3.0f, 22.0f};
        float planeZ = -2.0f;
        
        // LINE SHADER ----------
        lineShader = LoadShaderSingleFile("./Shaders/line_instanced.shader");
        lineShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(lineShader, "matModel");
        lineShader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetShaderLocationAttrib(lineShader, "direction");
        lineShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(lineShader, "matView");
        lineShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(lineShader, "matProjection");
        float glow = 0.0f;
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

        //Vector2 atlasSize = Vector2{15, 8};
        Vector2 atlasSize = Vector2{14, 12};
        SetShaderValue(textShader, GetShaderLocation(textShader, "atlasSize"), &atlasSize, SHADER_UNIFORM_VEC2);

        textShader.locs[SHADER_LOC_MAP_ALBEDO] = GetShaderLocation(textShader, "texture1");

        MaterialMap fontAtlasMap = { 0 };
        fontAtlasMap.texture = LoadTexture("./Textures/TerminusAtlas.png");
        fontAtlasMap.color = WHITE;

        textMaterial = LoadMaterialDefault(); // Text material
        textMaterial.shader = textShader;
        textMaterial.maps[0] = fontAtlasMap;

        // MESHES ----------
        quadMesh = GenMeshPlaneY(1.0f, 1.0f, 1, 1);

        // MISC ----------
        nextTetromino = static_cast<Tetromino>(GetRandomValue(0,6));
        dropTime = GetTime();
    }
    ~TetrisScene() {
        UnloadShader(lineShader);
        UnloadShader(textShader);
    }
    void Update() {
        float deltaTime = GetFrameTime();
        float gameTime = GetTime();

        if ((gameTime - dropTime) > (IsKeyDown(KEY_S) ? 0.05f : 0.5f)) {
            dropTime = gameTime;
            if (dropped.CanMove(cells, Vector2{0, -1})) {
                dropped.posY -= 1;
            } else {
                //Apply dropped tetromino
                dropped.Apply(cells);
                dropped = Dropped{nextTetromino, 5, 11, 0};
                nextTetromino = static_cast<Tetromino>(GetRandomValue(0,6));

                if (!dropped.CanMove(cells, Vector2{0, 0})) {
                    std::cout << "[Game Over] - Score: " << std::to_string(score) << std::endl;
                    for (int y = 0; y < 12; y++) {
                        for (int x = 0; x < 10; x++) {
                            cells[x][y].empty = true;
                        }
                    }
                    score = 0;
                }

                //Check for cleared lines
                for (int y = 11; y >= 0; y--) {
                    bool cleared = true;
                    for (int x = 0; x < 10; x++) {
                        if (cells[x][y].empty)
                            cleared = false;
                    }
                    if (cleared) {
                        score += 100;
                        //Clear this line
                        for (int x = 0; x < 10; x++) {
                            cells[x][y].empty = true;
                        }
                        //Clear top row
                        for (int x = 0; x < 10; x++) {
                            cells[x][11].empty = true;
                        }
                        //Shift everything downward
                        for (int sy = y; sy < 12 - 1; sy++) {
                            //Copy above row
                            for (int x = 0; x < 10; x++) {
                                cells[x][sy] = cells[x][sy + 1];
                            }
                        }
                    }
                }
            }
        }
        if (IsKeyDown(KEY_A) && dropped.CanMove(cells, Vector2{-1, 0})) {
            dropped.posX -= 1;
        }
        if (IsKeyDown(KEY_D) && dropped.CanMove(cells, Vector2{1, 0})) {
            dropped.posX += 1;
        }
        if (IsKeyDown(KEY_W) && dropped.CanRotate(cells, true)) {
            dropped.Rotate(true);
        }
    }
    void Draw() {
        float gameTime = GetTime();

        Matrix lineTransforms[1500];
        Vector4 lineColors[1500];
        int lineInstanceIdx = 0;

        Matrix textTransforms[1500];
        Vector4 textColors[1500];
        int textInstanceIdx = 0;

        const float CUBE_WIDTH = 0.36f;

        // Containing box
        LINE_WIDTH = 0.25f;
        rlPushMatrix();
            rlTranslatef(0.0f, -0.35f, 0);
            rlRotatef(-15.0f, 1, 0, 0);
            rlScalef(1.8f, 2.2f, 1.0f * CUBE_WIDTH);

            this->DrawBGLines(1.0f, WHITE,
                    lineTransforms, lineColors, lineInstanceIdx);
        rlPopMatrix();
        LINE_WIDTH = 0.15f;
        
        // Tetrominoes
        rlPushMatrix();
            rlTranslatef(0.0f, -0.35f, 0);
            rlRotatef(-15.0f, 1, 0, 0);
            rlTranslatef(4.5f * -CUBE_WIDTH, -2.2f + 0.5*CUBE_WIDTH, 0);

            rlPushMatrix();
                //Grid
                for (int y = 0; y < 12; y++) {
                    for (int x = 0; x < 10; x++) {
                        //Dot
                        if (y < 11 && x < 9)
                            this->DrawLine(Vector3{CUBE_WIDTH/2.0f,-0.025f + CUBE_WIDTH/2.0f, 0}, Vector3{CUBE_WIDTH/2.0f,0.025f + CUBE_WIDTH/2.0f, 0},
                                    LINE_WIDTH, WHITE, lineTransforms, lineColors, lineInstanceIdx);
                        //Occupied cells
                        if (!cells[x][y].empty) {
                            Color c = cells[x][y].color;
                            rlTranslatef(0, 0, CUBE_WIDTH * 0.5f);
                            this->DrawText(std::string(1, (char)0), c,
                                    CUBE_WIDTH * 1.05f, 1.0f, textTransforms, textColors, textInstanceIdx);
                            rlTranslatef(0, 0, -2.0f * CUBE_WIDTH * 0.5f);
                            this->DrawText(std::string(1, (char)0), Color{c.r-25,c.g-25,c.b-25,c.a},
                                    CUBE_WIDTH * 1.05f, 1.0f, textTransforms, textColors, textInstanceIdx);
                            rlTranslatef(0, 0, CUBE_WIDTH * 0.5f);
                        }
                        rlTranslatef(CUBE_WIDTH, 0, 0);
                    }
                    rlTranslatef(-CUBE_WIDTH * 10, 0, 0);
                    rlTranslatef(0, CUBE_WIDTH, 0);
                }
            rlPopMatrix();

            //Dropped
            rlTranslatef(CUBE_WIDTH * dropped.posX, CUBE_WIDTH * dropped.posY, 0);
            for (Vector2 cellPos : dropped.GetCells(dropped.rotation)) {
                rlPushMatrix();
                rlTranslatef(CUBE_WIDTH * cellPos.x, CUBE_WIDTH * cellPos.y, 0);
                    this->DrawCubeLines(CUBE_WIDTH/2.0f, dropped.GetColor(),
                            lineTransforms, lineColors, lineInstanceIdx);
                rlPopMatrix();
            }
        rlPopMatrix();

        rlPushMatrix();
            rlTranslatef(-1.5f, 2.3f, -0.575f);
            this->DrawText(std::to_string(score), LINE_COLOR, 0.6f, 0.5f,
                    textTransforms, textColors, textInstanceIdx);

            rlTranslatef(3.0f, 0, 0);
            Dropped preview = Dropped{nextTetromino};
            for (Vector2 cell : preview.GetCells(0)) {
                rlPushMatrix();
                    float s = 0.1f;
                    rlTranslatef(cell.x * s, cell.y * s, 0);
                    this->DrawText(std::string(1, (char)0), RAYWHITE,
                            s, 1.0f, textTransforms, textColors, textInstanceIdx);
                rlPopMatrix();
            }
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
    bool ShowFPS() { return false; };
};

/* TEXT DRAWING FUNCTIONS */

void TetrisScene::DrawChar(Matrix m, Color col, char c,
        Matrix* transforms, Vector4* colors, int& instanceIdx) {
    colors[instanceIdx] = ColorNormalize(Color{col.r,col.g,col.b,c/*-32*/});
    transforms[instanceIdx++] = m;
};
void TetrisScene::DrawText(std::string text, Color col, float scale, float charWidth,
        Matrix* transforms, Vector4* colors, int& instanceIdx) {
    rlPushMatrix();
        rlScalef(scale, scale, scale);
        for (char c : text) {
            if (c != 32) {
                this->DrawChar(MatrixMultiply(MatrixScale(charWidth, 1.0f, 1.0f), rlGetMatrixTransform()),
                        col, c, transforms, colors, instanceIdx);
            }
            rlTranslatef(0.4f, 0, 0);
        }
    rlPopMatrix();
};
float TetrisScene::TextWidth(std::string text, float scale) {
    return text.length() * (scale * 0.4f);
}

/* LINE DRAWING FUNCTIONS */

void TetrisScene::DrawLine(Vector3 start, Vector3 end, float width, Color color,
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
void TetrisScene::DrawCubeLines(float s, Color c, Matrix* transforms, Vector4* colors, int& instanceIdx) {
    this->DrawLine(Vector3{s, s, s}, Vector3{-s, s, s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, -s, s}, Vector3{-s, -s, s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, s, -s}, Vector3{-s, s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, -s, -s}, Vector3{-s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, s, s}, Vector3{s, s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, -s, s}, Vector3{s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, s}, Vector3{-s, s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, -s, s}, Vector3{-s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, s, s}, Vector3{s, -s, s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, s, -s}, Vector3{s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, s}, Vector3{-s, -s, s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, -s}, Vector3{-s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
}
void TetrisScene::DrawBGLines(float s, Color c, Matrix* transforms, Vector4* colors, int& instanceIdx) {
    this->DrawLine(Vector3{s, s, -s}, Vector3{-s, s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, -s, -s}, Vector3{-s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);

    this->DrawLine(Vector3{s, s, s}, Vector3{s, -s, s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, s}, Vector3{-s, -s, s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);

    this->DrawLine(Vector3{s, s, s}, Vector3{s, s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, s}, Vector3{-s, s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, -s, s}, Vector3{-s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, -s, s}, Vector3{s, -s, -s}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
}
void TetrisScene::DrawSquareLines(float s, Color c, Matrix* transforms, Vector4* colors, int& instanceIdx) {
    this->DrawLine(Vector3{s, s, 0}, Vector3{-s, s, 0}, LINE_WIDTH, c,
            transforms, colors, instanceIdx); // -
    this->DrawLine(Vector3{s, -s, 0}, Vector3{-s, -s, 0}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{s, s, 0}, Vector3{s, -s, 0}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
    this->DrawLine(Vector3{-s, s, 0}, Vector3{-s, -s, 0}, LINE_WIDTH, c,
            transforms, colors, instanceIdx);
}
void TetrisScene::DrawCircleLines(float radius, int segments, Matrix* transforms, Vector4* colors, int& instanceIdx) {
    for (int i = 0; i < segments; i++) {
        float angle = ((float)i/(float)segments) * PI * 2.0f;
        float next_angle = ((float)(i+1)/(float)segments) * PI * 2.0f;
        auto p = Vector3{cos(angle), sin(angle), 0};
        auto n = Vector3{cos(next_angle), sin(next_angle), 0};
        this->DrawLine(Vector3Scale(p, radius), Vector3Scale(n, radius), LINE_WIDTH, LINE_COLOR,
                transforms, colors, instanceIdx);
    }
}
