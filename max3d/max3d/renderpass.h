
#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "object.h"

class CRenderer;

class CRenderPass : public CObject{
public:
	virtual void Render( CRenderer *r )=0;
};

#endif
