
//@mode ambient,shadow

//@common

varying vec2 texCoords0;

#if BB_MODE==BB_AMBIENT
varying mat3 normalMat;
#endif

//@vertex

void main(){
	mat4 modelMat=bb_ModelMatrices[bb_InstanceID];
	vec4 vertex=modelMat * bb_Vertex;
	texCoords0=bb_TexCoords0.st;
	
#if BB_MODE==BB_SHADOW
	gl_Position=bb_ShadowMatrix * vertex;
#endif

#if BB_MODE==BB_AMBIENT
	gl_Position=bb_ViewProjectionMatrix * vertex;

	//tan space axiis
	vec3 s=bb_Tangent.xyz * bb_Tangent.w;
	vec3 t=cross( bb_Tangent.xyz,bb_Normal.xyz );
	vec3 n=bb_Normal.xyz;

	//tan space->view space
	normalMat=bb_mat3( bb_ViewMatrix * modelMat ) * mat3( s,t,n );
//	mat4 tmp=bb_ViewMatrix * modelMat;
//	normalMat=mat3( tmp[0].xyz,tmp[1].xyz,tmp[2].xyz ) * mat3( s,t,n );

#endif
}

//@fragment

uniform sampler2D DiffuseMap;

#if BB_MODE==BB_AMBIENT
uniform vec3 DiffuseColor;
uniform sampler2D SpecularMap;
uniform sampler2D NormalMap;
#endif

void main(){
	vec4 texel=texture2D( DiffuseMap,texCoords0 );
	if( texel.a<0.5 ){
		discard;
	}

#if BB_MODE==BB_AMBIENT
	vec3 diffuse=DiffuseColor * texel.rgb;

	float specular=texture2D( SpecularMap,texCoords0 ).r;
	
	vec3 normal=normalMat * ( texture2D( NormalMap,texCoords0 ).xyz-0.5 );
	
	//ambient/emissive color...
	gl_FragData[0]=vec4( diffuse * bb_AmbientColor,1.0 );
		
	//diffuse/specular color...
	gl_FragData[1]=vec4( diffuse,specular );
	
	//normal
	gl_FragData[2]=vec4( normal+0.5,1.0 );
#endif
}
