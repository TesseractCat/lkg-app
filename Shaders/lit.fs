#version 310 es
precision mediump float;

// IN OUT
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

void main (void) {
    finalColor = fragColor;
}
