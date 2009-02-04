
//@mode postprocess

//@common
varying vec2 texCoords;

//@vertex
void main(){
	gl_Position=bb_Vertex;
}

//@fragment
uniform float BlurStrength;

uniform sampler2DRect bb_BlurBuffer;

void main(){

	vec4 fragColor=texture2DRect( bb_ColorBuffer,gl_FragCoord.xy );
	
	vec4 blurColor=texture2DRect( bb_BlurBuffer,gl_FragCoord.xy/4.0 );
	
	gl_FragColor=mix( fragColor,blurColor,BlurStrength );
}

