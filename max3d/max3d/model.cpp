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
#include "model.h"

enum{
	DIRTY_BOUNDS=1,
	DIRTY_BUFFERS=2
};

CModel::CModel(){
}

CModel::CModel( CModel *model,CCopier *copier ):CEntity( model,copier ),
_surfaces( model->_surfaces ){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->Retain();
	}
}

CModel::~CModel(){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->Release();
	}
}

void CModel::AddSurface( CModelSurface *surface ){
	surface->Retain();
	_surfaces.push_back( surface );
}

void CModel::Clear(){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->Release();
	}
	_surfaces.clear();
}

void CModel::Flip(){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->Flip();
	}
}

void CModel::UpdateNormals(){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->UpdateNormals();
	}
}

void CModel::UpdateTangents(){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->UpdateTangents();
	}
}

void CModel::ScaleTexCoords( float s_scale,float t_scale ){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->ScaleTexCoords( s_scale,t_scale );
	}
}

void CModel::ResetTransform(){
	CMat4 matrix=Matrix();
	CMat4 itMatrix=(-matrix).Transpose();
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->Transform( matrix,itMatrix );
	}
	SetMatrix( CMat4() );
}

void CModel::Optimize(){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		(*it)->Optimize();
	}
}

void CModel::OnRenderWorld(){
	for( vector<CModelSurface*>::iterator it=_surfaces.begin();it!=_surfaces.end();++it ){
		CModelSurface *surface=*it;
		if( !surface->Instances().size() ) App.Scene()->AddSurface( surface );
		surface->Instances().push_back( RenderMatrix() );
	}
}

CModelSurface::CModelSurface():
_vertexBuffer(0),
_indexBuffer(0),
_dirty(~0){
	SetShader( App.ShaderUtil()->ModelShader() );
}

CModelSurface::~CModelSurface(){
	if( _vertexBuffer ) _vertexBuffer->Release();
	if( _indexBuffer ) _indexBuffer->Release();
}

void CModelSurface::AddVertex( const CVertex &vertex ){
	_vertices.push_back( vertex );
}

void CModelSurface::AddTriangle( int v0,int v1,int v2 ){
	_triangles.push_back( CTriangle( v0,v1,v2 ) );
	_dirty|=DIRTY_BOUNDS|DIRTY_BUFFERS;
}

void CModelSurface::AddSurface( CModelSurface *surface ){
	int nv=_vertices.size(),nt=_triangles.size();
	_vertices.insert( _vertices.end(),surface->Vertices().begin(),surface->Vertices().end() );
	_triangles.insert( _triangles.end(),surface->Triangles().begin(),surface->Triangles().end() );
	for( int i=nt;i<_triangles.size();++i ){
		CTriangle &t=_triangles[i];
		t.vertices[0]+=nv;
		t.vertices[1]+=nv;
		t.vertices[2]+=nv;
	}
	_dirty|=DIRTY_BOUNDS|DIRTY_BUFFERS;
}

void CModelSurface::Clear(){
	_vertices.clear();
	_triangles.clear();
	_dirty|=DIRTY_BOUNDS|DIRTY_BUFFERS;
}

void CModelSurface::Flip(){
	for( vector<CVertex>::iterator it=_vertices.begin();it!=_vertices.end();++it ){
		CVertex &v=*it;
		v.normal=-v.normal;
		v.tangent.xyz()=-v.tangent.xyz();
	}
	for( vector<CTriangle>::iterator it=_triangles.begin();it!=_triangles.end();++it ){
		CTriangle &t=*it;
		std::swap( t.vertices[1],t.vertices[2] );
	}
}

void CModelSurface::UpdateNormals(){
	vector<CVec3> norms( _vertices.size() );
	for( vector<CTriangle>::iterator it=_triangles.begin();it!=_triangles.end();++it ){
		const CTriangle &tri=*it;
		int i0=tri.vertices[0];
		int i1=tri.vertices[1];
		int i2=tri.vertices[2];
		const CVec3 &v0=_vertices[i0].position;
		const CVec3 &v1=_vertices[i1].position;
		const CVec3 &v2=_vertices[i2].position;
		CVec3 normal=CPlane( v0,v1,v2 ).n;
		norms[i0]+=normal;
		norms[i1]+=normal;
		norms[i2]+=normal;
	}
	for( int i=0;i<_vertices.size();++i ){
		_vertices[i].normal=norms[i].Normalize();
	}
}

void CModelSurface::UpdateTangents(){
	vector<CVec3> stangs( _vertices.size() );
	vector<CVec3> ttangs( _vertices.size() );
	for( vector<CTriangle>::iterator it=_triangles.begin();it!=_triangles.end();++it ){
		const CTriangle &tri=*it;
		int i0=tri.vertices[0];
		int i1=tri.vertices[1];
		int i2=tri.vertices[2];
		const CVec3 &v0=_vertices[i0].position;
		const CVec3 &v1=_vertices[i1].position;
		const CVec3 &v2=_vertices[i2].position;
		const CVec2 &c0=_vertices[i0].texcoords[0];
		const CVec2 &c1=_vertices[i1].texcoords[0];
		const CVec2 &c2=_vertices[i2].texcoords[0];
		CVec3 side0=v0-v1,side1=v2-v1;
		float deltas0=c0.x-c1.x,deltas1=c2.x-c1.x;
		float deltat0=c0.y-c1.y,deltat1=c2.y-c1.y;
		CVec3 stang=( (side0 * deltat1)-(side1 * deltat0) ).Normalize();
		CVec3 ttang=( (side0 * deltas1)-(side1 * deltas0) ).Normalize();
		stangs[i0]+=stang;
		ttangs[i0]+=ttang;
		stangs[i1]+=stang;
		ttangs[i1]+=ttang;
		stangs[i2]+=stang;
		ttangs[i2]+=ttang;
	}
	for( int i=0;i<_vertices.size();++i ){
		const CVec3 &n=_vertices[i].normal;
		const CVec3 &t=stangs[i];
		CVec3 tang=( t-n*n.Dot(t) ).Normalize();
		float w=n.Cross(t).Dot( ttangs[i] )>0 ? 1 : -1;
		_vertices[i].tangent=CVec4( tang,w );
	}
}

void CModelSurface::ScaleTexCoords( float s_scale,float t_scale ){
	for( vector<CVertex>::iterator it=_vertices.begin();it!=_vertices.end();++it ){
		CVertex &v=*it;
		v.texcoords[0].x*=s_scale;
		v.texcoords[0].y*=t_scale;
	}
}

void CModelSurface::Transform( const CMat4 &matrix,const CMat4 &itMatrix ){
	for( vector<CVertex>::iterator it=_vertices.begin();it!=_vertices.end();++it ){
		CVertex &v=*it;
		v.position=matrix * v.position;
		v.normal=(itMatrix * CVec4( v.normal,0.0f ) ).xyz();
		v.tangent=CVec4( (itMatrix * CVec4( v.tangent.xyz(),0.0f ) ).xyz(),v.tangent.w );
	}
}

void CModelSurface::Optimize(){
	if( _vertexBuffer && _vertexBuffer->Capacity()>_vertices.size() ){
		_vertexBuffer->Release();
		_vertexBuffer=0;
	}
	if( _indexBuffer && _indexBuffer->Capacity()>_triangles.size()*3 ){
		_indexBuffer->Release();
		_indexBuffer=0;
	}
	ValidateBounds();
	ValidateBuffers();
	_vertices.clear();
	_triangles.clear();
}

void CModelSurface::ValidateBounds(){
	if( _dirty & DIRTY_BOUNDS ){
		_bounds=CBox::Empty();
		for( int i=0;i<_triangles.size();++i ){
			for( int j=0;j<3;++j ){
				_bounds.Update( _vertices[ _triangles[i].vertices[j] ].position );
			}
		}
		_dirty&=~DIRTY_BOUNDS;
	}
}

void CModelSurface::ValidateBuffers(){
	if( _dirty & DIRTY_BUFFERS ){
		if( !_vertexBuffer || _vertexBuffer->Capacity()<_vertices.size() ){
			if( _vertexBuffer ) _vertexBuffer->Release();
			_vertexBuffer=App.Graphics()->CreateVertexBuffer( _vertices.size(),"3f3f4f2f" );
		}
		float *vp=(float*)_vertexBuffer->Lock();
		for( vector<CVertex>::const_iterator it=_vertices.begin();it!=_vertices.end();++it ){
			const CVertex &v=*it;
			memcpy( vp,&v.position,12 );
			memcpy( vp+3,&v.normal,12 );
			memcpy( vp+6,&v.tangent,16 );
			memcpy( vp+10,&v.texcoords[0],8 );
			vp+=12;
		}
		_vertexBuffer->Unlock();
		
		if( !_indexBuffer || _indexBuffer->Capacity()<_triangles.size()*3 ){
			if( _indexBuffer ) _indexBuffer->Release();
			_indexBuffer=App.Graphics()->CreateIndexBuffer( _triangles.size()*3,"1i" );
		}
		int *ip=(int*)_indexBuffer->Lock();
		for( vector<CTriangle>::const_iterator it=_triangles.begin();it!=_triangles.end();++it ){
			const CTriangle &t=*it;
			memcpy( ip,&t,12 );
			ip+=3;
		}
		_indexBuffer->Unlock();
		
		_dirty&=~DIRTY_BUFFERS;
	}
}

const CBox &CModelSurface::Bounds(){
	ValidateBounds();
	return _bounds;
}

CVertexBuffer *CModelSurface::VertexBuffer(){
	ValidateBuffers();
	return _vertexBuffer;
}

CIndexBuffer *CModelSurface::IndexBuffer(){
	ValidateBuffers();
	return _indexBuffer;
}

void CModelSurface::RenderInstances( int first,int count ){

	const int MAXINSTS=48;
	CParam *bb_mm=CParam::ForName( "bb_ModelMatrices" );
	
	for( int i=0;i<count / MAXINSTS;++i ){
		bb_mm->SetFloatValue( MAXINSTS*16,&_instances[first+i*MAXINSTS].i.x );
		App.Graphics()->Render( 3,0,_triangles.size()*3,MAXINSTS );
	}

	if( int r=count % MAXINSTS ){
		bb_mm->SetFloatValue( r*16,&_instances[first+count-r].i.x );
		App.Graphics()->Render( 3,0,_triangles.size()*3,r );
	}
}

void CModelSurface::OnRenderInstances( const CHull &bounds ){
	if( !_instances.size() ) return;
	
	App.Graphics()->SetVertexBuffer( VertexBuffer() );
	App.Graphics()->SetIndexBuffer( IndexBuffer() );
	
	const int MAXINSTS=48;
	CParam *bb_mm=CParam::ForName( "bb_ModelMatrices" );
	
	if( bounds.Empty() ){
		RenderInstances( 0,_instances.size() );
	}else{
		int n=0;
		for( int i=0;i<_instances.size();++i ){
			if( bounds.Intersects( _instances[i] * Bounds() ) ){
				++n;
			}else if( n ){
				RenderInstances( i-n,n );
				n=0;
			}
		}
		if( n ){
			RenderInstances( _instances.size()-n,n );
		}
	}
}

void CModelSurface::OnClearInstances(){
	_instances.clear();
}

REGISTERTYPE( CModel )
