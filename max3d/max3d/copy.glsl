
//@mode postprocess

//@vertex
void main(){
	gl_Position=bb_Vertex;
}

//@fragment
void main(){
	gl_FragColor=texture2DRect( bb_ColorBuffer,gl_FragCoord.xy );
}
