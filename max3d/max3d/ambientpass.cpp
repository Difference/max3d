
#include "std.h"

#include "app.h"
#include "ambientpass.h"

void CAmbientPass::Render( CRenderer *r ){

	App.Graphics()->SetShaderMode( CShader::ModeForName( "ambient" ) );
	App.Graphics()->SetFrameBuffer( r->FrameBuffer() );
	App.Graphics()->SetViewport( 0,0,1024,768 );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE|WRITEMASK_DEPTH );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ZERO );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
	App.Graphics()->SetCullMode( CULLMODE_BACK );

	App.Graphics()->Clear();

	r->RenderSurfaces( r->Frustum() );
}
