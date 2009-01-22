
/*

Godrays!

Effectively a radial blur...

Based on most excellent code by Fabien Sanglard: http://fabiensanglard.net/lightScattering/index.php

*/

//@mode postprocess

//@vertex
void main(){
	gl_Position=bb_Vertex;
}

//@fragment
uniform float Exposure;
uniform float LightPosX,LightPosY;

const int NUM_SAMPLES=100;

const float decay=1.0;
const float density=0.84;
const float weight=5.65;	

void main(){

	vec2 LightPos=vec2( LightPosX,LightPosY );

	vec2 texCoords=gl_FragCoord.xy;
	vec2 deltaTexCoords=texCoords-LightPos;
	deltaTexCoords*=1.0/float(NUM_SAMPLES)*density;
	float illumDecay=1.0;
	
	vec4 color=vec4( 0.0 );
	
	for( int i=0;i<NUM_SAMPLES;++i ){
		texCoords-=deltaTexCoords;
		
		vec4 sample=vec4( 0.0 );
		if( texture2DRect( bb_DepthBuffer,texCoords ).r==1.0 ){
			sample=vec4( 0.2,0.2,0.0,0.0 );
		}
		
		sample*=illumDecay*weight;
		color+=sample;
		illumDecay*=decay;
	}
	
	gl_FragColor=texture2DRect( bb_ColorBuffer,gl_FragCoord.xy ) + color*Exposure;
}
