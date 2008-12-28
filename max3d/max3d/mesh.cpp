
#include "std.h"

#include "app.h"
#include "mesh.h"
#include "renderer.h"

CMesh::CMesh():_vb(0),_ib(0),_what(0),_first(0),_count(0){
}

CMesh::~CMesh(){
	if( _vb ) _vb->Release();
	if( _ib ) _ib->Release();
}

void CMesh::SetVertexBuffer( CVertexBuffer *vb ){
	if( vb ) vb->Retain();
	if( _vb ) _vb->Release();
	_vb=vb;
}

void CMesh::SetIndexBuffer( CIndexBuffer *ib ){
	if( ib ) ib->Retain();
	if( _ib ) _ib->Release();
	_ib=ib;
}

void CMesh::SetRenderOp( int what,int first,int count ){
	_what=what;
	_first=first;
	_count=count;
}

void CMesh::RenderInstances( CRenderer *r ){
	App.Graphics()->SetVertexBuffer( _vb );
	App.Graphics()->SetIndexBuffer( _ib );
	
	const int INSTS=48;

	int n=0;
	float buf[INSTS*16],*p=buf;
	for( vector<CEntity*>::iterator it=Instances().begin();it!=Instances().end();++it ){
		CEntity *entity=*it;
		CMat4 mat=entity->RenderMatrix();
		memcpy( p,&mat,64 );
		p+=16;
		if( p==buf+INSTS*16 ){
//			if( INSTS==1 ) App.Graphics()->SetMat4Param( "bb_ModelMatrix",entity->RenderMatrix() );
			CParam::ForName( "bb_ModelMatrices" )->SetFloatValue( INSTS*16,buf );
			App.Graphics()->Render( _what,_first,_count,INSTS );
			p=buf;
		}
	}
	if( p!=buf ){
		int n=(p-buf)/16;
		CParam::ForName( "bb_ModelMatrices" )->SetFloatValue( n*16,buf );
		App.Graphics()->Render( _what,_first,_count,n );
		p=buf;
	}
}
