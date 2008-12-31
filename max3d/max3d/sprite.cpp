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
#include "sprite.h"

static map<CMaterial*,CSpriteSurface*> _matMap;

CSprite::CSprite():_surface(0){
}

CSprite::CSprite( CSprite *sprite,CCopier *copier ):CEntity( sprite,copier ),
_surface( sprite->_surface ){
	if( _surface ) _surface->Retain();
}

CSprite::~CSprite(){
	if( _surface ) _surface->Release();
}

void CSprite::SetMaterial( CMaterial *material ){
	CSpriteSurface *surface=0;
	if( material ){
		map<CMaterial*,CSpriteSurface*>::iterator it=_matMap.find( material );
		if( it==_matMap.end() ){
			surface=new CSpriteSurface;
			surface->SetShader( App.ShaderUtil()->SpriteShader() );
			surface->SetMaterial( material );
			_matMap.insert( make_pair( material,surface ) );
		}else{
			surface=it->second;
		}
	}
	if( surface ) surface->Retain();
	if( _surface ) _surface->Release();
	_surface=surface;
}

CMaterial *CSprite::Material(){
	return _surface->Material();
}

void CSprite::OnRenderWorld(){
	if( !_surface->Instances().size() ) App.Scene()->AddSurface( _surface );
	_surface->Instances().push_back( this );
}

CSpriteSurface::CSpriteSurface():_capacity(0),_vertexBuffer(0),_indexBuffer(0){
	SetShader( App.ShaderUtil()->SpriteShader() );
}

CSpriteSurface::~CSpriteSurface(){
	if( _vertexBuffer ) _vertexBuffer->Release();
	if( _indexBuffer ) _indexBuffer->Release();
}

void CSpriteSurface::OnRenderCamera( CCamera *camera ){
	int n=_instances.size();
	if( n>_capacity ){
		_capacity=n;
		
		const CVec3 norm(0,0,-1);
		const CVec4 tang0(1,0,0,1),tang1(0,-1,0,1);
		const CVec4 tang2(-1,0,0,1),tang3(0,1,0,1);
		const CVec2 tc0(0,0),tc1(1,0),tc2(1,1),tc3(0,1);
		
		if( _vertexBuffer ) _vertexBuffer->Release();
		_vertexBuffer=App.Graphics()->CreateVertexBuffer( n*4,"3f3f4f2f" );
		float *vp=(float*)_vertexBuffer->Lock();
		for( int i=0;i<_capacity;++i ){
			memcpy( vp+3,&norm,12 );
			memcpy( vp+6,&tang0,16 );
			memcpy( vp+10,&tc0,8 );
			vp+=12;
			memcpy( vp+3,&norm,12 );
			memcpy( vp+6,&tang1,16 );
			memcpy( vp+10,&tc1,8 );
			vp+=12;
			memcpy( vp+3,&norm,12 );
			memcpy( vp+6,&tang2,16 );
			memcpy( vp+10,&tc2,8 );
			vp+=12;
			memcpy( vp+3,&norm,12 );
			memcpy( vp+6,&tang3,16 );
			memcpy( vp+10,&tc3,8 );
			vp+=12;
		}
		_vertexBuffer->Unlock();
		
		if( _indexBuffer ) _indexBuffer->Release();
		_indexBuffer=App.Graphics()->CreateIndexBuffer( n*6,"1i" );
		int *ip=(int*)_indexBuffer->Lock();
		for( int i=0;i<_capacity;++i ){
			int v=i*4;
			ip[0]=v;ip[1]=v+1,ip[2]=v+2;
			ip[3]=v;ip[4]=v+2;ip[5]=v+3;
			ip+=6;
		}
		_indexBuffer->Unlock();
	}
	const CVec3 v0(-1,1,0),v1(1,1,0),v2(1,-1,0),v3(-1,-1,0);
	const CMat4 &viewMat=camera->InverseRenderMatrix();
	float *vp=(float*)_vertexBuffer->Lock();
	for( int i=0;i<n;++i ){
		CVec3 t=viewMat * _instances[i]->RenderMatrix().Translation();
		*(CVec3*)vp=t+v0;
		*(CVec3*)(vp+12)=t+v1;
		*(CVec3*)(vp+24)=t+v2;
		*(CVec3*)(vp+36)=t+v3;
		vp+=48;
	}
	_vertexBuffer->Unlock();
}

void CSpriteSurface::OnRenderInstances( const CHull &bounds ){
	App.Graphics()->SetVertexBuffer( _vertexBuffer );
	App.Graphics()->SetIndexBuffer( _indexBuffer );
	App.Graphics()->Render( 3,0,_instances.size()*6,1 );
}

void CSpriteSurface::OnClearInstances(){
	_instances.clear();
}

REGISTERTYPE( CSprite )
