
//@mode postprocess

//@common
varying vec2 texCoords;

//@vertex
attribute vec2 Attrib0;

void main(){
	gl_Position=bb_Vertex;
	texCoords=bb_TexCoords0;
}

//@fragment
uniform vec2 BlurScale;

void main(){
	vec4 t=vec4( 0.0 );
	for( float i=-7.0;i<=7.0;i+=1.0 ){
		t+=texture2D( bb_ColorBuffer,texCoords+BlurScale*i );
	}
	gl_FragColor=t/15.0;
}

