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

#ifndef CAMERA_H
#define CAMERA_H

#include "entity.h"

class CCamera : public CEntity{
public:
	CCamera();
	
	void SetViewport( const CRect &viewport );
	const CRect &Viewport(){ return _viewport; }
	
	void SetProjectionMatrix( const CMat4 &matrix );
	const CMat4 &ProjectionMatrix(){ return _projection; }

	//frustum planes in order:
	//left, right, bottom, top, near, far
	const CHull &Frustum(){ return _frustum; }

	float NearZ(){ return -_frustum.planes[4].d; }
	float FarZ(){ return _frustum.planes[5].d; }
	
	//Only valid during rendering...
	const CHull &RenderFrustum(){ return _renderFrustum; }
	
	virtual void OnRenderWorld();
	
private:
	CCamera( CCamera *camera,CCopier *copier );
	
	CCamera *OnCopy( CCopier *copier ){ return new CCamera( this,copier ); }

	CRect _viewport;
	CMat4 _projection;
	CHull _frustum;
	CHull _renderFrustum;
	
};

#endif
