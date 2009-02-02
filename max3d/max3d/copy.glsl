
//@mode postprocess

//@common
varying vec2 texCoords;

//@vertex
void main(){
	gl_Position=bb_Vertex;
	texCoords=bb_TexCoords0.st;
}

//@fragment
void main(){
	gl_FragColor=texture2DRect( bb_ColorBuffer,texCoords );
}
