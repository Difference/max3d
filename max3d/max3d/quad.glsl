
//@mode postprocess

//@common

varying vec2 TexCoords;

//@vertex
attribute vec2 Attrib0;

void main(){
	TexCoords=Attrib0 * (bb_ViewportSize/bb_WindowSize);
	gl_Position=vec4( Attrib0*2.0-1.0,0.0,1.0 );
}

//@fragment
uniform sampler2D bb_QuadTexture;

void main(){
	gl_FragColor=texture2D( bb_QuadTexture,TexCoords );
}

