
//@mode postprocess

//@common
varying vec2 texCoords;

//@vertex
void main(){
	gl_Position=bb_Vertex;
	texCoords=bb_TexCoords0.st;
}

//@fragment
uniform float BlurStrength;

uniform sampler2DRect bb_BlurBuffer;

void main(){

	vec4 fragColor=texture2DRect( bb_ColorBuffer,texCoords );
	
	vec4 blurColor=texture2DRect( bb_BlurBuffer,texCoords/4.0 );
	
	gl_FragColor=mix( fragColor,blurColor,BlurStrength );
}

