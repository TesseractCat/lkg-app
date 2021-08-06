#version 310 es
precision mediump float;

// COLORS
uniform vec4 startColor;
uniform vec4 endColor;

// IN OUT
in vec3 worldPosition;
in vec2 fragTexCoord;
out vec4 finalColor;

void main (void) {
    finalColor = mix(startColor, endColor, worldPosition.y/1.0);
}
