
//@mode additive

//@common

varying vec4 vpos;
varying vec2 texCoords;

//@vertex

void main(){

	vpos=bb_Vertex;
	
	texCoords=bb_TexCoords0.st;

	gl_Position=bb_ProjectionMatrix * bb_Vertex;	
}

//@fragment

#define SPHERICAL

uniform vec3 SpriteColor;
uniform sampler2D SpriteTexture;

void main(){
	vec3 texColor=texture2D( SpriteTexture,texCoords ).rgb;
	
	gl_FragData[0]=vec4( SpriteColor * texColor,1.0 );
	
#ifdef SPHERICAL

	vec2 d=texCoords * 2.0 - 1.0;
	
	float r=sqrt( 1.0 - dot( d,d ) );
	
	vec4 vp=vpos;
	
	vp.z-=r;
	
	vp=bb_ProjectionMatrix * vp;

	gl_FragDepth=vp.z / vp.w / 2.0 + 0.5;

#endif

}
