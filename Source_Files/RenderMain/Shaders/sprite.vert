R"(

uniform mat4 MS_ModelViewProjectionMatrix;
uniform mat4 MS_ModelViewMatrix;
uniform mat4 MS_ModelViewMatrixInverse;
uniform mat4 MS_TextureMatrix;
uniform vec4 uColor;
uniform vec4 uFogColor;
uniform float depth;
uniform float strictDepthMode;
attribute vec4 vPosition;
attribute vec2 vTexCoord;
attribute vec4 vColor;
varying vec2 textureUV;
varying vec4 fogColor;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
varying vec4 vPosition_eyespace;
void main(void) {
    vPosition_eyespace = MS_ModelViewMatrix * vPosition;
    gl_Position = MS_ModelViewProjectionMatrix * vPosition;
    classicDepth = gl_Position.z / 8192.0;
    vec4 v = MS_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
    viewDir = (vPosition - v).xyz;
    vec4 UV4 = vec4(vTexCoord.x, vTexCoord.y, 0.0, 1.0);           //DCW shitty attempt to stuff texUV into a vec4
    textureUV = (MS_TextureMatrix * UV4).xy;
    vertexColor = vColor; //Might need to switch between the u and v versions of this during debugging.
    FDxLOG2E = -uFogColor.a * 1.442695;
    fogColor = uFogColor;
}

)"
