#version 100

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;

uniform vec3 lightPos;
uniform float planeZ;

// NOTE: Add here your custom variables 

void main()
{
    //See: https://math.stackexchange.com/questions/35857/two-point-line-form-in-3d 
    vec3 A = (vec4(vertexPosition, 1.0)).xyz;
    vec3 B = (vec4(lightPos, 1.0)).xyz;
    
    //Solve for x when z = planeZ
    float x = (planeZ - A.z)/((B.z - A.z)/(B.x - A.x)) + A.x;
    float y = ((B.y - A.y)/(B.x - A.x)) * (x - A.x) + A.y;
    float z = planeZ;
    
    // Calculate final vertex position
    gl_Position = (mvp)*vec4(x, y, z, 1.0);
}
