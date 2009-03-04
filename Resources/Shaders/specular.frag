uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float wobble;

varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;

void main (void) {

	// parallax

	float scale = 0.010;
	float bias = -0.005;
	vec3 viewv = normalize(viewDir);
	vec3 viewxy = normalize(viewXY);
	vec3 norm;

	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);

	// wobble effect, no idea why x and y need swapping
	texCoords += vec3(viewxy.yx * wobble, 0.0);

	// iterative parallax mapping
	for(int i = 0; i < 4; ++i) {
		vec4 normal = texture2D(texture1, texCoords.xy);
		norm = (normal.rgb - 0.5) * 2.0;
		float h = normal.a * scale + bias;
		texCoords += h * viewv;
	}

	float diffuse = dot(norm, viewv);
	diffuse = log2(clamp(diffuse, 0.0, 1.0)) / 8.0 + 1.0;
	vec4 color = texture2D(texture0, texCoords.xy);
	gl_FragColor = vec4(color.rgb * vertexColor.rgb * diffuse, color.a);
}
