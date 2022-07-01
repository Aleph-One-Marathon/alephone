R"(
precision highp float;

uniform sampler2D texture0;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;

varying highp vec4 fogColor;
varying highp vec2 textureUV;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
varying vec4 vPosition_eyespace;

void main (void) {
    bool unwantedFragment = false;
    if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {unwantedFragment = true;}
    if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {unwantedFragment = true;}
    if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {unwantedFragment = true;}
	vec4 color = texture2D(texture0, textureUV.xy);
	float avg = (color.r + color.g + color.b) / 3.0;
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(fogColor.rgb, vertexColor.rgb * avg, fogFactor), vertexColor.a * color.a);
    if( unwantedFragment ) {gl_FragColor.a = 0.0;}
}

)"
