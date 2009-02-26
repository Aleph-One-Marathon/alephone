varying vec3 viewDir;

void main(void) {

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 v = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
	viewDir = (gl_Vertex - v).xyz;
}
