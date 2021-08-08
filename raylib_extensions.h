#include "rlgl.h"

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

void DrawFPSSize(int posX, int posY, int fontSize)
{
    Color color = LIME; // good fps
    int fps = GetFPS();

    if (fps < 30 && fps >= 15) color = ORANGE;  // warning FPS
    else if (fps < 15) color = RED;    // bad FPS

    DrawText(TextFormat("%2i FPS", GetFPS()), posX, posY, fontSize, color);
}

void DrawCubeAndWires(Vector3 pos, float x, float y, float z, Color c, Shader s) {
    BeginShaderMode(s);
        DrawCube(pos, x, y, z, c);
    EndShaderMode();
    DrawCubeWires(pos, x, y, z, BLACK);
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

typedef struct float4 {
    float v[4];
} float4;

void DrawMeshInstancedSimple(Mesh mesh, Material material, Matrix *transforms, Color *colors, int instances)
{
    // Check instancing
    if (instances <= 1) return;

    int MAX_MATERIAL_MAPS = 12;

    float16 *instanceTransforms = NULL;
    float4 *instanceColors = NULL;
    unsigned int instanceTransformsVboId = 0;
    unsigned int instanceColorsVboId = 0;

    // Bind shader program
    rlEnableShader(material.shader.id);

    // Send required data to shader (matrices, values)
    //-----------------------------------------------------
    // Upload to shader material.colDiffuse
    /*if (material.shader.locs[SHADER_LOC_COLOR_DIFFUSE] != -1)
    {
        float values[4] = {
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.r/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.g/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.b/255.0f,
            (float)material.maps[MATERIAL_MAP_DIFFUSE].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
    }*/

    // Upload to shader material.colSpecular (if location available)
    if (material.shader.locs[SHADER_LOC_COLOR_SPECULAR] != -1)
    {
        float values[4] = {
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.r/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.g/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.b/255.0f,
            (float)material.maps[SHADER_LOC_COLOR_SPECULAR].color.a/255.0f
        };

        rlSetUniform(material.shader.locs[SHADER_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Get a copy of current matrices to work with,
    // just in case stereo render is required and we need to modify them
    // NOTE: At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (material.shader.locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Create instances buffer
    instanceTransforms = (float16 *)RL_MALLOC(instances*sizeof(float16));
    instanceColors = (float4 *)RL_MALLOC(instances*sizeof(float4));
    
    // Fill buffer with instances transformations as float16 arrays
    for (int i = 0; i < instances; i++) instanceTransforms[i] = MatrixToFloatV(transforms[i]);
    for (int i = 0; i < instances; i++) {
	float4 c = { 0 };
	c.v[0] = (float)colors[i].r/255.0f;
	c.v[1] = (float)colors[i].g/255.0f;
	c.v[2] = (float)colors[i].b/255.0f;
	c.v[3] = (float)colors[i].a/255.0f;
        instanceColors[i] = c;
    }
    
    // Enable mesh VAO to attach new buffer
    rlEnableVertexArray(mesh.vaoId);
    
    // This could alternatively use a static VBO and either glMapBuffer() or glBufferSubData().
    // It isn't clear which would be reliably faster in all cases and on all platforms,
    // anecdotally glMapBuffer() seems very slow (syncs) while glBufferSubData() seems
    // no faster, since we're transferring all the transform matrices anyway
    instanceTransformsVboId = rlLoadVertexBuffer(instanceTransforms, instances*sizeof(float16), false);
    
    // Instances transformation matrices are send to shader attribute location: SHADER_LOC_MATRIX_MODEL
    for (unsigned int i = 0; i < 4; i++)
    {
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 4, RL_FLOAT, 0, sizeof(Matrix), (void *)(i*sizeof(Vector4)));
        rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_MATRIX_MODEL] + i, 1);
    }

    instanceColorsVboId = rlLoadVertexBuffer(instanceColors, instances*sizeof(float4), false);
    rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE]);
    rlSetVertexAttribute(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], 4, RL_FLOAT, 0, sizeof(float4), 0);
    rlSetVertexAttributeDivisor(material.shader.locs[SHADER_LOC_COLOR_DIFFUSE], 1);
    
    rlDisableVertexBuffer();
    rlDisableVertexArray();
    
    // Accumulate internal matrix transform (push/pop) and view matrix
    // NOTE: In this case, model instance transformation must be computed in the shader
    matModelView = MatrixMultiply(rlGetMatrixTransform(), matView);

    // Upload model normal matrix (if locations available)
    if (material.shader.locs[SHADER_LOC_MATRIX_NORMAL] != -1) rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    /*for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Enable texture for active slot
            if ((i == MATERIAL_MAP_IRRADIANCE) ||
                (i == MATERIAL_MAP_PREFILTER) ||
                (i == MATERIAL_MAP_CUBEMAP)) rlEnableTextureCubemap(material.maps[i].texture.id);
            else rlEnableTexture(material.maps[i].texture.id);

            rlSetUniform(material.shader.locs[SHADER_LOC_MAP_DIFFUSE + i], &i, SHADER_UNIFORM_INT, 1);
        }
    }*/

    // Try binding vertex array objects (VAO)
    // or use VBOs if not possible
    //rlEnableVertexArray(mesh.vaoId);
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[1]);
        rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD01]);

        if (material.shader.locs[SHADER_LOC_VERTEX_NORMAL] != -1)
        {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(mesh.vboId[2]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_COLOR] != -1)
        {
            if (mesh.vboId[3] != 0)
            {
                rlEnableVertexBuffer(mesh.vboId[3]);
                rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for unused attribute
                // NOTE: Required when using default shader and no VAO support
                float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                rlSetVertexAttributeDefault(material.shader.locs[SHADER_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC2, 4);
                rlDisableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TANGENT] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[4]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[5]);
            rlSetVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(material.shader.locs[SHADER_LOC_VERTEX_TEXCOORD02]);
        }

        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
    }

    // Calculate model-view-projection matrix (MVP)
    Matrix matModelViewProjection = MatrixIdentity();
    matModelViewProjection = MatrixMultiply(matModelView, matProjection);
    
    // Send combined model-view-projection matrix to shader
    rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);
    
    if (mesh.indices != NULL) rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount*3, 0, instances);
    else rlDrawVertexArrayInstanced(0, mesh.vertexCount, instances);

    // Unbind all binded texture maps
    /*for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        // Select current shader texture slot
        rlActiveTextureSlot(i);

        // Disable texture for active slot
        if ((i == MATERIAL_MAP_IRRADIANCE) ||
            (i == MATERIAL_MAP_PREFILTER) ||
            (i == MATERIAL_MAP_CUBEMAP)) rlDisableTextureCubemap();
        else rlDisableTexture();
    }*/

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Remove instance transforms buffer
    rlUnloadVertexBuffer(instanceTransformsVboId);
    rlUnloadVertexBuffer(instanceColorsVboId);
    RL_FREE(instanceTransforms);
    RL_FREE(instanceColors);
}

std::string slurp(std::ifstream& in) {
    std::ostringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

Shader LoadShaderSingleFile(const std::string path) {
    Shader shader = { 0 };

    std::cout << "INFO: Loading shader '" + path + "'\n";

    std::ifstream shaderFile(path);
    std::string shaderStr(slurp(shaderFile));

    std::string vertexShaderStr = "#version 310 es\n#define VERTEX\n" + shaderStr;
    std::string fragmentShaderStr = "#version 310 es\n#define FRAGMENT\n" + shaderStr;

    const char *vShaderStr = (vertexShaderStr.c_str());
    const char *fShaderStr = (fragmentShaderStr.c_str());

    shader = LoadShaderFromMemory(vShaderStr, fShaderStr);

    return shader;
}
