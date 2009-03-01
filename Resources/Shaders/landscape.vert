varying vec3 viewDir;
varying float texScale;
varying float texOffset;

void main(void) {

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 v = gl_ModelViewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0);
	viewDir = (gl_Vertex - v).xyz;
	texScale = gl_TextureMatrix[0][1][1];
	texOffset = gl_TextureMatrix[0][3][1];
}
