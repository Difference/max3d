
//@mode distantlight

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

	vec3 fragPos=vec3( fragXY * fragZ,fragZ );
	//
	
	//light vector
	vec3 lvec=bb_ViewSpaceLightVector;
	
	//half vector
	vec3 hvec=normalize( normalize( -fragPos ) + lvec );
	
	//normal vector
	vec3 normal=normalize( texture2D( bb_NormalBuffer,fragCoords ).rgb-0.5 );
	
	//diffuse atten
	float diffi=max( dot( lvec,normal ),0.0 );
	
	//specular atten
	float speci=pow( max( dot( hvec,normal ),0.0 ),128.0 );
	
	//diffuse color
	vec3 diffuse=texture2D( bb_MaterialBuffer,fragCoords ).rgb * diffi;
	
	//specular color
	vec3 specular=vec3( texture2D( bb_MaterialBuffer,fragCoords ).a ) * speci;

	//total lighting color
	gl_FragColor=vec4( (diffuse+specular) * bb_LightColor,1.0 );
}
