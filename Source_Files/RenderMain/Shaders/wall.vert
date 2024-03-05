R"(

uniform mat4 MS_ModelViewProjectionMatrix;
uniform mat4 MS_ModelViewMatrix;
uniform mat4 MS_ModelViewMatrixInverse;
uniform mat4 MS_TextureMatrix;

uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;
uniform vec4 uColor;
uniform float bloomScale;
uniform float bloomShift;
uniform float flare;
uniform float selfLuminosity;
uniform float pulsate;
uniform float wobble;
uniform float glow;
uniform float depth;
uniform vec4 uFogColor;

attribute vec2 vTexCoord;
attribute vec3 vNormal;
attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec4 vTexCoord4;

varying vec2 textureUV2;
varying vec2 textureUV;
varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
varying mat3 tbnMatrix;
varying vec4 vPosition_eyespace;
varying vec3 eyespaceNormal;
varying vec4 fogColor;

highp mat4 transpose(in highp mat4 inMatrix) {  //I have not tested this.
    highp vec4 i0 = inMatrix[0];
	highp vec4 i1 = inMatrix[1];
    highp vec4 i2 = inMatrix[2];
    highp vec4 i3 = inMatrix[3];

    highp mat4 outMatrix = mat4(
                 vec4(i0.x, i1.x, i2.x, i3.x),
                 vec4(i0.y, i1.y, i2.y, i3.y),
                 vec4(i0.z, i1.z, i2.z, i3.z),
                 vec4(i0.w, i1.w, i2.w, i3.w)
                 );

    return outMatrix;
}

void main(void) {
    vPosition_eyespace = MS_ModelViewMatrix * vPosition;
    gl_Position  = MS_ModelViewProjectionMatrix * vPosition;
    gl_Position.z = gl_Position.z + depth*gl_Position.z/65536.0;
    classicDepth = gl_Position.z / 8192.0;
	
#ifndef DISABLE_CLIP_VERTEX
//        "    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
#endif
	
    vec4 UV4 = vec4(vTexCoord.x, vTexCoord.y, 0.0, 1.0);           //DCW shitty attempt to stuff texUV into a vec4
    mat3 normalMatrix = mat3(transpose(MS_ModelViewMatrixInverse));           //DCW shitty replacement for gl_NormalMatrix
    textureUV = (MS_TextureMatrix * UV4).xy;
    /* SETUP TBN MATRIX in normal matrix coords, gl_MultiTexCoord1 = tangent vector */
    vec3 n = normalize(normalMatrix * vNormal);
    vec3 t = normalize(normalMatrix * vTexCoord4.xyz);
    vec3 b = normalize(cross(n, t) * vTexCoord4.w);
    /* (column wise) */
    tbnMatrix = mat3(t.x, b.x, n.x, t.y, b.y, n.y, t.z, b.z, n.z);
    
    /* SETUP VIEW DIRECTION in unprojected local coords */
    viewDir = tbnMatrix * (MS_ModelViewMatrix * vPosition).xyz;
    viewXY = -(MS_TextureMatrix * vec4(viewDir.xyz, 1.0)).xyz;
    viewDir = -viewDir;
    vertexColor = vColor;
    FDxLOG2E = -uFogColor.a * 1.442695;
    fogColor = uFogColor;
 
    eyespaceNormal = vec3(MS_ModelViewMatrix * vec4(vNormal, 0.0));
}

)"
