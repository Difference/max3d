
//@mode postprocess

//@vertex

void main(){
	gl_Position=bb_Vertex;
}

//@fragment
const float PI=3.1415926535;

uniform float SkyStart;
uniform float SkyEnd;
uniform sampler2D SkyTexture;

void main(){
	vec2 fragCoords=gl_FragCoord.xy;

	
	vec3 fragPos=vec3( fragCoords * bb_FragScale + bb_FragOffset,1.0 );
	
	fragPos=(bb_CameraMatrix * vec4( fragPos,0.0 ) ).xyz;

	float s=atan( fragPos.x,fragPos.z )/PI/2.0+0.5;

//	float t=0.5-normalize( fragPos ).y/2.0;
	float t=0.5-atan( fragPos.y / length( vec2( fragPos.x,fragPos.z ) ) )/PI;

//	vec4 skyColor=vec4( s,t,0.0,1.0 );
	vec4 skyColor=texture2DLod( SkyTexture,vec2( s,t*1.0 ),0.0 );
	
	float fragZ=texture2DRect( bb_DepthBuffer,fragCoords ).r;
	fragZ=bb_zNear * bb_zFar / (fragZ * (bb_zNear-bb_zFar) + bb_zFar);

	float mx=clamp( (fragZ-SkyStart)/(SkyEnd-SkyStart),0.0,1.0 );
		
	gl_FragColor=mix( texture2DRect( bb_ColorBuffer,fragCoords ),skyColor,mx );
}

