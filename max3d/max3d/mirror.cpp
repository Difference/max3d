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

#include "std.h"

#include "app.h"
#include "mirror.h"

enum{
	DIRTY_SURFACE=1,
	DIRTY_TEXTURE=2
};

CMirror::CMirror():
_width(1),
_height(1),
_rWidth(256),
_rHeight(256),
_texture(0),
_dirty(~0){
	_camera=new CCamera;
	_material=new CMaterial;
	_surface=new CModelSurface;
	_surface->SetMaterial( _material );
}

CMirror::~CMirror(){
	delete _camera;
	if( _texture ) _texture->Release();
	_material->Release();
	_surface->Release();
}

void CMirror::SetSize( float width,float height ){
	_width=width;
	_height=height;
	_dirty|=DIRTY_SURFACE;
}

void CMirror::SetResolution( int width,int height ){
	_rWidth=width;
	_rHeight=height;
	_dirty|=DIRTY_TEXTURE;
}
/*
void CMirror::OnBeginCameraPass( CCamera *camera ){
	if( _dirty & DIRTY_TEXTURE ){
		if( _texture ) _texture->Release();
		_texture=App.Graphics()->CreateTexture( _rWidth,_rHeight,FORMAT_RGB8,TEXTURE_FILTER|TEXTURE_CLAMPST|TEXTURE_RENDER );
		_material->SetTexture( "MirrorTexture",_texture );
		_dirty&=~DIRTY_TEXTURE;
	}
	App.Renderer()->Render( camera,_texture );
}
*/
void CMirror::OnRender(){
	if( _dirty & DIRTY_SURFACE ){
		CVertex v0,v1,v2,v3;
		v0.position=CVec3( -_width/2,_height/2,0 );
		v0.texcoords[0]=CVec2( 0,0 );
		v1.position=CVec3( _width/2,_height/2,0 );
		v1.texcoords[0]=CVec2( 1,0 );
		v2.position=CVec3( _width/2,-_height/2,0 );
		v2.texcoords[0]=CVec2( 1,1 );
		v3.position=CVec3( -_width/2,-_height/2,0 );
		v3.texcoords[0]=CVec2( 0,1 );
		_surface->Clear();
		_surface->AddVertex( v0 );
		_surface->AddVertex( v1 );
		_surface->AddVertex( v2 );
		_surface->AddVertex( v3 );
		_surface->AddTriangle( 0,1,2 );
		_surface->AddTriangle( 0,2,3 );
		_dirty&=~DIRTY_SURFACE;
	}
}


