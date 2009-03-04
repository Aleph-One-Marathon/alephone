uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float wobble;
uniform float flare;

varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;

varying float FDxLOG2E;
varying float MLxLOG2E;

void main (void) {

	// parallax

	float scale = 0.010;
	float bias = -0.005;
	vec3 viewv = normalize(viewDir);
	vec3 viewxy = normalize(viewXY);
	vec3 norm;

	vec3 texCoords = vec3(gl_TexCoord[0].xy, 0.0);

	texCoords += vec3(viewxy.yx * wobble, 0.0);

	// iterative parallax mapping
	for(int i = 0; i < 4; ++i) {
		vec4 normal = texture2D(texture1, texCoords.xy);
		norm = (normal.rgb - 0.5) * 2.0;
		float h = normal.a * scale + bias;
		texCoords += h * viewv;
	}

	float fogFactor = exp2(FDxLOG2E * dot(viewDir, viewDir)); 
	fogFactor = clamp(fogFactor, 0.0, 1.0); 

	float flash = exp2((flare - 1.0) * 2.0);
	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) / flash + 1.0); 
	mlFactor = clamp(mlFactor, 0.0, flare * 0.75);

	vec4 normal = texture2D(texture1, texCoords.xy);
	norm = (normal.rgb - 0.5) * 2.0;

	float diffuse = dot(norm, viewv);
	vec4 color = texture2D(texture0, texCoords.xy);

	gl_FragColor = vec4(color.rgb * (vertexColor.rgb + mlFactor) * diffuse, color.a);
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, gl_FragColor.rgb, fogFactor), color.a );
}
