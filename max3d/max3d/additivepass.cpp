
#include "std.h"

#include "app.h"
#include "additivepass.h"

void CAdditivePass::Render( CRenderer *r ){

	App.Graphics()->SetShaderMode( CShader::ModeForName( "additive" ) );
	App.Graphics()->SetFrameBuffer( r->AccumBuffer() );
	App.Graphics()->SetViewport( 0,0,1024,768 );
	App.Graphics()->SetWriteMask( WRITEMASK_RED|WRITEMASK_GREEN|WRITEMASK_BLUE );
	App.Graphics()->SetBlendFunc( BLENDFUNC_ONE,BLENDFUNC_ONE );
	App.Graphics()->SetDepthFunc( DEPTHFUNC_LE );
	App.Graphics()->SetCullMode( CULLMODE_BACK );

	r->RenderSurfaces( CHull() );//r->Frustum() );
}
