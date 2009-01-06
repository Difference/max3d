
//@mode pointlight

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
	
	vec3 fragPos=vec3( (fragCoords * bb_FragScale + bb_FragOffset) * fragZ,fragZ );
	//

	//light vector
	vec3 lvec=normalize( bb_ViewSpaceLightPosition-fragPos );

	//half vector
	vec3 hvec=normalize( normalize( -fragPos ) + lvec );
	
	//normal vector
	vec3 normal=normalize( texture2DRect( bb_NormalBuffer,fragCoords ).rgb-0.5 );
	
	//diffuse atten
	float diffi=max( dot( lvec,normal ),0.0 );
	
	//specular atten
	float speci=pow( max( dot( hvec,normal ),0.0 ),128.0 );
	
	//distance attenutation
	float atten=max( 1.0-distance( fragPos,bb_ViewSpaceLightPosition )/bb_LightRange,0.0 );
	atten*=atten;
	
	//diffuse color
	vec3 diffuse=texture2DRect( bb_MaterialBuffer,fragCoords ).rgb * diffi;
	
	//specular color
	vec3 specular=vec3( texture2DRect( bb_MaterialBuffer,fragCoords ).a ) * speci;

	//total lighting color
	gl_FragColor=vec4( (diffuse+specular) * bb_LightColor * atten,1.0 );
}
