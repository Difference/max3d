
//@mode clear

//@vertex

void main(){
	gl_Position=bb_Vertex;
}

//@fragment
uniform vec4 bb_ClearColor;

void main(){

	//ambient/emissive color...
	gl_FragData[0]=bb_ClearColor;
		
	//diffuse/specular color...
	gl_FragData[1]=vec4( 0.0 );
		
	//normal
	gl_FragData[2]=vec4( 0.5,0.5,0.5,1.0 );
}

