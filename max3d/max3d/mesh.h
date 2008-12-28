
#ifndef MESH_H
#define MESH_H

#include "surface.h"

class CMesh : public CSurface{
public:
	CMesh();
	~CMesh();

	void SetVertexBuffer( CVertexBuffer *vb );
	CVertexBuffer *VertexBuffer(){ return _vb; }

	void SetIndexBuffer( CIndexBuffer *ib );
	CIndexBuffer *IndexBuffer(){ return _ib; }

	void SetRenderOp( int what,int first,int count );
	void GetRenderOp( int *what,int *first,int *count ){ *what=_what;*first=_first;*count=_count; }
	
protected:

	void RenderInstances( CRenderer *r );

private:

	CVertexBuffer *_vb;
	CIndexBuffer *_ib;
	int _what,_first,_count;
};

#endif
