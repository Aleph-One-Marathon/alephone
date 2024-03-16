R"(

precision highp float;
varying vec2 textureUV;
uniform mat4 MS_ModelViewMatrix;
uniform sampler2D texture0;
uniform vec4 lightPositions[32];
uniform vec4 lightColors[32];
uniform float fogMode;
uniform float fogStart;
uniform float fogEnd;
uniform vec4 uFogColor;
uniform vec4 clipPlane0;
uniform vec4 clipPlane1;
uniform vec4 clipPlane5;
uniform float pulsate;
uniform float wobble;
uniform float glow;
uniform float flare;
uniform float selfLuminosity;

varying vec4 fogColor;
varying vec3 viewXY;
varying vec3 viewDir;
varying vec4 vertexColor;
varying float FDxLOG2E;
varying float classicDepth;
varying vec4 vPosition_eyespace;
varying vec3 eyespaceNormal;

float getFogFactor(float distance) {
	if (fogMode == 0.0) {
		return clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
	} else if (fogMode == 1.0) {
		return clamp(exp(-fogColor.a * distance), 0.0, 1.0);
	} else if (fogMode == 2.0) {
		return clamp(exp(-fogColor.a * fogColor.a * distance * distance), 0.0, 1.0);
	} else {
		return 1.0;
	}
}

void main (void) {
	if( dot( vPosition_eyespace, clipPlane0) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, clipPlane1) < 0.0 ) {discard;}
	if( dot( vPosition_eyespace, clipPlane5) < 0.0 ) {discard;}

    vec3 texCoords = vec3(textureUV.xy, 0.0);
    vec3 normXY = normalize(viewXY);
    texCoords += vec3(normXY.y * -pulsate, normXY.x * pulsate, 0.0);
    texCoords += vec3(normXY.y * -wobble * texCoords.y, wobble * texCoords.y, 0.0);
    float mlFactor = clamp(selfLuminosity + flare - classicDepth, 0.0, 1.0);
    // more realistic: replace classicDepth with (length(viewDir)/8192.0)
    vec3 intensity;
    if (vertexColor.r > mlFactor) {
        intensity = vertexColor.rgb + (mlFactor * 0.5); }
    else {
        intensity = (vertexColor.rgb * 0.5) + mlFactor; }
    intensity = clamp(intensity, glow, 1.0);
#ifdef GAMMA_CORRECTED_BLENDING
    intensity = intensity * intensity; // approximation of pow(intensity, 2.2)
#endif
	vec4 color = texture2D(texture0, texCoords.xy);
	float fogFactor = getFogFactor(length(viewDir));

	//Calculate light
    vec4 lightAddition = vec4(0.0, 0.0, 0.0, 1.0);
    for(int i = 0; i < 32; ++i) {
		float size = lightPositions[i].w;
		if( size < .1) { break; } //End of light list
		vec3 lightPosition = vec3(lightPositions[i].xyz);
		vec4 lightColor = vec4(lightColors[i].rgb, 1.0);
		float mode = lightColors[i].a;
		float distance = length(lightPosition - vPosition_eyespace.xyz);
		vec3 lightVector = normalize(lightPosition - vPosition_eyespace.xyz);
		float diffuse = max(dot(eyespaceNormal, lightVector), 0.0);
		diffuse = diffuse * max((size*size - distance*distance)/(size*size), 0.0 ); //Attenuation

		// Spotlight attenuation
		if(mode >= 0.33 && mode <= 0.66) {
			//vec3 spotlightDirection = normalize(vec3(lightPositions[i+1].xyz)); //Input should be an eyespace vector.
			vec3 spotlightDirection = (MS_ModelViewMatrix * normalize(vec4(lightPositions[i+1].x, lightPositions[i+1].y, lightPositions[i+1].z, 0.0))).xyz; //Convert world spotlight vector to eyespace vector.

			float outerLimitCos = lightColors[i+1].x; //Cosine of outside angle (in radians). There is no light beyond this angle.
			float innerLimitCos = lightColors[i+1].y; //Cosine of inside angle (in radians). Light is at 100% inside of this angle.
			float dotFromDirection = dot(-lightVector, spotlightDirection);
			float inLight = smoothstep(outerLimitCos, innerLimitCos, dotFromDirection);
			
			if(dot(-lightVector, spotlightDirection) > outerLimitCos) {
				diffuse *= inLight; // apply the spotlight effect to the diffuse component
			} else {
				diffuse = 0.0; // light is outside of the spotlight cone
			}
			
			i++; //Remember, spot lights take up two slots.
		}

		lightAddition = lightAddition + color * diffuse * lightColor;
    }
	
    gl_FragColor = vec4(mix(fogColor.rgb, lightAddition.rgb + color.rgb * intensity, fogFactor), color.a);
}

)"
