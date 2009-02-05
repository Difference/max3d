
//@mode postprocess

//@common
varying vec2 texCoords;

//@vertex
attribute vec2 Attrib0;

void main(){
	gl_Position=bb_Vertex;
	texCoords=bb_TexCoords0.st;
}

//@fragment
uniform vec2 BlurScale;

uniform float BlurFactors[15];

//float blurFactors[15]=float[15](
//0.0044,0.0115,0.0257,0.0488,0.0799,0.1133,0.1394,0.1494,0.1394,0.1133,0.0799,0.0488,0.0257,0.0115,0.0044
//);

void main(){
	
	vec4 color=vec4( 0.0 );

	for( int i=0;i<15;++i ){
		vec2 tc=min( texCoords+BlurScale*float(i-7),bb_ViewportSize );
		color+=texture2DRect( bb_ColorBuffer,tc ) * BlurFactors[i];
	}

	gl_FragColor=color;
}

