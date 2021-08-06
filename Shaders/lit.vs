#version 310 es
precision mediump float;

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform vec4 colDiffuse;

// Input uniform values
uniform mat4 mvp;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;

uniform int shadow;
uniform vec3 lightPos;
uniform float planeZ;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;

    vec4 modelPos = matModel*vec4(vertexPosition, 1.0);
    vec3 lightDir = normalize(vec3(-3.0, 5.0, 8.0) - modelPos.xyz);
    float diff = (max(dot(vertexNormal, lightDir), 0.0) + 0.2);

    if (shadow == 0) {
        float gradient = (modelPos.y + 1.0)/2.0;
        gradient = mix(0.3, 1.0, gradient);
	fragColor = vec4((diff * colDiffuse * gradient).xyz, 1.0);
        gl_Position = (mvp)*vec4(vertexPosition, 1.0);
        return;
    }
    
    fragColor = vec4(vec3(0.8), 1.0);

    //See: https://math.stackexchange.com/questions/35857/two-point-line-form-in-3d 
    vec3 A = ((vec4(vertexPosition, 1.0))).xyz;
    vec3 B = (vec4(lightPos, 1.0)).xyz;

    float viewPlaneZ = planeZ;//(matView*vec4(0,0,planeZ,1.0)).z;

    //Solve for x when z = planeZ
    float x = (viewPlaneZ - A.z)/((B.z - A.z)/(B.x - A.x)) + A.x;
    float y = ((B.y - A.y)/(B.x - A.x)) * (x - A.x) + A.y;
    float z = viewPlaneZ;
    
    // Calculate final vertex position
    gl_Position = (mvp*vec4(x, y, z, 1.0));
}
