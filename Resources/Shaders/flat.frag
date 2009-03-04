uniform sampler2D texture0;
uniform float flare;

varying vec3 viewDir;
varying vec4 vertexColor;

varying float FDxLOG2E; 
varying float MLxLOG2E;

void main (void) {

	vec3 viewv = normalize(viewDir);

	float fogFactor = exp2(FDxLOG2E * dot(viewDir, viewDir)); 
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	float flash = exp2((flare - 1.0) * 2.0);
	float mlFactor = exp2(MLxLOG2E * dot(viewDir, viewDir) / flash + 1.0); 
	mlFactor = clamp(mlFactor, 0.0, flare * 0.75);

	vec4 color = texture2D(texture0, gl_TexCoord[0].xy);
	gl_FragColor = vec4(color.rgb * (vertexColor.rgb + mlFactor), color.a);
	gl_FragColor = vec4(mix(gl_Fog.color.rgb, gl_FragColor.rgb, fogFactor), gl_FragColor.a ); 
}
