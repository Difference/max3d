
//@mode shadowmap

//@common

varying vec4 vpos;

//@vertex

void main(){
	gl_Position=bb_ModelViewProjectionMatrix * bb_Vertex;
	vpos=gl_Position;
}

//@fragment

void main(){

	//generate fragPos and fragCoords...
	//
	//fragPos is view space fragment pos, and fragCoords is fragbuffer texcoords
	//
	vec2 fragCoords=gl_FragCoord.xy;
	
	float fragZ=texture2DRect( bb_DepthBuffer,fragCoords ).r;
	fragZ=bb_zNear * bb_zFar / (fragZ * (bb_zNear-bb_zFar) + bb_zFar);
	if( fragZ<bb_ShadowNearClip || fragZ>bb_ShadowFarClip ) discard;
	
	vec3 fragPos=vec3( (fragCoords * bb_FragScale + bb_FragOffset) * fragZ,fragZ );
	//

	vec4 shadowpos=bb_ViewShadowMatrix * vec4( fragPos,1.0 );
	if( shadowpos.w<=0.0 ) discard;

	vec3 t=shadowpos.xyz/shadowpos.w;
	if( abs(t.x)>=1.0 || abs(t.y)>=1.0 ) discard;

	t.xy*=bb_ShadowMapScale;
	t=t/2.0+0.5;
	
	//Simple shadow...
	float sh=0.0;
	if( t.z-.0005 < texture2D( bb_ShadowBuffer,t.xy ).r ) sh=1.0;
	gl_FragData[0]=vec4( sh );
}
