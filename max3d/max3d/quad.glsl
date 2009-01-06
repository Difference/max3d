
//@mode postprocess

//@common

varying vec2 TexCoords;

//@vertex
attribute vec2 Attrib0;

void main(){
	TexCoords=Attrib0 * bb_ViewportSize;
	gl_Position=vec4( Attrib0*2.0-1.0,0.0,1.0 );
}

//@fragment

void main(){
	gl_FragColor=texture2DRect( bb_QuadTexture,TexCoords );
}

