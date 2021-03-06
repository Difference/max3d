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
#include "scene.h"

static CHull _nullHull;

static CVertexBuffer *quadVB;
static CIndexBuffer *quadIB;
//static CShader *quadShader;

static CVertexBuffer *boxVB;
static CIndexBuffer *boxIB;

static CTexture *accumBuffer;		//for single renderpass
static CTexture *accumBuffer2;		//for multiple renderpasses
static CTexture *accumBuffer3;		//for more renderpasses
static CTexture *normalBuffer;
static CTexture *materialBuffer;
static CTexture *depthBuffer;

static CShader *clearShader;

static CShader *copyShader;
static CShader *halveShader;
static CShader *blur15Shader;

static int maxShadowBufSize;
static CTexture *shadowBuffer;
static CShader *shadowMapShader;

void scene_init(){

	int w=App.Graphics()->WindowWidth();
	int h=App.Graphics()->WindowHeight();
	
	App.Graphics()->SetVec2Param( "bb_WindowSize",CVec2( w,h ) );
	App.Graphics()->SetVec2Param( "bb_WindowScale",CVec2( 1.0f/w,1.0f/h ) );
	
	float vbdata[]={ 
		-1,-1,.999f,0,0,
		-1,1,.999f,0,1,
		1,1,.999f,1,1,
		1,-1,.999f,1,0 };
	int ibdata[]={0,1,2,0,2,3 };
	quadVB=App.Graphics()->CreateVertexBuffer( 4,"3f0f0f2f" );
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
	
	int color_fmt=FORMAT_RGBA8;
	if( App.Flags() & MAX3D_FLOATCOLORBUFFER ) color_fmt=FORMAT_RGBA16F;
	
	int normal_fmt=FORMAT_RGBA8;
	if( App.Flags() & MAX3D_FLOATNORMALBUFFER ) normal_fmt=FORMAT_RGBA16F;
	
	accumBuffer=App.Graphics()->CreateTexture( w,h,color_fmt,TEXTURE_CLAMPST|TEXTURE_FILTER|TEXTURE_RENDER|TEXTURE_RECTANGULAR );
	accumBuffer2=App.Graphics()->CreateTexture( w,h,color_fmt,TEXTURE_CLAMPST|TEXTURE_FILTER|TEXTURE_RENDER|TEXTURE_RECTANGULAR );
	accumBuffer3=App.Graphics()->CreateTexture( w/4,h/4,color_fmt,TEXTURE_CLAMPST|TEXTURE_FILTER|TEXTURE_RENDER|TEXTURE_RECTANGULAR );
	
	materialBuffer=App.Graphics()->CreateTexture( w,h,FORMAT_RGBA8,TEXTURE_CLAMPST|TEXTURE_RENDER|TEXTURE_RECTANGULAR );
	normalBuffer=App.Graphics()->CreateTexture( w,h,normal_fmt,TEXTURE_CLAMPST|TEXTURE_RENDER|TEXTURE_RECTANGULAR );
	depthBuffer=App.Graphics()->CreateTexture( w,h,FORMAT_DEPTH,TEXTURE_CLAMPST|TEXTURE_RENDER|TEXTURE_RECTANGULAR );

	App.Graphics()->SetTextureParam( "bb_AccumBuffer",accumBuffer );
	App.Graphics()->SetTextureParam( "bb_MaterialBuffer",materialBuffer );
	App.Graphics()->SetTextureParam( "bb_NormalBuffer",normalBuffer );
	App.Graphics()->SetTextureParam( "bb_DepthBuffer",depthBuffer );
		
	copyShader=(CShader*)App.ImportObject( "CShader", "copy.glsl" );
	halveShader=(CShader*)App.ImportObject( "CShader", "halve.glsl" );
	blur15Shader=(CShader*)App.ImportObject( "CShader", "blur15.glsl" );

	clearShader=(CShader*)App.ImportObject( "CShader", "clear.glsl" );
	shadowMapShader=(CShader*)App.ImportObject( "CShader", "shadowmap.glsl" );
}

CScene::CScene():
_renderNest(0){
}

CScene::~CScene(){
}

void CScene::Clear(){
	_cameras.clear();
	_lights.clear();
	_surfaces.clear();
}

void CScene::AddCamera( CCamera *camera ){
	_cameras.push_back( camera );
}

void CScene::AddLight( CLight *light ){
	_lights.push_back( light );
}

void CScene::AddSurface( CSurface *surface ){
	_surfaces.push_back( surface );
}

void CScene::AddRenderPass( CRenderPass *pass ){
	_passes.push_back( pass );
}

void CScene::ClearRenderPasses(){
	_passes.clear();
}

void CScene::SetShadowsEnabled( bool enabled ){
	_shadowsEnabled=enabled;
}

void CScene::SetShaderMode( string mode ){
	int shaderMode=CShader::ModeForName( mode );
	App.Graphics()->SetShaderMode( shaderMode );
	_modeMask=1<<shaderMode;
	if( mode=="ambient" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( depthBuffer );
		App.Graphics()->SetViewport( _viewport );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE|WRITEMASK_ALPHA|WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );
	}else if( mode=="clear" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( depthBuffer );
		App.Graphics()->SetViewport( _viewport );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE|WRITEMASK_ALPHA );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_EQ );
		App.Graphics()->SetCullMode( CULLMODE_NONE );
	}else if( mode=="shadow" ){
		App.Graphics()->SetColorBuffer( 0,0 );
		App.Graphics()->SetDepthBuffer( shadowBuffer );
		App.Graphics()->SetViewport( CRect( maxShadowBufSize/2-_shadowBufSize/2,maxShadowBufSize/2-_shadowBufSize/2,_shadowBufSize,_shadowBufSize ) );
		App.Graphics()->SetWriteMask( WRITEMASK_DEPTH );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );
	}else if( mode=="shadowmap" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( 0 );
		App.Graphics()->SetViewport( _viewport );
		App.Graphics()->SetWriteMask( WRITEMASK_ALPHA );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
	}else if( mode=="spotlight" || mode=="pointlight" || mode=="distantlight" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( 0 );
		App.Graphics()->SetViewport( _viewport );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( _shadowBufSize ? BLENDFUNC_DSTALPHA : BLENDFUNC_ONE,BLENDFUNC_ONE );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_FRONT );
	}else if( mode=="additive" ){
		App.Graphics()->SetColorBuffer( 0,accumBuffer );
		App.Graphics()->SetDepthBuffer( depthBuffer );
		App.Graphics()->SetViewport( _viewport );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );
	}else if( mode=="postprocess" ){
		App.Graphics()->SetColorBuffer( 0,0 );
		App.Graphics()->SetDepthBuffer( 0 );
		App.Graphics()->SetViewport( _viewport );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
		App.Graphics()->SetCullMode( CULLMODE_NONE );
	}
}

void CScene::RenderQuad(){
	CRect vp=App.Graphics()->Viewport();
	float quadVBData[]={
		/*
		-1,-1,.999f,0,0,
		-1,1,.999f,0,vp.height,
		1,1,.999f,vp.width,vp.height,
		1,-1,.999f,vp.width,0 };
		*/
		-1,-1,1,0,0,
		-1,1,1,0,vp.height,
		1,1,1,vp.width,vp.height,
		1,-1,1,vp.width,0 };
	quadVB->SetData( quadVBData );
	App.Graphics()->SetVertexBuffer( quadVB );
	App.Graphics()->SetIndexBuffer( quadIB );
	App.Graphics()->Render( 3,0,6,1 );
}

void CScene::RenderBox( const CBox &box ){
	CVec3 vp[8];
	for( int i=0;i<8;++i ) vp[i]=box.Corner(i);
	boxVB->SetData( vp );
	App.Graphics()->SetVertexBuffer( boxVB );
	App.Graphics()->SetIndexBuffer( boxIB );
	App.Graphics()->Render( 3,0,36,1 );
}

void CScene::RenderSurfaces( const CHull &bounds ){

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

void CScene::SetShadowBufferSize( int size ){
	size=(int(size+1)&~1);
	if( size>maxShadowBufSize ){
		maxShadowBufSize=size;
		if( shadowBuffer ) shadowBuffer->Release();
		shadowBuffer=App.Graphics()->CreateTexture( size,size,FORMAT_DEPTH,TEXTURE_CLAMPST|TEXTURE_RENDER );
		App.Graphics()->SetTextureParam( "bb_ShadowBuffer",shadowBuffer );
	}
	_shadowBufSize=size;
	App.Graphics()->SetFloatParam( "bb_ShadowMapScale",float(size)/float(maxShadowBufSize) );
}

void CScene::RenderSpotLight( CLight *light,CCamera *camera ){
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
	
	int sz=_shadowsEnabled && _renderNest==1 ? light->ShadowSize() : 0;
	if( !sz ){
		SetShaderMode( "spotlight" );
		App.Graphics()->SetShader( light->Shader() );
		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",light->RenderMatrix() );
		RenderBox( lightBox );
		return;
	}
	const CMat4 &camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	float d=camMat.Translation().Distance( light->RenderMatrix().Translation() );
	sz=int( range/d * sz );
	if( sz>light->ShadowSize() ) sz=light->ShadowSize();
	SetShadowBufferSize( sz );
	
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

void CScene::RenderPointLight( CLight *light,CCamera *camera ){
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

	int sz=_shadowsEnabled && _renderNest==1 ? light->ShadowSize() : 0;
	if( !sz ){
		SetShaderMode( "pointlight" );
		App.Graphics()->SetShader( light->Shader() );
		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",light->RenderMatrix() );
		RenderBox( lightBox );
		return;
	}
	CMat4 camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	float d=camMat.Translation().Distance( light->RenderMatrix().Translation() );
	sz=int( range/d * sz );
	if( sz>light->ShadowSize() ) sz=light->ShadowSize();
	SetShadowBufferSize( sz );
	
	CHull rhull=light->RenderMatrix() * lightBox;

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

void CScene::RenderDistantLight( CLight *light,CCamera *camera ){
	CBox lightBox=CBox( CVec3(-128),CVec3(128) );

	CMat4 camMat=App.Graphics()->Mat4Param( "bb_CameraMatrix" );

	App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );
	App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );

	int sz=_shadowsEnabled && _renderNest==1 ? light->ShadowSize() : 0;
	if( !sz ){
		SetShaderMode( "distantlight" );
		App.Graphics()->SetShader( light->Shader() );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
		RenderBox( lightBox );
		return;
	}
	SetShadowBufferSize( sz );
	
	const vector<float> &splits=light->ShadowSplits();
	
	CMat4 cameraLightMatrix=light->InverseRenderMatrix() * camMat;
	
	const CHull &frustum=camera->Frustum();

	for( int i=0;i<splits.size()-1;++i ){
		float znear=splits[i],zfar=splits[i+1];
		
		//Now handles off-centre frustums, eg: mirrors...
		float lnear=frustum.planes[0].SolveX( 0,znear );
		float rnear=frustum.planes[1].SolveX( 0,znear );
		float bnear=frustum.planes[2].SolveY( 0,znear );
		float tnear=frustum.planes[3].SolveY( 0,znear );
		float lfar=frustum.planes[0].SolveX( 0,zfar );
		float rfar=frustum.planes[1].SolveX( 0,zfar );
		float bfar=frustum.planes[2].SolveY( 0,zfar );
		float tfar=frustum.planes[3].SolveY( 0,zfar );
		
		CVec3 verts[]={
		CVec3(lnear,tnear,znear),CVec3(rnear,tnear,znear),
		CVec3(rnear,bnear,znear),CVec3(lnear,bnear,znear),
		CVec3(lfar,tfar,zfar),CVec3(rfar,tfar,zfar),
		CVec3(rfar,bfar,zfar),CVec3(lfar,bfar,zfar) };
		
		CBox box=CBox::Empty();
		for( int j=0;j<8;++j ){
			box.Update( cameraLightMatrix * verts[j] );
		}
		CMat4 shadowProjectionMatrix=CMat4::OrthoMatrix( box.a.x,box.b.x,box.a.y,box.b.y,-512,512 );

		App.Graphics()->SetMat4Param( "bb_ShadowProjectionMatrix",shadowProjectionMatrix );

		//render models to shadowmap
		SetShaderMode( "shadow" );
		App.Graphics()->Clear();
		RenderSurfaces( _nullHull );

		//render shadows
		SetShaderMode( "shadowmap" );
		App.Graphics()->SetShader( shadowMapShader );
		App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
		App.Graphics()->SetFloatParam( "bb_ShadowNearClip",znear );
		App.Graphics()->SetFloatParam( "bb_ShadowFarClip",zfar );
		RenderBox( lightBox );
	}

	//render light
	SetShaderMode( "distantlight" );
	App.Graphics()->SetShader( light->Shader() );
	App.Graphics()->SetMat4Param( "bb_ModelMatrix",camMat );
	RenderBox( lightBox );
}

void CScene::RenderCamera( CCamera *camera ){
	++_renderNest;

	for( vector<CSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		CSurface *surface=*it;
		surface->OnRenderScene( camera );
	}

	//push graphics target state
	CTexture *bufStates[5];
	for( int i=0;i<4;++i ){
		bufStates[i]=App.Graphics()->ColorBuffer( i );
	}
	bufStates[4]=App.Graphics()->DepthBuffer();
	
	for( vector<CSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		CSurface *surface=*it;
		surface->OnRenderCamera( camera );
	}

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
	
	const CHull &frustum=camera->Frustum();
	float left=frustum.planes[0].SolveX( 0,1 );
	float right=frustum.planes[1].SolveX( 0,1 );
	float bottom=frustum.planes[2].SolveY( 0,1 );
	float top=frustum.planes[3].SolveY( 0,1 );
	float xscale=(right-left)/_viewport.width;
	float yscale=(top-bottom)/_viewport.height;
	App.Graphics()->SetVec2Param( "bb_FragScale",CVec2( xscale,yscale ) );
	App.Graphics()->SetVec2Param( "bb_FragOffset",CVec2( left,bottom ) );

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
	RenderQuad();

	App.Graphics()->SetColorBuffer( 1,0 );
	App.Graphics()->SetColorBuffer( 2,0 );
	
	//additive
	SetShaderMode( "additive" );
	RenderSurfaces( camera->RenderFrustum() );

	//lighting
	for( vector<CLight*>::const_iterator it=_lights.begin();it!=_lights.end();++it ){
		_shadowBufSize=0;
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
	
	//execute render passes
	vector<CRenderPass*> passes=_passes;
	if( !passes.size() ){
		CRenderPass *pass=new CRenderPass;
		pass->SetShader( copyShader );
		passes.push_back( pass );
	}

	SetShaderMode( "postprocess" );
	
	CTexture *colorBuf=accumBuffer;

	for( int i=0;i<passes.size();++i ){

		CRenderPass *pass=passes[i];
	
		CShader *shader=pass->Shader();
		for( vector<CParam*>::const_iterator it=shader->Params().begin();it!=shader->Params().end();++it ){
			CParam *p=*it;
			if( p->Name()=="bb_BlurBuffer" ){
				
				CTexture *colorBuf2=(colorBuf==accumBuffer) ? accumBuffer2 : accumBuffer;
				
				App.Graphics()->SetShader( halveShader );

				App.Graphics()->SetTextureParam( "bb_ColorBuffer",colorBuf );
				App.Graphics()->SetColorBuffer( 0,colorBuf2 );
				App.Graphics()->SetViewport( CRect( 0,0,_viewport.width/2,_viewport.height/2 ) );
				App.Graphics()->SetVec2Param( "bb_ViewportSize",CVec2( _viewport.width/2,_viewport.height/2 ) );
				RenderQuad();
				
				App.Graphics()->SetTextureParam( "bb_ColorBuffer",colorBuf2 );
				App.Graphics()->SetColorBuffer( 0,accumBuffer3 );
				App.Graphics()->SetViewport( CRect( 0,0,_viewport.width/4,_viewport.height/4 ) );
				App.Graphics()->SetVec2Param( "bb_ViewportSize",CVec2( _viewport.width/4,_viewport.height/4 ) );
				RenderQuad();
				
				App.Graphics()->SetShader( blur15Shader );

				//Mac GLSL still a PITA - you still can't init arrays!
				float blurFactors[]={ 0.0044,0.0115,0.0257,0.0488,0.0799,0.1133,0.1394,0.1494,0.1394,0.1133,0.0799,0.0488,0.0257,0.0115,0.0044 };
				CParam::ForName( "BlurFactors" )->SetFloatValue( 15,blurFactors );
					
				App.Graphics()->SetVec2Param( "BlurScale",CVec2( 1.0f,0.0f ) );
				App.Graphics()->SetTextureParam( "bb_ColorBuffer",accumBuffer3 );
				App.Graphics()->SetColorBuffer( 0,colorBuf2 );
				App.Graphics()->SetViewport( CRect( 0,0,_viewport.width/4,_viewport.height/4 ) );
				RenderQuad();
				
				App.Graphics()->SetVec2Param( "BlurScale",CVec2( 0.0f,1.0f ) );
				App.Graphics()->SetTextureParam( "bb_ColorBuffer",colorBuf2 );
				App.Graphics()->SetColorBuffer( 0,accumBuffer3 );
				App.Graphics()->SetViewport( CRect( 0,0,_viewport.width/4,_viewport.height/4 ) );
				RenderQuad();
				
				App.Graphics()->SetTextureParam( "bb_BlurBuffer",accumBuffer3 );
				App.Graphics()->SetVec2Param( "bb_ViewportSize",CVec2( _viewport.width,_viewport.height ) );
				break;
			}
		}
		
		App.Graphics()->SetTextureParam( "bb_ColorBuffer",colorBuf );

		if( i==passes.size()-1 ){
			//last pass!
			colorBuf=bufStates[0];
			_viewport=camera->Viewport();
			App.Graphics()->SetViewport( _viewport );
		}else{
			colorBuf=(colorBuf==accumBuffer) ? accumBuffer2 : accumBuffer;
		}
		
		App.Graphics()->SetColorBuffer( 0,colorBuf );
		App.Graphics()->SetShader( pass->Shader() );
		if( pass->Material() ) pass->Material()->Bind();
		RenderQuad();
	}

	if( !_passes.size() ){
		passes.back()->Release();
	}

	//pop graphics target state
	for( int i=0;i<4;++i ){
		App.Graphics()->SetColorBuffer( i,bufStates[i] );
	}
	App.Graphics()->SetDepthBuffer( bufStates[4] );
	
	--_renderNest;
}

/*
void CScene::RenderQuad( const CRect &rect,CShader *shader,CMaterial *material ){
	SetShaderMode( "postprocess" );
	App.Graphics()->SetShader( shader );
	if( material ) material->Bind();
	RenderQuad();
}
*/
