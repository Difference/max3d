
//@mode ambient,shadow

//@common

#if BB_MODE==BB_AMBIENT
varying mat3 normalMat;
varying vec2 texCoords0;
#endif

//@vertex

void main(){

	mat4 modelMat=bb_ModelMatrices[bb_InstanceID];
	vec4 vertex=modelMat * bb_Vertex;
	gl_Position=bb_ViewProjectionMatrix * vertex;

#if BB_MODE==BB_AMBIENT
	//tan space axiis
	vec3 s=bb_Tangent.xyz * bb_Tangent.w;
	vec3 t=cross( bb_Tangent.xyz,bb_Normal.xyz );
	vec3 n=bb_Normal.xyz;

	//tan space->view space
	normalMat=bb_mat3( bb_ViewMatrix * modelMat ) * mat3( s,t,n );
//	mat4 tmp=bb_ViewMatrix * modelMat;
//	normalMat=mat3( tmp[0].xyz,tmp[1].xyz,tmp[2].xyz ) * mat3( s,t,n );
	
	texCoords0=bb_TexCoords0.st;
#endif
}

//@fragment

#if BB_MODE==BB_AMBIENT
float TileMapSize=1024.0;
float TileMapScale=1.0/TileMapSize;

uniform sampler2D TileMap;		//1 x 1024 x 1024 tile map
uniform sampler2D DiffuseAtlas;	//4 x 256 x 256 texture atlas
#endif

void main(){
#if BB_MODE==BB_AMBIENT
	vec2 tc=floor( texCoords0.st*TileMapSize )/TileMapSize;
	float t0=texture2D( TileMap,tc ).a * 255.0;
	float t1=texture2D( TileMap,tc+vec2(TileMapScale,0.0) ).a * 255.0;
	float t2=texture2D( TileMap,tc+vec2(0.0,TileMapScale) ).a * 255.0;
	float t3=texture2D( TileMap,tc+vec2(TileMapScale,TileMapScale) ).a * 255.0;
	
	vec2 gca=fract( texCoords0.st*16.0 );
	gca.x/=4.0;
	gca*=255.0/256.0;
	gca+=0.5/1024.0;
	vec3 d0a=texture2D( DiffuseAtlas,gca+vec2(t0/4.0,0.0) ).rgb;
	vec3 d1a=texture2D( DiffuseAtlas,gca+vec2(t1/4.0,0.0) ).rgb;
	vec3 d2a=texture2D( DiffuseAtlas,gca+vec2(t2/4.0,0.0) ).rgb;
	vec3 d3a=texture2D( DiffuseAtlas,gca+vec2(t3/4.0,0.0) ).rgb;

	vec2 gcb=fract( texCoords0.st*128.0 );
	gcb.x/=4.0;
	gcb*=255.0/256.0;
	gcb+=0.5/1024.0;
	vec3 d0b=texture2D( DiffuseAtlas,gcb+vec2(t0/4.0,0.0) ).rgb;
	vec3 d1b=texture2D( DiffuseAtlas,gcb+vec2(t1/4.0,0.0) ).rgb;
	vec3 d2b=texture2D( DiffuseAtlas,gcb+vec2(t2/4.0,0.0) ).rgb;
	vec3 d3b=texture2D( DiffuseAtlas,gcb+vec2(t3/4.0,0.0) ).rgb;
	
	vec3 d0,d1,d2,d3;
	float depth=64.0;
	
	if( depth<0.0 ){
		d0=d0b;
		d1=d1b;
		d2=d2b;
		d3=d3b;
	}else if( depth>=64.0 ){
		d0=d0a;
		d1=d1a;
		d2=d2a;
		d3=d3a;
	}else{
		float t=(depth-0.0)/64.0;
		d0=mix(d0b,d0a,t );
		d1=mix(d1b,d1a,t );
		d2=mix(d2b,d2a,t );
		d3=mix(d3b,d3a,t );
	}
	
	vec2 fc=texCoords0.st*TileMapSize-floor( texCoords0*TileMapSize );
	vec3 diffuse=mix( mix( d0,d1,fc.x ),mix( d2,d3,fc.x ),fc.y );

	float specular=0.0;
	
	vec3 normal=normalMat * vec3( 0.0,0.0,1.0 );//( texture2D( NormalMap,texCoords0 ).xyz-0.5 );
	
	//ambient/emissive color...
	gl_FragData[0]=vec4( diffuse * bb_AmbientColor,1.0 );
		
	//diffuse/specular color...
	gl_FragData[1]=vec4( diffuse,specular );
	
	//normal
	gl_FragData[2]=vec4( normal+0.5,1.0 );
#endif
}
