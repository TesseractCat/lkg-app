precision mediump float;

#ifdef VERTEX

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

in vec4 colDiffuse;
in mat4 matModel;

// Input uniform values
uniform mat4 matView;
uniform mat4 matProjection;

uniform vec2 atlasSize;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    float charIdx = floor(colDiffuse.a * 255.0);

    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord * vec2(1.0/atlasSize.x, 1.0/atlasSize.y);
    fragTexCoord += vec2(
        (1.0/atlasSize.x) * mod(charIdx, atlasSize.x),
        (1.0/atlasSize.y) * floor(charIdx/atlasSize.x));

    vec4 modelPos = matModel*vec4(vertexPosition, 1.0);
    vec3 lightDir = normalize(vec3(-3.0, 5.0, 8.0) - modelPos.xyz);
    float diff = (max(dot(vertexNormal, lightDir), 0.0) + 0.2);

    fragColor = vec4((diff * colDiffuse).xyz, 1.0);
    gl_Position = (matProjection*matView*matModel)*vec4(vertexPosition, 1.0);
    return;
}

#endif
#ifdef FRAGMENT

precision mediump float;

// IN OUT
uniform sampler2D texture1;

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

void main (void) {
    finalColor = texture(texture1, fragTexCoord);
    finalColor = vec4(finalColor.rgb * fragColor.rgb, finalColor.a);
    if (finalColor.a < 0.5)
        discard;
}

#endif
