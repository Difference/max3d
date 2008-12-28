
#include "std.h"

#include "app.h"
#include "tonemappingpass.h"

static CTexture *blur1Map;
static CTexture *blur2Map;
static CTexture *blur3Map;
static CFrameBuffer *blur1FrameBuf;
static CFrameBuffer *blur2FrameBuf;
static CFrameBuffer *blur3FrameBuf;
static CShader *blurShader;
static CShader *quadShader;
static CShader *tonemapShader;

void CToneMappingPass::Render( CRenderer *r ){

	App.Graphics()->SetShaderMode( CShader::ModeForName( "postprocess" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_T );
	App.Graphics()->SetCullMode( CULLMODE_NONE );
	App.Graphics()->SetShader( quadShader );

	bool hdr=false;//true;//false;

	if( hdr ){

		App.Graphics()->SetTextureParam( "bb_QuadTexture",r->AccumTexture() );
		App.Graphics()->SetFrameBuffer( blur1FrameBuf );
		App.Graphics()->SetViewport( 0,0,512,384 );
		r->RenderQuad();

		App.Graphics()->SetTextureParam( "bb_QuadTexture",blur1Map );
		App.Graphics()->SetFrameBuffer( blur2FrameBuf );
		App.Graphics()->SetViewport( 0,0,256,192 );
		r->RenderQuad();

		App.Graphics()->SetShader( blurShader );

		App.Graphics()->SetTextureParam( "bb_QuadTexture",blur2Map );
		App.Graphics()->SetFrameBuffer( blur3FrameBuf );
		App.Graphics()->SetViewport( 0,0,256,192 );
		App.Graphics()->SetVec2Param( "bb_BlurScale",CVec2( 1.0f/256.0f,0.0f ) );
		r->RenderQuad();

		App.Graphics()->SetTextureParam( "bb_QuadTexture",blur3Map );
		App.Graphics()->SetFrameBuffer( blur2FrameBuf );
		App.Graphics()->SetViewport( 0,0,256,192 );
		App.Graphics()->SetVec2Param( "bb_BlurScale",CVec2( 0.0f,1.0f/192.0f ) );
		r->RenderQuad();

		App.Graphics()->SetShader( tonemapShader );

		App.Graphics()->SetTextureParam( "bb_QuadTexture",blur2Map );
		App.Graphics()->SetFrameBuffer( 0 );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		r->RenderQuad();

	}else{

		App.Graphics()->SetTextureParam( "bb_QuadTexture",r->AccumTexture() );
		App.Graphics()->SetFrameBuffer( 0 );
		App.Graphics()->SetViewport( 0,0,1024,768 );
		r->RenderQuad();
	}
}

void tonemappingpass_init(){
	blur1Map=App.Graphics()->Create2dTexture( 512,384,FORMAT_ARGB16F,TEXTURE_FILTER|TEXTURE_CLAMPST );
	CTexture *renderTargets0[]={blur1Map,0,0,0};
	blur1FrameBuf=App.Graphics()->CreateFrameBuffer( renderTargets0,0 );

	blur2Map=App.Graphics()->Create2dTexture( 256,192,FORMAT_ARGB16F,TEXTURE_FILTER|TEXTURE_CLAMPST );
	CTexture *renderTargets1[]={blur2Map,0,0,0};
	blur2FrameBuf=App.Graphics()->CreateFrameBuffer( renderTargets1,0 );

	blur3Map=App.Graphics()->Create2dTexture( 256,192,FORMAT_ARGB16F,TEXTURE_FILTER|TEXTURE_CLAMPST );
	CTexture *renderTargets2[]={blur3Map,0,0,0};
	blur3FrameBuf=App.Graphics()->CreateFrameBuffer( renderTargets2,0 );

	blurShader=App.ShaderUtil()->LoadShader( "blur" );
	quadShader=App.ShaderUtil()->LoadShader( "quad" );
	tonemapShader=App.ShaderUtil()->LoadShader( "tonemap" );
}
