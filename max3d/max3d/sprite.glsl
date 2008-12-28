
//@mode additive

//@common

varying vec2 texCoords0;

//@vertex

void main(){
	gl_Position=bb_ProjectionMatrix * bb_Vertex;	
	texCoords0=bb_TexCoords0.st;
}

//@fragment

uniform vec3 SpriteColor;
uniform sampler2D SpriteTexture;

void main(){
	vec3 texColor=texture2D( SpriteTexture,texCoords0 ).rgb;
	gl_FragData[0]=vec4( SpriteColor * texColor,1.0 );
}
