
//@mode postprocess

//@common
varying vec2 texCoords;

//@vertex
void main(){
	gl_Position=bb_Vertex;
	texCoords=bb_TexCoords0.st;
}

//@fragment
uniform float FogStart;
uniform float FogEnd;
uniform vec3 FogColor;

void main(){

	float fragZ=texture2DRect( bb_DepthBuffer,texCoords ).r;
	
	if( fragZ==1.0 ){
	
		gl_FragColor=texture2DRect( bb_ColorBuffer,texCoords );

	}else{

		fragZ=bb_zNear * bb_zFar / (fragZ * (bb_zNear-bb_zFar) + bb_zFar);
		
		float t=clamp( (fragZ-FogStart)/(FogEnd-FogStart),0.0,1.0 );
		
		vec3 color=mix( texture2DRect( bb_ColorBuffer,texCoords ).rgb,FogColor,t );
		
		gl_FragColor=vec4( color,1.0 );
	}
}
