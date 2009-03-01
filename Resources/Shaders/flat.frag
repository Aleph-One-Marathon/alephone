uniform sampler2D texture0;

varying vec3 viewDir;
varying vec4 vertexColor;

varying float FDxLOG2E; 
varying float MLxLOG2E;

void main (void) {

	vec3 viewv = normalize(viewDir);

	float fogFactor = exp2(FDxLOG2E * dot(viewDir, viewDir)); 
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) + 1.0); 
	mlFactor = clamp(mlFactor, 0.0, 0.75);

	float d = clamp(1.0 - dot(viewDir, viewDir) * 0.00000005, -0.5, 1.0) * 0.2;
	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	gl_FragColor = color * (vertexColor + vec4(d,d,d,0.0));
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, gl_FragColor.rgb, fogFactor), gl_FragColor.a ); 
}
