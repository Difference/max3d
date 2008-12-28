
#include "std.h"

#include "app.h"
#include "directrenderer.h"

CDirectRenderer::CDirectRenderer(){
}

void CDirectRenderer::RenderCamera(){

	//ambient pass...
	App.Graphics()->SetShaderMode( CShader::ModeForName( "ambient" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE|WRITEMASK_DEPTH );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
	App.Graphics()->SetCullMode( CULLMODE_BACK );
	App.Graphics()->Clear();
	RenderSurfaces( _frustum );

	//lighting passes...
	for( vector<CLight*>::const_iterator it=_lights.begin();it!=_lights.end();++it ){
		CLight *light=*it;
		switch( light->Type() ){
		case 1:App.Graphics()->SetShaderMode( CShader::ModeForName( "pointlight" ) );break;
		case 2:App.Graphics()->SetShaderMode( CShader::ModeForName( "distantlight" ) );break;
		default:Error( "TODO" );
		}
		App.Graphics()->SetFloatParam( "bb_LightRange",light->Range() );
		App.Graphics()->SetVec3Param( "bb_LightColor",light->Color() );
		App.Graphics()->SetMat4Param( "bb_LightMatrix",light->RenderMatrix() );
		App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
		App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
		App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
		App.Graphics()->SetCullMode( CULLMODE_BACK );
		RenderSurfaces( _frustum );
	}

	//additive pass...
	App.Graphics()->SetShaderMode( CShader::ModeForName( "additive" ) );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
	App.Graphics()->SetCullMode( CULLMODE_BACK );
	RenderSurfaces( CHull() );//r->Frustum() );
}

