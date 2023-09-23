R"(

uniform sampler2D texture0;
uniform float visibility;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
void main(void) {
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	vec3 intensity = vec3(0.0, 0.0, 0.0);
	float fogFactor = clamp(exp2(FDxLOG2E * length(viewDir)), 0.0, 1.0);
	gl_FragColor = vec4(mix(vec3(0.0, 0.0, 0.0), intensity, fogFactor), vertexColor.a * color.a * visibility);
}

)"
