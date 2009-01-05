
//@mode ambient

//@common

varying vec2 texCoords0;
varying vec4 normal;

//@vertex

void main(){
	mat4 modelMat=bb_ModelMatrices[bb_InstanceID];
	
	vec4 vertex=modelMat * bb_Vertex;
	
	texCoords0=bb_TexCoords0.st;
	
	normal=vec4( ( bb_ModelViewMatrix * vec4( bb_Normal.xyz,0.0 ) ).xyz + 0.5,1.0 );

	gl_Position=bb_ViewProjectionMatrix * vertex;
}

//@fragment

uniform sampler2D MirrorTexture;

void main(){

	//ambient/emissive
	gl_FragData[0]=texture2D( MirrorTexture,texCoords0 );
	
	//diffuse/specular
	gl_FragData[1]=vec4( 0.0 );
	
	//normal
	gl_FragData[2]=normal;
}
