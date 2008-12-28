
//@mode postprocess

//@common

varying vec2 TexCoords;

//@vertex
attribute vec2 Attrib0;

void main(){
	TexCoords=Attrib0;
	gl_Position=vec4( Attrib0*2.0-1.0,0.0,1.0 );
}

//@fragment
uniform vec2 bb_BlurScale;

uniform sampler2D bb_QuadTexture;

void main(){
	vec4 t=vec4( 0.0 );
	for( float i=-7.0;i<=7.0;i+=1.0 ){
		t+=texture2D( bb_QuadTexture,TexCoords+bb_BlurScale*i );
	}
	gl_FragColor=t/15.0;
}

