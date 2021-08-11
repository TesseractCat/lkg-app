precision mediump float;

#ifdef VERTEX

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

//in vec4 colDiffuse;
in vec4 direction; // Line screen direction (xy), line width (z), line color packed (w)
in mat4 matModel;

// Input uniform values
uniform mat4 matView;
uniform mat4 matProjection;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

vec3 unpackColor(float value) {
	vec3 color;
	color.r = mod(value, 129.0) / 128.0;
	color.b = mod(floor(value / 129.0), 129.0) / 128.0;
	color.g = floor(value / (129.0 * 129.0)) / 128.0;
	return color;
}

void main()
{
    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;

    vec4 modelPos = matModel*vec4(vertexPosition, 1.0);

    float aspect = 1536.0/2048.0;

    vec4 projectedPos = (matProjection*matView*matModel) * vec4(vertexPosition, 1.0);

    vec2 screenDir = direction.xy;
    vec2 screenNormal = vec2(-screenDir.y, screenDir.x);
    screenNormal *= direction.z;
    screenNormal.x /= aspect;
    screenNormal *= (vertexTexCoord.x < 0.5 ? 1.0 : -1.0);

    vec4 offset = vec4(screenNormal.xy, 0.0, 0.0);

    //gl_Position = (matProjection*matView*matModel)*vec4(vertexPosition, 1.0);
    gl_Position = projectedPos + offset;
    fragColor = vec4(unpackColor(direction.w), 1.0);
}

#endif
#ifdef FRAGMENT

precision mediump float;

// IN OUT
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

void main (void) {
    finalColor = fragColor;
    //finalColor = mix(fragColor, mix(fragColor, vec4(1,1,1,1), 0.65), pow(sin(fragTexCoord.x * 3.14159), 9.0));
}

#endif
