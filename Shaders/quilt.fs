#version 310 es
precision mediump float;

//See: https://github.com/patriciogonzalezvivo/glslViewer/blob/main/src/shaders/holoplay.h
// QUILT TEXTURE
//uniform sampler2D texture0;
uniform sampler2D texture1;

//uniform vec2        resolution;
vec2 resolution = vec2(1536, 2048);
uniform vec2 tile;
//uniform vec4 holoPlayCalibration;  // dpi, pitch, slope, center
//uniform vec2 holoPlayRB;           // ri, bi
uniform float pitch;
uniform float slope;
uniform float center;
uniform float dpi;

uniform float ri;
uniform float bi;

// IN OUT
in vec2 fragTexCoord;
out vec4 finalColor;

// GET CORRECT VIEW
vec2 quilt_map(vec3 t, vec2 pos, float a) {
    vec2 tile2 = vec2(t.x - 1.0, t.y - 1.0);
    vec2 dir = vec2(-1.0, -1.0);
    
    a = fract(a) * t.y;
    tile2.y += dir.y * floor(a);
    a = fract(a) * t.x;
    tile2.x += dir.x * floor(a);
    
    //Vertically flip quilt uvs, I don't know why this is necessary
    return ((tile2 + pos) / t.xy);// * vec2(1,-1) + vec2(0, 1);
}

void main (void) {
    vec4 holoPlayCalibration = vec4(dpi, pitch, slope, center);
    vec2 holoPlayRB = vec2(ri, bi);
    
    vec3 color = vec3(0.0);
    vec2 uv = gl_FragCoord.xy;
    vec2 st = uv/resolution.xy;
    //st.x = (1.0 - st.x);
    //st.y = (1.0 - st.y);
    
    float pitch = -resolution.x / holoPlayCalibration.x  * holoPlayCalibration.y * sin(atan(abs(holoPlayCalibration.z)));
    float tilt = resolution.y / (resolution.x * holoPlayCalibration.z);
    
    float subp = 1.0 / (3.0 * resolution.x);
    float subp2 = subp * pitch;
    
    float a = (-st.x - st.y * tilt) * pitch - holoPlayCalibration.w;
    color.r = texture(texture1, quilt_map(vec3(tile.xy, tile.x * tile.y), st, a-holoPlayRB.x*subp2)).r;
    color.g = texture(texture1, quilt_map(vec3(tile.xy, tile.x * tile.y), st, a-subp2)).g;
    color.b = texture(texture1, quilt_map(vec3(tile.xy, tile.x * tile.y), st, a-holoPlayRB.y*subp2)).b;
    
    #if defined(HOLOPLAY_DEBUG_CENTER)
    // Mark center line only in central view
    color.r = color.r * 0.001 + (st.x>0.49 && st.x<0.51 && fract(a)>0.48&&fract(a)<0.51 ?1.0:0.0);
    color.g = color.g * 0.001 + st.x;
    color.b = color.b * 0.001 + st.y;
    #elif defined(HOLOPLAY_DEBUG)
    // use quilt texture
    color = texture(texture1, st).rgb;
    #endif
    
    finalColor = vec4(color,1.0);
}
