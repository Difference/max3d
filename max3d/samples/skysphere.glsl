
//@mode postprocess

//@vertex

void main(){
	gl_Position=bb_Vertex;
}

//@fragment
const float PI=3.1415926535;

uniform float FogStart;
uniform float FogEnd;

uniform sampler2D SkyTexture;

void main(){
	vec2 fragCoords=gl_FragCoord.xy;

	float fragZ=texture2DRect( bb_DepthBuffer,fragCoords ).r;
	
	if( fragZ!=1.0 ){
		gl_FragColor=texture2DRect( bb_ColorBuffer,fragCoords );
		return;
	}
	
	fragZ=bb_zNear * bb_zFar / (fragZ * (bb_zNear-bb_zFar) + bb_zFar);
	
	vec3 fragPos=vec3( (fragCoords * bb_FragScale + bb_FragOffset) * fragZ,fragZ );
	
	fragPos=(bb_CameraMatrix * vec4( fragPos,1.0 ) ).xyz;
	
	float s=atan( fragPos.x,fragPos.z )/PI/2.0+0.5;
	
//	float t=0.5-normalize( fragPos ).y/2.0;
	float t=0.5-atan( fragPos.y / length( vec2( fragPos.x,fragPos.z ) ) )/PI;

//	gl_FragColor=vec4( s,t,0.0,1.0 );
	
	gl_FragColor=texture2DLod( SkyTexture,vec2( s,t*1.0 ),0.0 );
}

