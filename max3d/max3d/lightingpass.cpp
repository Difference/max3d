
#include "std.h"

#include "app.h"
#include "lightingpass.h"

static const int SHADOWMAP_SIZE=2048;

static CTexture *shadowMap;
static CTexture *shadowDepth;
static CFrameBuffer *shadowFrameBuf;
static CShader *shadowMapShader;
static CShader *pointLightShader;
static CShader *distantLightShader;

void CLightingPass::Render( CRenderer *r ){
	for( vector<CLight*>::const_iterator it=r->Lights().begin();it!=r->Lights().end();++it ){
		CLight *light=*it;
		switch( light->Type() ){
		case 1:RenderPointLight( light,r );break;
		case 2:RenderDistantLight( light,r );break;
		default:Error( "TODO" );
		}
	}
}

void CLightingPass::RenderPointLight( CLight *light,CRenderer *r ){
//	CSphere bounds( light->RenderMatrix().Translation(),light->Range() );
//	if( !frustum.Intersects( bounds ) ) return;

	float range=light->Range();

	CBox lightBox=CBox( CVec3(-range),CVec3(range) );

	App.Graphics()->SetFloatParam( "bb_NearClip",-1024 );
	App.Graphics()->SetFloatParam( "bb_FarClip",1024 );
	App.Graphics()->SetFloatParam( "bb_LightRange",range );
	App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );

	if( !light->ShadowMask() ){
		App.Graphics()->SetShaderMode( CShader::ModeForName( "pointlight" ) );
		App.Graphics()->SetFrameBuffer( r->AccumBuffer() );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		App.Graphics()->SetShader( pointLightShader );
		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",light->RenderMatrix() );
		r->RenderBox( lightBox );
		return;
	}

	CHull rhull;
	rhull.planes.push_back( CPlane( CVec3( +1,0,0 ),range ) );
	rhull.planes.push_back( CPlane( CVec3( -1,0,0 ),range ) );
	rhull.planes.push_back( CPlane( CVec3( 0,+1,0 ),range ) );
	rhull.planes.push_back( CPlane( CVec3( 0,-1,0 ),range ) );
	rhull.planes.push_back( CPlane( CVec3( 0,0,+1 ),range ) );
	rhull.planes.push_back( CPlane( CVec3( 0,0,-1 ),range ) );
	rhull=light->RenderMatrix() * rhull;

	//calc shadowmap size
	CMat4 camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	float d=camMat.Translation().Distance( light->RenderMatrix().Translation() );
	float sz=light->Range()/d * SHADOWMAP_SIZE;
	if( sz>SHADOWMAP_SIZE ) sz=SHADOWMAP_SIZE;
	App.Graphics()->SetFloatParam( "bb_ShadowMapScale",sz/SHADOWMAP_SIZE );
	
	CMat4 shadowProjectionMatrix=FrustumMatrix( -1,1,-1,1,1,0 );

	for( int i=0;i<6;++i ){
		float y=0,p=0;
		if( i==0 ){
			p=HALFPI;		//down
		}else if( i==5 ){
			p=-HALFPI;		//up
		}else{
			y=(i-1)*HALFPI;	//around
		}
		CMat4 lightMatrix=light->RenderMatrix() * RotationMatrix( CVec3( y,p,0 ) );

		App.Graphics()->SetMat4Param( "bb_LightMatrix",lightMatrix );
		App.Graphics()->SetMat4Param( "bb_ShadowProjectionMatrix",shadowProjectionMatrix );

		//render to shadowmap
		App.Graphics()->SetShaderMode( CShader::ModeForName( "shadow" ) );
		App.Graphics()->SetFrameBuffer( shadowFrameBuf );
		App.Graphics()->SetViewport( SHADOWMAP_SIZE/2-sz/2,SHADOWMAP_SIZE/2-sz/2,sz,sz );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		App.Graphics()->Clear();
		for( int j=0;j<rhull.planes.size();++j ){
			App.Graphics()->SetClipPlane( j,(float*)&rhull.planes[j].n );
		}
		r->RenderSurfaces( rhull );
		for( int j=0;j<rhull.planes.size();++j ){
			App.Graphics()->SetClipPlane( j,0 );
		}

		//render shadows
		App.Graphics()->SetShaderMode( CShader::ModeForName( "shadowmap" ) );
		App.Graphics()->SetFrameBuffer( r->AccumBuffer() );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		App.Graphics()->SetWriteMask( WRITEMASK_ALPHA );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		if( !i ) App.Graphics()->Clear();
		App.Graphics()->SetShader( shadowMapShader );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",lightMatrix );
		r->RenderBox( lightBox );
	}

	//render light
	App.Graphics()->SetShaderMode( CShader::ModeForName( "pointlight" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_DSTALPHA,BLENDFUNC_ONE );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
	App.Graphics()->SetCullMode( CULLMODE_FRONT );
	App.Graphics()->SetShader( pointLightShader );
	App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
	r->RenderBox( lightBox );
}

void CLightingPass::RenderDistantLight( CLight *light,CRenderer *r ){
//	float segs[]={1.0f,16.0f};
	float segs[]={0.0f,8.0,16.0,64.0,256.0f};
//	float segs[]={1.0f,4.0f,16.0f,64.0f,256.0f};
//	float segs[]={1.0f,2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,128.0f,256.0f };

	float sz=SHADOWMAP_SIZE;
	App.Graphics()->SetFloatParam( "bb_ShadowMapScale",sz/SHADOWMAP_SIZE );

	CBox lightBox=CBox( CVec3(-2),CVec3(2) );

	CMat4 camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );

	CMat4 cameraLightMatrix=light->InverseRenderMatrix() * camMat;

	App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );

	for( int i=0;i<sizeof(segs)/sizeof(float)-1;++i ){

		float nnear=segs[i];
		float ffar=segs[i+1];

		CVec3 verts[]={
		CVec3(-nnear,+nnear,nnear),CVec3(+nnear,+nnear,nnear),CVec3(+nnear,-nnear,nnear),CVec3(-nnear,-nnear,nnear),
		CVec3(-ffar,+ffar,ffar),CVec3(+ffar,+ffar,ffar),CVec3(+ffar,-ffar,ffar),CVec3(-ffar,-ffar,ffar) };

		float left=100000,right=-100000,bottom=100000,top=-100000,tnear=100000,tfar=-100000;
		for( int j=0;j<8;++j ){
			CVec3 v=cameraLightMatrix * verts[j];
			if( v.x<left ) left=v.x;if( v.x>right ) right=v.x;
			if( v.y<bottom ) bottom=v.y;if( v.y>top ) top=v.y;
			if( v.z<tnear ) tnear=v.z;if( v.z>tfar ) tfar=v.z;
		}

		CMat4 shadowProjectionMatrix=OrthoMatrix( left,right,bottom,top,-512,512 );

		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetMat4Param( "bb_ShadowProjectionMatrix",shadowProjectionMatrix );

		//render to shadowmap
		App.Graphics()->SetShaderMode( CShader::ModeForName( "shadow" ) );
		App.Graphics()->SetFrameBuffer( shadowFrameBuf );
		App.Graphics()->SetViewport( SHADOWMAP_SIZE/2-sz/2,SHADOWMAP_SIZE/2-sz/2,sz,sz );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		App.Graphics()->Clear();

		App.Graphics()->SetFloatParam( "bb_NearClip",nnear );
		App.Graphics()->SetFloatParam( "bb_FarClip",ffar );
//		CPlane nplane=camera->RenderMatrix() * CPlane( CVec3(0,0,1),-nnear );
//		CPlane fplane=camera->RenderMatrix() * CPlane( CVec3(0,0,-1),ffar );
//		r->SetClipPlane( 0,(float*)&nplane.n );
//		r->SetClipPlane( 1,(float*)&fplane.n );
		r->RenderSurfaces( CHull() );
//		App.Graphics()->SetClipPlane( 0,0 );//camera->RenderMatrix() * nplane );
//		App.Graphics()->SetClipPlane( 1,0 );//camera->RenderMatrix() * fplane );

		//render shadows
		App.Graphics()->SetShaderMode( CShader::ModeForName( "shadowmap" ) );
		App.Graphics()->SetFrameBuffer( r->AccumBuffer() );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		App.Graphics()->SetWriteMask( WRITEMASK_ALPHA );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		if( !i ) App.Graphics()->Clear();
		App.Graphics()->SetShader( shadowMapShader );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
		r->RenderBox( lightBox );
	}

	//render light
	App.Graphics()->SetShaderMode( CShader::ModeForName( "distantlight" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_DSTALPHA,BLENDFUNC_ONE );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
	App.Graphics()->SetCullMode( CULLMODE_FRONT );
	App.Graphics()->SetShader( distantLightShader );
	App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
	r->RenderBox( lightBox );
}

void lightingpass_init(){
	string h=string( 
		"//@common\n"
		"#define bb_ShadowMapSize " )+SHADOWMAP_SIZE+"\n";

	App.Graphics()->AppendShaderHeader( h );

	shadowMapShader=App.ShaderUtil()->LoadShader( "shadowmap" );
	pointLightShader=App.ShaderUtil()->LoadShader( "pointlight" );
	distantLightShader=App.ShaderUtil()->LoadShader( "distantlight" );

	shadowMap=App.Graphics()->Create2dTexture( SHADOWMAP_SIZE,SHADOWMAP_SIZE,FORMAT_R16F,TEXTURE_CLAMPST|TEXTURE_RENDER );
	shadowDepth=App.Graphics()->Create2dTexture( SHADOWMAP_SIZE,SHADOWMAP_SIZE,FORMAT_DEPTHBUF,TEXTURE_CLAMPST|TEXTURE_RENDER );

	CTexture *renderTargets[]={shadowMap,0,0,0};
	shadowFrameBuf=App.Graphics()->CreateFrameBuffer( renderTargets,shadowDepth );

	App.Graphics()->SetTextureParam( "bb_ShadowMap",shadowMap );
}

