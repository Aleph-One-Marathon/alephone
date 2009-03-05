uniform sampler2D texture0;
uniform float flare;

varying vec3 viewDir;
varying vec4 vertexColor;

void main (void) {

	vec3 viewv = normalize(viewDir);

	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	gl_FragColor = vec4(0.0, 0.0, 0.0, vertexColor.a * color.a);
}
