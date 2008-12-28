
//@mode shadow

//@common

varying vec4 vpos;

//@vertex

void main(){

	gl_Position=bb_ModelShadowMatrix * bb_Vertex;	
	vpos=gl_Position;
	
	gl_ClipVertex=bb_ModelMatrix * bb_Vertex;
}

//@fragment

void main(){
	gl_FragData[0]=vec4( vpos.z/vpos.w );
}

//@mode ambient

//@common

varying vec2 texCoords0;
varying vec3 normal;
varying float depth;

//@vertex

void main(){

	gl_Position=bb_ModelViewProjectionMatrix * bb_Vertex;
	
	texCoords0=bb_TexCoords0.st;
	
	normal=(bb_ModelViewMatrix * vec4( bb_Normal,0.0 ) ).xyz;
	
	depth=(bb_ModelViewMatrix * bb_Vertex).z;
}

//@fragment

uniform vec3 DiffuseColor;

uniform sampler2D DiffuseTexture;

void main(){

	//total diffuse
	vec3 diffuse=DiffuseColor * texture2D( DiffuseTexture,texCoords0 ).rgb;
		
	//ambient/emissive color...
	gl_FragData[0]=vec4( diffuse * bb_AmbientColor,1.0 );
		
	//diffuse/specular color...
	gl_FragData[1]=vec4( diffuse,0.0 );
		
	//normal
	gl_FragData[2]=vec4( normalize( normal )/2.0+0.5,1.0 );
	
	//depth
	gl_FragData[3]=vec4( depth );
}
