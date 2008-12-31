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
#include "terrain.h"

enum{
	DIRTY_SURFACE=1,
	DIRTY_MATERIAL=2
};

CTerrain::CTerrain():
_data(0),
_dirty(0),
_surface( new CModelSurface ){
}

CTerrain::~CTerrain(){
	_surface->Release();
}

void CTerrain::SetShader( CShader *shader ){
	_surface->SetShader( shader );
}

CShader *CTerrain::Shader(){
	return _surface->Shader();
}

void CTerrain::SetMaterial( CMaterial *material ){
	_surface->SetMaterial( material );
}

CMaterial *CTerrain::Material(){
	return _surface->Material();
}

void CTerrain::SetData( int xsize,int zsize,float width,float height,float depth,const unsigned char *data ){
	_xsize=xsize;
	_zsize=zsize;
	_scale.x=width/(_xsize-1),_trans.x=-width/2;
	_scale.z=depth/(_zsize-1),_trans.z=-depth/2;
	_scale.y=height,_trans.y=0.0f;
	if( _data ) delete[] _data;
	_data=new float[_xsize*_zsize];
	if( data ){
		float *p=_data;
		for( int z=0;z<_zsize;++z ){
			for( int x=0;x<_xsize;++x ){
				*p++=*data++;
			}
		}
	}else{
		memset( _data,0,_xsize*_zsize*4 );
	}
	_dirty|=DIRTY_SURFACE;
}

void CTerrain::SetHeight( float height,int x,int z ){
	_data[z*_xsize+x]=height;
	_dirty|=DIRTY_SURFACE;
}

CModelSurface *CTerrain::Surface(){
	ValidateSurface();
	return _surface;
}

void CTerrain::OnRenderWorld(){
	ValidateSurface();
	if( !_surface->Instances().size() ) App.Scene()->AddSurface( _surface );
	_surface->Instances().push_back( RenderMatrix() );
	
}

void CTerrain::ValidateSurface(){
	if( _dirty & DIRTY_SURFACE ){
		if( _data ){
			float *h=_data;
			_surface->Clear();
			for( int z=0;z<_zsize;++z ){
				for( int x=0;x<_xsize;++x ){
					float y=*h++;
					CVertex v;
					v.position=CVec3( x,y,z ) * _scale + _trans;
					v.texcoords[0]=CVec2( x,z );///float(_xsize-1),z/float(_zsize-1) );
					_surface->AddVertex( v );
				}
			}
			for( int z=0;z<_zsize-1;++z ){
				for( int x=0;x<_xsize-1;++x ){
					int v0=z*_xsize+x;
					_surface->AddTriangle( v0,v0+_xsize+1,v0+1 );
					_surface->AddTriangle( v0,v0+_xsize,v0+_xsize+1 );
				}
			}
			_surface->UpdateNormals();
			_surface->UpdateTangents();
		}
		_dirty&=~DIRTY_SURFACE;
	}
}

REGISTERTYPE( CTerrain )
