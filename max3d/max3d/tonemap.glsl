
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
uniform sampler2D bb_QuadTexture;

void main(){
	vec4 t=texture2D( bb_QuadTexture,TexCoords );	//blurred
	vec4 c=texture2D( bb_FragAccumMap,TexCoords );	//original
	
//	gl_FragColor=c/(c+1.0);
	
//	gl_FragColor=c/(c+1.0)+max(t-1.0,0.0);
//	gl_FragColor=t;
//	gl_FragColor=max(t-1.0,0.0);

//	COOL!	
//	float lwhite=1.0;
//	gl_FragColor=c*(c/lwhite+1.0)/(c+1.0);

//	NOP!
	gl_FragColor=c;

//	glow/bloom
//	gl_FragColor=max( t-1.0,0.0 );//c + max(t-1.0,0.0);
}

