
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "graphics.h"
#include "material.h"

class CRenderPass : public CResource{
public:
	CRenderPass();
	~CRenderPass();
	
	void SetShader( CShader *shader );
	CShader *Shader(){ return _shader; }
	
	void SetMaterial( CMaterial *material );
	CMaterial *Material(){ return _material; }
	
private:
	CShader *_shader;
	CMaterial *_material;
};

#endif
