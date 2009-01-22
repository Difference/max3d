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

#ifndef MIRROR_H
#define MIRROR_H

#include "model.h"

class CMirrorSurface;

class CMirror : public CEntity{
public:
	CMirror();
	~CMirror();
	
	void SetSize( float width,float height );
	void SetResolution( int width,int height );

	virtual void OnRenderWorld();

private:
	friend class CMirrorSurface;

	CSurface *Surface();
	CTexture *Texture();
	void OnRenderScene( CCamera *camera );	//forwarded from surface
	
	int _dirty;
	float _width,_height;
	int _rWidth,_rHeight;
	CCamera *_camera;
	CTexture *_texture;
	CMaterial *_material;
	CMirrorSurface *_surface;
};

class CMirrorSurface : public CModelSurface{
public:
	CMirrorSurface( CMirror *mirror );
	
	virtual void OnRenderScene( CCamera *camera );

private:
	CMirror *_mirror;
};

#endif
