
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
	vec2 fragXY=vpos.xy/vpos.w;
	vec2 fragCoords=fragXY/2.0+0.5;
	fragCoords*=bb_ViewportSize/bb_WindowSize;
	
	float fragZ=texture2D( bb_DepthBuffer,fragCoords ).r;
	fragZ=bb_zNear * bb_zFar / (fragZ * (bb_zNear-bb_zFar) + bb_zFar);
	if( fragZ<bb_ShadowNearClip || fragZ>bb_ShadowFarClip ) discard;

	vec3 fragPos=vec3( fragXY * fragZ,fragZ );
	//

	vec4 shadowpos=bb_ViewShadowMatrix * vec4( fragPos,1.0 );
	if( shadowpos.w<=0.0 ) discard;
	
	vec3 t=shadowpos.xyz/shadowpos.w;
	if( abs(t.x)>=1.0 || abs(t.y)>=1.0 ) discard;

	t.xy*=bb_ShadowMapScale;
	t=t/2.0+0.5;
	
	float sh=0.0,oneTex=1.0/bb_ShadowMapSize,halfTex=oneTex/2.0;

/* 4 x 4
	for( float dx=-halfTex*3.0;dx<=halfTex*3.0;dx+=oneTex ){
		for( float dy=-halfTex*3.0;dy<=halfTex*3.0;dy+=oneTex ){
			if( texture2D( bb_ShadowBuffer,t.xy+vec2(dx,dy) ).r>t.z ) sh+=1.0;
		}
	}
	sh/=16.0;
*/

/* 3 x 3
	for( float dx=-halfTex*2.0;dx<=halfTex*2.0;dx+=oneTex ){
		for( float dy=-halfTex*2.0;dy<=halfTex*2.0;dy+=oneTex ){
			if( texture2D( bb_ShadowBuffer,t.xy+vec2(dx,dy) ).r>t.z ) sh+=1.0;
		}
	}
	sh/=9.0;
*/

///* 1 x 1	
	if( t.z-.0005 < texture2D( bb_ShadowBuffer,t.xy ).r ) sh=1.0;
//*/	
	gl_FragData[0]=vec4( sh );
}
