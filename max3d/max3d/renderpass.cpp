
#include "std.h"

#include "renderpass.h"

CRenderPass::CRenderPass():
_shader(0),
_material(0){
}

CRenderPass::~CRenderPass(){
	if( _shader ) _shader->Release();
	if( _material ) _material->Release();
}

void CRenderPass::SetShader( CShader *shader ){
	CResource::Assign( &_shader,shader );
}

void CRenderPass::SetMaterial( CMaterial *material ){
	CResource::Assign( &_material,material );
}
