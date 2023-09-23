R"(

uniform sampler2D texture0;
uniform float glow;
uniform float bloomScale;
uniform float bloomShift;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
void main (void) {
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	vec3 intensity = clamp(vertexColor.rgb, glow, 1.0);
	//intensity = intensity * clamp(2.0 - length(viewDir)/8192.0, 0.0, 1.0);
	intensity = clamp(intensity * bloomScale + bloomShift, 0.0, 1.0);
#ifdef GAMMA_CORRECTED_BLENDING
	intensity = intensity * intensity;  // approximation of pow(intensity, 2.2)
	color.rgb = (color.rgb - 0.06) * 1.02;
#else
	color.rgb = (color.rgb - 0.2) * 1.25;
#endif
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), color.rgb * intensity, fogFactor), vertexColor.a * color.a);
}

)"
