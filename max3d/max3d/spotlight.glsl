
//@mode spotlight

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
	
	//fragment vals
	vec3 diffuse=texture2D( bb_MaterialBuffer,fragCoords ).rgb;
	vec3 specular=vec3( texture2D( bb_MaterialBuffer,fragCoords ).a );
	vec3 normal=texture2D( bb_NormalBuffer,fragCoords ).rgb-0.5;
	normal=-normalize( bb_mat3(bb_ViewLightMatrix) * normal );
	
	//frag/eye pos in light space
	vec3 fpos=(bb_ViewLightMatrix * vec4( fragPos,1.0 )).xyz;
	vec3 epos=bb_ViewLightMatrix[3].xyz;
	
	vec3 lvec=normalize( fpos );
	vec3 hvec=normalize( normalize( epos ) + lvec );
	diffuse*=max( dot( lvec,normal ),0.0 );
	specular*=pow( max( dot( hvec,normal ),0.0 ),128.0 );

	float rangeAtten=max( 1.0-length( fpos )/bb_LightRange,0.0 );

	float cosAngle=lvec.z;
	float angleAtten=max( (cosAngle-.7071)/(1.0-.7071),0.0 );
	
	vec3 light=texture2D( bb_LightTexture,fpos.xy/fpos.z ).rgb * bb_LightColor * rangeAtten * angleAtten;

	gl_FragColor=vec4( (diffuse+specular) * light ,1.0 );
}
