#version 330

// COLORS
uniform vec4 startColor;
uniform vec4 endColor;

// IN OUT
in vec2 fragTexCoord;
out vec4 finalColor;

varying vec3 worldPosition;

void main (void) {
    finalColor = mix(startColor, endColor, worldPosition.y/1.0);
}
