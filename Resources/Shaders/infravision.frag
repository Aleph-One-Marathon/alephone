uniform sampler2D texture0;
uniform sampler2D texture1;

varying vec3 viewDir;
varying vec4 vertexColor;

void main (void) {

	// parallax

	float scale = 0.010;
	float bias = -0.005;
	vec3 viewv = normalize(viewDir);
	vec3 norm;

	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);

	// iterative parallax mapping
	for(int i = 0; i < 4; ++i) {
		vec4 normal = texture2D(texture1, texCoords.xy);
		norm = (normal.rgb - 0.5) * 2.0;
		float h = normal.a * scale + bias;
		texCoords += h * viewv;
	}

	vec4 normal = texture2D(texture1, texCoords.xy);
	norm = (normal.rgb - 0.5) * 2.0;

	float diffuse = dot(norm, viewv);
	vec4 color = texture2D(texture0, texCoords.xy);
	float c = max(max(color.r, color.g), color.b);
	gl_FragColor = vec4(0.0, 0.0, c * diffuse, 1.0);
}
