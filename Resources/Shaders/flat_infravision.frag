uniform sampler2D texture0;

varying vec3 viewDir;
varying vec4 vertexColor;

void main (void) {

	vec3 viewv = normalize(viewDir);

	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	float c = max(color.r, max(color.g,color.b));
	gl_FragColor = vec4(c, 0.0,0.0, color.a ); 
}
