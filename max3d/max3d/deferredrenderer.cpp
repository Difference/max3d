
#include "std.h"

#include "app.h"
#include "deferredrenderer.h"

static CVertexBuffer *quadVB;
static CIndexBuffer *quadIB;

static CVertexBuffer *boxVB;
static CIndexBuffer *boxIB;

static CFrameBuffer *_frameBuffer;
static CFrameBuffer *_accumBuffer;
static CTexture *_accumTexture;
static CTexture *_colorTexture;
static CTexture *_normalTexture;
static CTexture *_depthTexture;
static CTexture *_depthBuffer;

static const int SHADOWMAP_SIZE=2048;

static CTexture *shadowMap;
static CTexture *shadowDepth;
static CFrameBuffer *shadowFrameBuf;
static CShader *shadowMapShader;
static CShader *pointLightShader;
static CShader *distantLightShader;

static CShader *quadShader;

CDeferredRenderer::CDeferredRenderer(){
	static bool _inited;

	if( _inited ) return;
	_inited=true;

	float vbdata[]={ 0,0,1,0,1,1,0,1 };
	int ibdata[]={0,1,2,0,2,3 };
	quadVB=App.Graphics()->CreateVertexBuffer( 4,"2f" );
	quadVB->SetData( vbdata );
	quadIB=App.Graphics()->CreateIndexBuffer( 6,"1i" );
	quadIB->SetData( ibdata );

	boxVB=App.Graphics()->CreateVertexBuffer( 8,"3f" );
	int ip[]={
		2,3,1,2,1,0,
		3,7,5,3,5,1,
		7,6,4,7,4,5,
		6,2,0,6,0,4,
		0,1,5,0,5,4,
		6,7,3,6,3,2 };
	boxIB=App.Graphics()->CreateIndexBuffer( 36,"1i" );
	boxIB->SetData( ip );

	_accumTexture=App.Graphics()->Create2dTexture( 1024,768,FORMAT_ARGB16F,TEXTURE_CLAMPST|TEXTURE_RENDER );
	_colorTexture=App.Graphics()->Create2dTexture( 1024,768,FORMAT_ARGB8,TEXTURE_CLAMPST|TEXTURE_RENDER );
	_normalTexture=App.Graphics()->Create2dTexture( 1024,768,FORMAT_ARGB8,TEXTURE_CLAMPST|TEXTURE_RENDER );
	_depthTexture=App.Graphics()->Create2dTexture( 1024,768,FORMAT_R16F,TEXTURE_CLAMPST|TEXTURE_RENDER );
	_depthBuffer=App.Graphics()->Create2dTexture( 1024,768,FORMAT_DEPTHBUF,TEXTURE_RENDER );

	CTexture *renderTargets1[]={_accumTexture,0,0,0};
	_accumBuffer=App.Graphics()->CreateFrameBuffer( renderTargets1,0 );

	CTexture *renderTargets2[]={_accumTexture,_colorTexture,_normalTexture,_depthTexture};
	_frameBuffer=App.Graphics()->CreateFrameBuffer( renderTargets2,_depthBuffer );

	App.Graphics()->SetTextureParam( "bb_FragAccumMap",_accumTexture );
	App.Graphics()->SetTextureParam( "bb_FragColorMap",_colorTexture );
	App.Graphics()->SetTextureParam( "bb_FragNormalMap",_normalTexture );
	App.Graphics()->SetTextureParam( "bb_FragDepthMap",_depthTexture );

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

	quadShader=App.ShaderUtil()->LoadShader( "quad" );
}

CFrameBuffer *CDeferredRenderer::FrameBuffer(){
	return _frameBuffer; 
}

CFrameBuffer *CDeferredRenderer::AccumBuffer(){
	return _accumBuffer;
}

CTexture *CDeferredRenderer::AccumTexture(){
	return _accumTexture; 
}

CTexture *CDeferredRenderer::ColorTexture(){ 
	return _colorTexture; 
}

CTexture *CDeferredRenderer::NormalTexture(){
	return _normalTexture; 
}

CTexture *CDeferredRenderer::DepthTexture(){ 
	return _depthTexture; 
}

void CDeferredRenderer::RenderQuad(){
	App.Graphics()->SetVertexBuffer( quadVB );
	App.Graphics()->SetIndexBuffer( quadIB );
	App.Graphics()->Render( 3,0,6 );
}

void CDeferredRenderer::RenderBox( const CBox & box ){
	CVec3 vp[8];
	for( int i=0;i<8;++i ) vp[i]=box.Corner(i);
	boxVB->SetData( vp );
	App.Graphics()->SetVertexBuffer( boxVB );
	App.Graphics()->SetIndexBuffer( boxIB );
	App.Graphics()->Render( 3,0,36 );
}

void CDeferredRenderer::RenderPointLight( CLight *light ){
//	CSphere bounds( light->RenderMatrix().Translation(),light->Range() );
//	if( !frustum.Intersects( bounds ) ) return;

	float range=light->Range();

	CBox lightBox=CBox( CVec3(-range),CVec3(range) );

	App.Graphics()->SetFloatParam( "bb_ShadowNearClip",-1024 );
	App.Graphics()->SetFloatParam( "bb_ShadowFarClip",1024 );
	App.Graphics()->SetFloatParam( "bb_LightRange",range );
	App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );

	if( !light->ShadowMask() ){
		App.Graphics()->SetShaderMode( CShader::ModeForName( "pointlight" ) );
		App.Graphics()->SetFrameBuffer( AccumBuffer() );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		App.Graphics()->SetShader( pointLightShader );
		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",light->RenderMatrix() );
		RenderBox( lightBox );
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
		App.Graphics()->SetViewport( int(SHADOWMAP_SIZE/2-sz/2),int(SHADOWMAP_SIZE/2-sz/2),int(sz),int(sz) );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		App.Graphics()->Clear();
		for( int j=0;j<rhull.planes.size();++j ){
			App.Graphics()->SetClipPlane( j,(float*)&rhull.planes[j].n );
		}
		RenderSurfaces( rhull );
		for( int j=0;j<rhull.planes.size();++j ){
			App.Graphics()->SetClipPlane( j,0 );
		}

		//render shadows
		App.Graphics()->SetShaderMode( CShader::ModeForName( "shadowmap" ) );
		App.Graphics()->SetFrameBuffer( AccumBuffer() );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		App.Graphics()->SetWriteMask( WRITEMASK_ALPHA );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		if( !i ) App.Graphics()->Clear();
		App.Graphics()->SetShader( shadowMapShader );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",lightMatrix );
		RenderBox( lightBox );
	}

	//render light
	App.Graphics()->SetShaderMode( CShader::ModeForName( "pointlight" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_DSTALPHA,BLENDFUNC_ONE );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
	App.Graphics()->SetCullMode( CULLMODE_FRONT );
	App.Graphics()->SetShader( pointLightShader );
	App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
	RenderBox( lightBox );
}

void CDeferredRenderer::RenderDistantLight( CLight *light ){
//	float segs[]={1.0f,16.0f};
//	float segs[]={0.0f,8.0,16.0,64.0,256.0f};
	float segs[]={0.0f,4.0f,12.0f,24.0f,72.0f,256.0f};
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
		App.Graphics()->SetViewport( int(SHADOWMAP_SIZE/2-sz/2),int(SHADOWMAP_SIZE/2-sz/2),int(sz),int(sz) );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );//CULLMODE_FRONT );
		App.Graphics()->Clear();

		App.Graphics()->SetFloatParam( "bb_ShadowNearClip",nnear );
		App.Graphics()->SetFloatParam( "bb_ShadowFarClip",ffar );
		RenderSurfaces( CHull() );

		//render shadows
		App.Graphics()->SetShaderMode( CShader::ModeForName( "shadowmap" ) );
		App.Graphics()->SetFrameBuffer( AccumBuffer() );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		App.Graphics()->SetWriteMask( WRITEMASK_ALPHA );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
		if( !i ) App.Graphics()->Clear();
		App.Graphics()->SetShader( shadowMapShader );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
		RenderBox( lightBox );
	}

	//render light
	App.Graphics()->SetShaderMode( CShader::ModeForName( "distantlight" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_DSTALPHA,BLENDFUNC_ONE );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
	App.Graphics()->SetCullMode( CULLMODE_FRONT );
	App.Graphics()->SetShader( distantLightShader );
	App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
	RenderBox( lightBox );
}

void CDeferredRenderer::RenderCamera(){

	//Ambient pass...
	App.Graphics()->SetShaderMode( CShader::ModeForName( "ambient" ) );
	App.Graphics()->SetFrameBuffer( FrameBuffer() );
	App.Graphics()->SetViewport( 0,0,1024,768 );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE|WRITEMASK_DEPTH );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
	App.Graphics()->SetCullMode( CULLMODE_BACK );
	App.Graphics()->Clear();
	RenderSurfaces( _frustum );

	//Lighting passes...
	for( vector<CLight*>::const_iterator it=_lights.begin();it!=_lights.end();++it ){
		CLight *light=*it;
		switch( light->Type() ){
		case 1:RenderPointLight( light );break;
		case 2:RenderDistantLight( light );break;
		default:Error( "TODO" );
		}
	}

	//Additive pass...
	App.Graphics()->SetShaderMode( CShader::ModeForName( "additive" ) );
	App.Graphics()->SetFrameBuffer( AccumBuffer() );
	App.Graphics()->SetViewport( 0,0,1024,768 );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
	App.Graphics()->SetCullMode( CULLMODE_BACK );
	RenderSurfaces( CHull() );

	App.Graphics()->SetShaderMode( CShader::ModeForName( "postprocess" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
	App.Graphics()->SetCullMode( CULLMODE_NONE );
	App.Graphics()->SetShader( quadShader );
	App.Graphics()->SetTextureParam( "bb_QuadTexture",AccumTexture() );
	App.Graphics()->SetFrameBuffer( 0 );
	RenderQuad();
}
