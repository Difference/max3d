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

#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "light.h"
#include "surface.h"
#include "renderpass.h"

class CScene : public CObject{
public:
	CScene();
	~CScene();
	
	void Clear();

	void AddCamera( CCamera *camera );
	const vector<CCamera*> &Cameras(){ return _cameras; }
	
	void AddLight( CLight *light );
	const vector<CLight*> &Lights(){ return _lights; }
	
	void AddSurface( CSurface *surface );
	const vector<CSurface*> &Surfaces(){ return _surfaces; }
	
	void AddRenderPass( CRenderPass *pass );
	void ClearRenderPasses();
	
	void SetShadowsEnabled( bool enabled );
	bool ShadowsEnabled(){ return _shadowsEnabled; }
	
	void RenderCamera( CCamera *camera );
	
	void RenderQuad( const CRect &rect,CShader *shader,CMaterial *material );

private:
	void SetShaderMode( string mode );
	void SetShadowBufferSize( int size );

	void RenderQuad();
	void RenderBox( const CBox &box );
	void RenderSurfaces( const CHull &bounds );

	void RenderSpotLight( CLight *light,CCamera *camera );
	void RenderPointLight( CLight *light,CCamera *camera );
	void RenderDistantLight( CLight *light,CCamera *camera );
	
	int _modeMask;
	int _shadowBufSize;
	CRect _viewport;
	bool _shadowsEnabled;
	int _renderNest;

	vector<CCamera*> _cameras;
	vector<CLight*> _lights;
	vector<CSurface*> _surfaces;
	
	vector<CRenderPass*> _passes;
};

#endif
