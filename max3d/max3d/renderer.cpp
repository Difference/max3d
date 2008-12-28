/*
Max3D
Copyright (c) 2008, Mark Sibly
All rights reserved.

Redistribution and use in source and binary forms, with or without
conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

* Neither the name of Max3D's copyright owner nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "std.h"

#include "app.h"
#include "renderer.h"

static CHull _nullHull;

static bool shadows;
static bool hdr;					

static CVertexBuffer *quadVB;
static CIndexBuffer *quadIB;
static CShader *quadShader;

static CVertexBuffer *boxVB;
static CIndexBuffer *boxIB;

static CTexture *accumBuffer;
static CTexture *normalBuffer;
static CTexture *materialBuffer;
static CTexture *depthBuffer;

static CShader *clearShader;

static const int SHADOWMAP_SIZE=2048;
static CTexture *shadowBuffer;
static CShader *shadowMapShader;

CRenderer::CRenderer(){
	static bool _inited;

	if( _inited ) return;
	_inited=true;
	
	int w=App.Graphics()->WindowWidth();
	int h=App.Graphics()->WindowHeight();
	
	App.Graphics()->SetVec2Param( "bb_WindowSize",CVec2( w,h ) );
	App.Graphics()->SetVec2Param( "bb_WindowScale",CVec2( 1.0f/w,1.0f/h ) );
	
	shadows=(App.Config( "SHADOWS" )=="TRUE");
	hdr=(App.Config( "HDR" )=="TRUE");

	float vbdata[]={ 0,0,1,0,1,1,0,1 };
	int ibdata[]={0,1,2,0,2,3 };
	quadVB=App.Graphics()->CreateVertexBuffer( 4,"2f" );
	quadVB->SetData( vbdata );
	quadIB=App.Graphics()->CreateIndexBuffer( 6,"1i" );
	quadIB->SetData( ibdata );
	quadShader=App.ShaderUtil()->LoadShader( "quad" );

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
	
	if( hdr ){
		accumBuffer=App.Graphics()->CreateTexture( w,h,FORMAT_RGBA16F,TEXTURE_CLAMPST|TEXTURE_RENDER );
	}else{
		accumBuffer=App.Graphics()->CreateTexture( w,h,FORMAT_RGBA8,TEXTURE_CLAMPST|TEXTURE_RENDER );
	}
	materialBuffer=App.Graphics()->CreateTexture( w,h,FORMAT_RGBA8,TEXTURE_CLAMPST|TEXTURE_RENDER );
	normalBuffer=App.Graphics()->CreateTexture( w,h,FORMAT_RGBA8,TEXTURE_CLAMPST|TEXTURE_RENDER );
	depthBuffer=App.Graphics()->CreateTexture( w,h,FORMAT_DEPTH,TEXTURE_RENDER );

	App.Graphics()->SetTextureParam( "bb_AccumBuffer",accumBuffer );
	App.Graphics()->SetTextureParam( "bb_MaterialBuffer",materialBuffer );
	App.Graphics()->SetTextureParam( "bb_NormalBuffer",normalBuffer );
	App.Graphics()->SetTextureParam( "bb_DepthBuffer",depthBuffer );
		
	clearShader=App.ShaderUtil()->LoadShader( "clear" );

	if( shadows ){
		string h=string( 
			"//@common\n"
			"#define bb_ShadowMapSize " )+SHADOWMAP_SIZE+".0\n";
		App.Graphics()->AppendShaderHeader( h );

		shadowBuffer=App.Graphics()->CreateTexture( SHADOWMAP_SIZE,SHADOWMAP_SIZE,FORMAT_DEPTH,TEXTURE_CLAMPST|TEXTURE_RENDER );
		App.Graphics()->SetTextureParam( "bb_ShadowBuffer",shadowBuffer );

		shadowMapShader=App.ShaderUtil()->LoadShader( "shadowmap" );
	}
}

void CRenderer::SetShaderMode( string mode ){
	int shaderMode=CShader::ModeForName( mode );
	App.Graphics()->SetShaderMode( shaderMode );
	_modeMask=1<<shaderMode;
	if( mode=="ambient" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( depthBuffer );
		App.Graphics()->SetViewport( _viewport.x,_viewport.y,_viewport.width,_viewport.height );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE|WRITEMASK_ALPHA|WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );
	}else if( mode=="clear" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( depthBuffer );
		App.Graphics()->SetViewport( _viewport.x,_viewport.y,_viewport.width,_viewport.height );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE|WRITEMASK_ALPHA|WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
	}else if( mode=="shadow" ){
		App.Graphics()->SetColorBuffer( 0,0 );
		App.Graphics()->SetDepthBuffer( shadowBuffer );
		App.Graphics()->SetViewport( SHADOWMAP_SIZE/2-_shadowMapSize/2,SHADOWMAP_SIZE/2-_shadowMapSize/2,_shadowMapSize,_shadowMapSize );
		App.Graphics()->SetWriteMask( WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );
	}else if( mode=="shadowmap" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( 0 );
		App.Graphics()->SetViewport( _viewport.x,_viewport.y,_viewport.width,_viewport.height );
		App.Graphics()->SetWriteMask( WRITEMASK_ALPHA );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
	}else if( mode=="spotlight" || mode=="pointlight" || mode=="distantlight" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( 0 );
		App.Graphics()->SetViewport( _viewport.x,_viewport.y,_viewport.width,_viewport.height );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( _shadowMapSize ? BLENDFUNC_DSTALPHA : BLENDFUNC_ONE,BLENDFUNC_ONE );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
	}else if( mode=="additive" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( depthBuffer );
		App.Graphics()->SetViewport( _viewport.x,_viewport.y,_viewport.width,_viewport.height );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );
	}else if( mode=="postprocess" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( 0 );
		App.Graphics()->SetViewport( _viewport.x,_viewport.y,_viewport.width,_viewport.height );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_NONE );
	}
}

void CRenderer::RenderQuad(){
	App.Graphics()->SetVertexBuffer( quadVB );
	App.Graphics()->SetIndexBuffer( quadIB );
	App.Graphics()->Render( 3,0,6,1 );
}

void CRenderer::RenderBox( const CBox & box ){
	CVec3 vp[8];
	for( int i=0;i<8;++i ) vp[i]=box.Corner(i);
	boxVB->SetData( vp );
	App.Graphics()->SetVertexBuffer( boxVB );
	App.Graphics()->SetIndexBuffer( boxIB );
	App.Graphics()->Render( 3,0,36,1 );
}

void CRenderer::RenderSurfaces( const CHull &bounds ){

	CShader *_shader=0;
	CMaterial *_material=0;
	
	for( vector<CSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		CSurface *surface=*it;
		
		CShader *shader=surface->Shader();
		if( shader!=_shader ){
			if( !(shader->ModeMask() & _modeMask )) continue;
			_shader=shader;
			App.Graphics()->SetShader( shader );
		}
		
		CMaterial *material=surface->Material();
		if( material!=_material ){
			_material=material;
			material->Bind();
		}
		
		surface->OnRenderInstances( bounds );
	}
}

void CRenderer::RenderSpotLight( CLight *light,CCamera *camera ){
	float angle=light->Angle();
	float range=light->Range();	
	float extent=tan(angle/2)*range;
	
	//TODO: should be a cone
	CBox lightBox=CBox( CVec3(-range),CVec3(range) );

	App.Graphics()->SetFloatParam( "bb_LightAngle",angle );
	App.Graphics()->SetFloatParam( "bb_LightRange",range );
	App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );
	App.Graphics()->SetFloatParam( "bb_ShadowNearClip",-1024 );
	App.Graphics()->SetFloatParam( "bb_ShadowFarClip",1024 );
	
	CTexture *tex=light->Texture();
	if( !tex ) tex=App.TextureUtil()->WhiteTexture();
	App.Graphics()->SetTextureParam( "bb_LightTexture",tex );
	
	if( !shadows || !light->ShadowMask() ){
		SetShaderMode( "spotlight" );
		App.Graphics()->SetShader( light->Shader() );
		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",light->RenderMatrix() );
		RenderBox( lightBox );
		return;
	}
	
	//calc shadowmap size
	CMat4 camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	float d=camMat.Translation().Distance( light->RenderMatrix().Translation() );
	float sz=light->Range()/d * SHADOWMAP_SIZE;
	sz=(int(sz)&~1);
	if( sz>SHADOWMAP_SIZE ) sz=SHADOWMAP_SIZE;
	_shadowMapSize=int(sz);
	
	App.Graphics()->SetFloatParam( "bb_ShadowMapScale",sz/SHADOWMAP_SIZE );
	
	CMat4 shadowProjectionMatrix=CMat4::FrustumMatrix( -1,1,-1,1,1,0 );
	CMat4 lightMatrix=light->RenderMatrix();

	App.Graphics()->SetMat4Param( "bb_LightMatrix",lightMatrix );
	App.Graphics()->SetMat4Param( "bb_ShadowProjectionMatrix",shadowProjectionMatrix );

	//render to shadowmap
	SetShaderMode( "shadow" );
	App.Graphics()->Clear();
	RenderSurfaces( _nullHull );

	//render shadows
	SetShaderMode( "shadowmap" );
	App.Graphics()->SetShader( shadowMapShader );
	App.Graphics()->SetMat4Param( "bb_ModelMatrix",lightMatrix );
	RenderBox( lightBox );

	//render light
	SetShaderMode( "spotlight" );
	App.Graphics()->SetShader( light->Shader() );
	App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
	App.Graphics()->SetMat4Param( "bb_ModelMatrix",light->RenderMatrix() );
	RenderBox( lightBox );
}

void CRenderer::RenderPointLight( CLight *light,CCamera *camera ){
	
	float range=light->Range();
	
	if( !camera->RenderFrustum().Intersects( CSphere( light->RenderMatrix().Translation(),range ) ) ) return;
	
//	CSphere bounds( light->RenderMatrix().Translation(),light->Range() );
//	if( !frustum.Intersects( bounds ) ) return;

	//TODO: Should be a sphere
	CBox lightBox=CBox( CVec3(-range),CVec3(range) );
	
	App.Graphics()->SetFloatParam( "bb_LightRange",range );
	App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );
	App.Graphics()->SetFloatParam( "bb_ShadowNearClip",-1024 );
	App.Graphics()->SetFloatParam( "bb_ShadowFarClip",1024 );

	if( !shadows || !light->ShadowMask() ){
		SetShaderMode( "pointlight" );
		App.Graphics()->SetShader( light->Shader() );
		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",light->RenderMatrix() );
		RenderBox( lightBox );
		return;
	}

	CHull rhull=light->RenderMatrix() * lightBox;

	//calc shadowmap size
	CMat4 camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	float d=camMat.Translation().Distance( light->RenderMatrix().Translation() );
	float sz=light->Range()/d * SHADOWMAP_SIZE;
	sz=(int(sz)&~1);
	if( sz>SHADOWMAP_SIZE ) sz=SHADOWMAP_SIZE;
	_shadowMapSize=int(sz);

	App.Graphics()->SetFloatParam( "bb_ShadowMapScale",sz/SHADOWMAP_SIZE );
	
	CMat4 shadowProjectionMatrix=CMat4::FrustumMatrix( -1,1,-1,1,1,range );

	for( int i=0;i<6;++i ){
		/*
		float y=0,p=0;
		if( i==0 ){
			p=HALFPI;		//down
		}else if( i==5 ){
			p=-HALFPI;		//up
		}else{
			y=(i-1)*HALFPI;	//around
		}
		CMat4 lightMatrix=light->RenderMatrix() * RotationMatrix( CVec3( y,p,0 ) );
		*/
		static const CMat4 rotMats[]={
			CMat4(CVec4(1,0,0,0),CVec4(0,0,1,0),CVec4(0,-1,0,0),CVec4(0,0,0,1)),
			CMat4(CVec4(1,0,0,0),CVec4(0,1,0,0),CVec4(0,0,1,0),CVec4(0,0,0,1)),
			CMat4(CVec4(0,0,1,0),CVec4(0,1,0,0),CVec4(-1,0,0,0),CVec4(0,0,0,1)),
			CMat4(CVec4(-1,0,0,0),CVec4(0,1,0,0),CVec4(0,0,-1,0),CVec4(0,0,0,1)),
			CMat4(CVec4(0,0,-1,0),CVec4(0,1,0,0),CVec4(1,0,0,0),CVec4(0,0,0,1)),
			CMat4(CVec4(1,0,0,0),CVec4(0,0,-1,0),CVec4(0,1,0,0),CVec4(0,0,0,1)) };

		CMat4 lightMatrix=light->RenderMatrix() * rotMats[i];

		App.Graphics()->SetMat4Param( "bb_LightMatrix",lightMatrix );
		App.Graphics()->SetMat4Param( "bb_ShadowProjectionMatrix",shadowProjectionMatrix );

		//render to shadowmap
		SetShaderMode( "shadow" );
		App.Graphics()->Clear();
		RenderSurfaces( rhull );

		//render shadows
		SetShaderMode( "shadowmap" );
		if( !i ) App.Graphics()->Clear();
		App.Graphics()->SetShader( shadowMapShader );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",lightMatrix );
		RenderBox( lightBox );
	}

	//render light
	SetShaderMode( "pointlight" );
	App.Graphics()->SetShader( light->Shader() );
	App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
	RenderBox( lightBox );
}

void CRenderer::RenderDistantLight( CLight *light,CCamera *camera ){
	CBox lightBox=CBox( CVec3(-2),CVec3(2) );

	CMat4 camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );

	App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );
	App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );

	if( !shadows || !light->ShadowMask() ){
		SetShaderMode( "distantlight" );
		App.Graphics()->SetShader( light->Shader() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
		RenderBox( lightBox );
		return;
	}
	
	//2 passes
//	float segs[]={1.0f,16.0f,256.0f};

	//3 passes
//	float segs[]={0.1f,4.0f,16.0f,256.0f};
	
	//4 passes
	float segs[]={1.0f,4.0f,16.0f,64.0f,256.0f};
	
	//8 passes
//	float segs[]={1.0f,2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,128.0f,256.0f };

	float sz=SHADOWMAP_SIZE;
	_shadowMapSize=int(sz);
	
	App.Graphics()->SetFloatParam( "bb_ShadowMapScale",sz/SHADOWMAP_SIZE );

	CMat4 cameraLightMatrix=light->InverseRenderMatrix() * camMat;

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

		CMat4 shadowProjectionMatrix=CMat4::OrthoMatrix( left,right,bottom,top,-512,512 );

		App.Graphics()->SetMat4Param( "bb_ShadowProjectionMatrix",shadowProjectionMatrix );

		//render models to shadowmap
		SetShaderMode( "shadow" );
		App.Graphics()->Clear();
		RenderSurfaces( _nullHull );

		//render shadows
		SetShaderMode( "shadowmap" );
		App.Graphics()->SetShader( shadowMapShader );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
		App.Graphics()->SetFloatParam( "bb_ShadowNearClip",nnear );
		App.Graphics()->SetFloatParam( "bb_ShadowFarClip",ffar );
		RenderBox( lightBox );
	}

	//render light
	SetShaderMode( "distantlight" );
	App.Graphics()->SetShader( light->Shader() );
	App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
	RenderBox( lightBox );
}

void CRenderer::Render( CCamera *camera ){

	int spotMask=1<<CShader::ModeForName( "spotlight" );
	int pointMask=1<<CShader::ModeForName( "pointlight" );
	int distantMask=1<<CShader::ModeForName( "distantlight" );

	_viewport=camera->Viewport();
	_viewport.x=_viewport.y=0;
	
	App.Graphics()->SetVec2Param( "bb_ViewportSize",CVec2( _viewport.width,_viewport.height ) );
	App.Graphics()->SetVec2Param( "bb_ViewportScale",CVec2( 1.0f/_viewport.width,1.0f/_viewport.height ) );
	
	App.Graphics()->SetMat4Param( "bb_CameraMatrix",camera->RenderMatrix() );
	App.Graphics()->SetMat4Param( "bb_ProjectionMatrix",camera->ProjectionMatrix() );
	
	App.Graphics()->SetFloatParam( "bb_zNear",camera->NearZ() );
	App.Graphics()->SetFloatParam( "bb_zFar",camera->FarZ() );

	for( vector<CSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		CSurface *surface=*it;
		surface->OnBeginCameraPass( camera );
	}
	
	App.Graphics()->SetColorBuffer( 1,materialBuffer );
	App.Graphics()->SetColorBuffer( 2,normalBuffer );
	App.Graphics()->SetColorBuffer( 3,0 );
	
	//ambient pass
	SetShaderMode( "ambient" );
	App.Graphics()->Clear();
	RenderSurfaces( camera->RenderFrustum() );
	
	//'skybox' for now...
	SetShaderMode( "clear" );
	App.Graphics()->SetShader( clearShader );
	App.Graphics()->SetMat4Param( "bb_ModelMatrix",CMat4() );
	RenderBox( CBox( CVec3(-128),CVec3(128) ) );

	App.Graphics()->SetColorBuffer( 1,0 );
	App.Graphics()->SetColorBuffer( 2,0 );

	//lighting
	for( vector<CLight*>::const_iterator it=_lights.begin();it!=_lights.end();++it ){
		_shadowMapSize=0;
		CLight *light=*it;
		CShader *shader=light->Shader();
		if( shader->ModeMask() & spotMask ){
			RenderSpotLight( light,camera );
		}else if( shader->ModeMask() & pointMask ){
			RenderPointLight( light,camera );
		}else if( shader->ModeMask() & distantMask ){
			RenderDistantLight( light,camera );
		}else{
			Error( "Unrecognized light type" );
		}
	}

	//additive
	SetShaderMode( "additive" );
	RenderSurfaces( camera->RenderFrustum() );
	
	//copy to visual 
	_viewport=camera->Viewport();
	SetShaderMode( "postprocess" );
	App.Graphics()->SetColorBuffer( 0,0 );
	App.Graphics()->SetDepthBuffer( 0 );
	App.Graphics()->SetShader( quadShader );
	App.Graphics()->SetTextureParam( "bb_QuadTexture",accumBuffer );
	RenderQuad();

	for( vector<CSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		CSurface *surface=*it;
		surface->OnEndCameraPass();
	}
	_surfaces.clear();
}
