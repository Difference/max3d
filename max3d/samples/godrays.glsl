
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
uniform vec3 GodRaysColor;
uniform float GodRaysExposure;
uniform float GodRaysLightX,GodRaysLightY;

const int NUM_SAMPLES=100;

void main(){

	vec2 texCoords=gl_FragCoord.xy;
	vec2 lightPos=vec2( GodRaysLightX,GodRaysLightY );
	vec2 deltaTexCoords=(texCoords-lightPos)/float( NUM_SAMPLES );
	
	vec3 color=vec3( 0.0 );

	for( int i=0;i<NUM_SAMPLES;++i ){
		texCoords-=deltaTexCoords;
		color+=GodRaysColor * floor( texture2DRect( bb_DepthBuffer,texCoords ).r );
	}

	gl_FragColor=texture2DRect( bb_ColorBuffer,gl_FragCoord.xy ) + vec4( color/float( NUM_SAMPLES ) * GodRaysExposure,0.0 );
}

