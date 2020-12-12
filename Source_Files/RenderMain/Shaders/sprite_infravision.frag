R"(

uniform sampler2D texture0;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
void main (void) {
	
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	float avg = (color.r + color.g + color.b) / 3.0;
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, vertexColor.rgb * avg, fogFactor), vertexColor.a * color.a);
}

)"
