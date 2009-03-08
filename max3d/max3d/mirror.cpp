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
	_surface=new CMirrorSurface( this );
	_surface->SetShader( App.ShaderUtil()->MirrorShader() );
	_surface->SetMaterial( _material );
}

CMirror::~CMirror(){
	if( _texture ) _texture->Release();
	_material->Release();
	_surface->Release();
	delete _camera;
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

CSurface *CMirror::Surface(){
	if( _dirty & DIRTY_SURFACE ){
		CVertex v0,v1,v2,v3;
		v0.position=CVec3( -_width/2,_height/2,0 );
		v0.normal=CVec3(0,0,-1);
		v0.texcoords[0]=CVec2( 1,1 );
		v1.position=CVec3( _width/2,_height/2,0 );
		v1.normal=CVec3(0,0,-1);
		v1.texcoords[0]=CVec2( 0,1 );
		v2.position=CVec3( _width/2,-_height/2,0 );
		v2.normal=CVec3(0,0,-1);
		v2.texcoords[0]=CVec2( 0,0 );
		v3.position=CVec3( -_width/2,-_height/2,0 );
		v3.normal=CVec3(0,0,-1);
		v3.texcoords[0]=CVec2( 1,0 );
		_surface->Clear();
		_surface->AddVertex( v0 );
		_surface->AddVertex( v1 );
		_surface->AddVertex( v2 );
		_surface->AddVertex( v3 );
		_surface->AddTriangle( 0,1,2 );
		_surface->AddTriangle( 0,2,3 );
		_dirty&=~DIRTY_SURFACE;
	}
	return _surface;
}

CTexture *CMirror::Texture(){
	if( _dirty & DIRTY_TEXTURE ){
		if( _texture ) _texture->Release();
		_texture=App.Graphics()->CreateTexture( _rWidth,_rHeight,FORMAT_RGB8,TEXTURE_FILTER|TEXTURE_CLAMPST|TEXTURE_RENDER );
		_material->SetTexture( "MirrorTexture",_texture );
		_camera->SetViewport( CRect( 0,0,_rWidth,_rHeight ) );
		_dirty&=~DIRTY_TEXTURE;
	}
	return _texture;
}

void CMirror::OnRenderWorld(){
	App.Scene()->AddSurface( Surface() );
}

//Forwared here from CMirrorSurface
void CMirror::OnRenderScene( CCamera *camera ){
	_surface->Instances().clear();
	
	//No recursion yet...
	if( camera==_camera ) return;

	//frustum clipping.
	//Note: CModelSurface still clips itself - add enable/disable clipping for CModelSurface
	CBox bounds=RenderMatrix() * CBox( CVec3( -_width/2,-_height/2,0 ),CVec3( _width/2,_height/2,0 ) );
	if( !camera->RenderFrustum().Intersects( bounds ) ) return;
	
	const CMat4 &mir=RenderMatrix();
	const CMat4 &cam=camera->RenderMatrix();

	//put eye into mirror space
	CVec3 eye=-mir * cam.t.xyz();
	if( eye.z>0 ) return;

	//reflect
	eye.z=-eye.z;

	//projection matrix
	float znear=eye.z;
	float zfar=256.0f;
	float left=(-_width/2-eye.x),right=left+_width;
	float bottom=(-_height/2+eye.y),top=bottom+_height;
	CMat4 proj=CMat4::FrustumMatrix( left,right,bottom,top,znear,zfar );

	//camera matrix
	CMat4 mat;
	mat.i.x=-1;
	mat.k.z=-1;
	mat.t.xyz()=eye;
	mat=mir * mat;

	_camera->SetWorldMatrix( mat );
	_camera->SetProjectionMatrix( proj );
	
	CTexture *cb=App.Graphics()->ColorBuffer( 0 );
	App.Graphics()->SetColorBuffer( 0,Texture() );
	App.Scene()->RenderCamera( _camera );
	App.Graphics()->SetColorBuffer( 0,cb );
	
	_surface->Instances().push_back( mir );
}

CMirrorSurface::CMirrorSurface( CMirror *mirror ):
_mirror( mirror ){
}

void CMirrorSurface::OnRenderScene( CCamera *camera ){
	_mirror->OnRenderScene( camera );
}
