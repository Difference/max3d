
//@mode postprocess

//@vertex
void main(){
	gl_Position=bb_Vertex;
}

//@fragment
uniform float FogStart;
uniform float FogEnd;
uniform vec3 FogColor;

void main(){

	float fragZ=texture2DRect( bb_DepthBuffer,gl_FragCoord.xy ).r;
	fragZ=bb_zNear * bb_zFar / (fragZ * (bb_zNear-bb_zFar) + bb_zFar);
	
	float t=clamp( (fragZ-FogStart)/(FogEnd-FogStart),0.0,1.0 );
	
	vec3 color=mix( texture2DRect( bb_ColorBuffer,gl_FragCoord.xy ).rgb,FogColor,t );
	
	gl_FragColor=vec4( color,1.0 );
}
