
//@mode AMBIENT,POINTLIGHT,DISTANTLIGHT

//@common

varying vec2 texCoords0;

#if BB_MODE & BB_POINTLIGHT

//vertex/normal in light space
varying vec3 vertex,normal;

#elif BB_MODE & BB_DISTANTLIGHT

//normal in light space
varying vec3 normal;

#endif

//@vertex

void main(){

	gl_Position=bb_ModelViewProjectionMatrix * bb_Vertex;
	
	texCoords0=bb_TexCoords0.st;
	
#if BB_MODE & BB_POINTLIGHT

	//vertex position in light space
	vertex=(bb_ModelLightMatrix * bb_Vertex).xyz;

	//vertex normal in lightspace	
	normal=(bb_ModelLightMatrix * vec4( bb_Normal,0.0 ) ).xyz;
	
#elif BB_MODE & BB_DISTANTLIGHT

	//vertex normal in lightspace	
	normal=(bb_ModelLightMatrix * vec4( bb_Normal,0.0 ) ).xyz;
	
#endif
}

//@fragment

uniform vec3 DiffuseColor;
uniform sampler2D DiffuseTexture;

void main(){

	//material diffuse color
	vec3 diffuse=DiffuseColor * texture2D( DiffuseTexture,texCoords0 ).rgb;
	
#if BB_MODE & BB_AMBIENT

	gl_FragColor=vec4( diffuse * bb_AmbientColor,1.0 );

#elif BB_MODE & BB_POINTLIGHT

	//light attenutation
	float atten=max( 1.0-length( vertex )/bb_LightRange,0.0 );
	atten*=atten;
	
	//more attenuation
	atten*=max( -dot( normalize( vertex ),normalize( normal ) ),0.0 );
	
	gl_FragColor=vec4( diffuse * bb_LightColor * atten,1.0 );

#elif BB_MODE & BB_DISTANTLIGHT

	//attenuation
	float atten=max( -normalize( normal ).z,0.0 );
	
	gl_FragColor=vec4( diffuse * bb_LightColor * atten,1.0 );

#endif
	
}
