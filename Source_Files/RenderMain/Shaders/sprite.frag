R"(

uniform sampler2D texture0;
uniform float glow;
uniform float flare;
uniform float selfLuminosity;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
void main (void) {
	float mlFactor = clamp(selfLuminosity + flare - classicDepth, 0.0, 1.0);
	// more realistic: replace classicDepth with (length(viewDir)/8192.0)
	vec3 intensity;
	if (vertexColor.r > mlFactor) {
		intensity = vertexColor.rgb + (mlFactor * 0.5); }
	else {
		intensity = (vertexColor.rgb * 0.5) + mlFactor; }
	intensity = clamp(intensity, glow, 1.0);
#ifdef GAMMA_CORRECTED_BLENDING
	intensity = intensity * intensity; // approximation of pow(intensity, 2.2)
#endif
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, color.rgb * intensity, fogFactor), vertexColor.a * color.a);
}

)"
