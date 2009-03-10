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

#ifndef MODEL_H
#define MODEL_H

#include "scene.h"

class CModelSurface;

class CModel : public CEntity{
public:
	CModel();
	~CModel();
	
	void AddSurface( CModelSurface *surface );
	void RemoveSurface( CModelSurface *surface );
	const vector<CModelSurface*> &Surfaces(){ return _surfaces; }
	
	void Clear();
	void Flip();
	void UpdateNormals();
	void UpdateTangents();
	void Scale( const CVec3 &v );
	void TransformSurfaces( const CMat4 &matrix );
	void ScaleTexCoords( float s_scale,float t_scale );
	void ResetTransform();
	void SplitEdges( float length );
	void Optimize();
	
	virtual void OnRenderWorld();
	
private:
	CModel( CModel *model,CCopier *copier );
	
	CModel *OnCopy( CCopier *copier ){ return new CModel( this,copier ); }

	vector<CModelSurface*> _surfaces;
};

class CVertex{
public:
	CVec3 position;
	CVec3 normal;
	CVec4 tangent;
	CVec2 texcoords[2];
	CVec4 weights;
	int   bones;

	CVertex():weights(1,0,0,0),bones(0){}
	CVertex( const CVec3 &position ):position( position ),weights(1,0,0,0),bones(0){}
	CVertex( float x,float y,float z ):position( CVec3(x,y,z) ),weights(1,0,0,0),bones(0){}

	bool operator<( const CVertex &q )const{ return memcmp( &position,&q.position,sizeof(q) )<0; }

	CVertex Blend( const CVertex &v,float t )const;
};

class CTriangle{
public:
	int vertices[3];

	CTriangle( int v0,int v1,int v2 ){ vertices[0]=v0;vertices[1]=v1;vertices[2]=v2; }
};

class CModelSurface : public CSurface{
public:
	CModelSurface();
	~CModelSurface();
	
	void AddVertex( const CVertex &vertex );
	const vector<CVertex> &Vertices(){ return _vertices; }
	
	void AddTriangle( int v0,int v1,int v2 );
	const vector<CTriangle> &Triangles(){ return _triangles; }
	
	void AddSurface( CModelSurface *surface );

	void Clear();
	void ClearVertices();
	void ClearTriangles();
	void Flip();
	void UpdateNormals();
	void UpdateTangents();
	void ScaleTexCoords( float s_scale,float t_scale );
	void Transform( const CMat4 &matrix,const CMat4 &itMatrix );
	void SplitEdges( float length );
	void Optimize();

	const CBox &Bounds();
	CVertexBuffer *VertexBuffer();
	CIndexBuffer *IndexBuffer();
	
	vector<CMat4> &Instances(){ return _instances; }
	
	void OnRenderInstances( const CHull &bounds );
	void OnClearInstances();

private:
	CModelSurface( CModelSurface *surf,CCopier *copier );
	CModelSurface *OnCopy( CCopier *copier ){ return new CModelSurface( this,copier ); }

	void ValidateBounds();
	void ValidateBuffers();
	void RenderInstances( int first,int count );

	int _dirty;
	vector<CVertex> _vertices;
	vector<CTriangle> _triangles;
	
	CVertexBuffer *_vertexBuffer;
	CIndexBuffer *_indexBuffer;
	CBox _bounds;
	
	vector<CMat4> _instances;
};

#endif
