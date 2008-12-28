
#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H

#include "renderer.h"

class CDeferredRenderer : public CRenderer{
public:
	CDeferredRenderer();

protected:
	virtual void RenderCamera();

	void RenderPointLight( CLight *light );
	void RenderDistantLight( CLight *light );

	void RenderQuad();
	void RenderBox( const CBox &box );

	CFrameBuffer *FrameBuffer();
	CFrameBuffer *AccumBuffer();
	CTexture *AccumTexture();
	CTexture *ColorTexture();
	CTexture *NormalTexture();
	CTexture *DepthTexture();
};

#endif
