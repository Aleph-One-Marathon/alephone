varying vec4 vertexColor;

void main(void) {
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vertexColor = gl_Color;
}
