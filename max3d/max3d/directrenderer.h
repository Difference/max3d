
#ifndef DIRECTRENDERER_H
#define DIRECTRENDERER_H

#include "renderer.h"

class CDirectRenderer : public CRenderer{
public:
	CDirectRenderer();

protected:
	virtual void RenderCamera();
};

#endif
