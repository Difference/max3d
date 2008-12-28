
#include "std.h"

#include "app.h"
#include "meshutil.h"

CMesh *CMeshUtil::CreateMesh( CMaterial *material,CModel *model ){
	CVertexBuffer *vb=App.Graphics()->CreateVertexBuffer( model->Vertices().size(),"3f3f4f2f" );

	float *vp=(float*)vb->Lock();
	for( vector<CVertex>::const_iterator it=model->Vertices().begin();it!=model->Vertices().end();++it ){
		const CVertex &v=*it;
		memcpy( vp,&v.position,12 );
		memcpy( vp+3,&v.normal,12 );
		memcpy( vp+6,&v.tangent,16 );
		memcpy( vp+10,&v.texcoords[0],8 );
		vp+=12;
	}
	vb->Unlock();

	CIndexBuffer *ib=App.Graphics()->CreateIndexBuffer( model->Triangles().size()*3,"1i" );
	int *ip=(int*)ib->Lock();
	for( vector<CTriangle>::const_iterator it=model->Triangles().begin();it!=model->Triangles().end();++it ){
		const CTriangle &t=*it;
		memcpy( ip,&t,12 );
		ip+=3;
	}
	ib->Unlock();

	CMesh *mesh=new CMesh;
	mesh->SetMaterial( material );
	mesh->SetVertexBuffer( vb );
	mesh->SetIndexBuffer( ib );
	mesh->SetRenderOp( 3,0,model->Triangles().size()*3 );

	vb->Release();
	ib->Release();

	return mesh;
}

CMesh *CMeshUtil::CreateSphereMesh( CMaterial *material,float radius ){
	int segs=8;
	CModel *model=new CModel;
	int segs2=segs*2;
	for( int j=0;j<segs2;++j ){
		CVertex v( 0,radius,0 );
		v.normal=CVec3( 0,1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),0 );
		model->AddVertex( v );
	}
	for( int i=1;i<segs;++i ){
		float r=sinf( i*PI/segs )*radius;
		float y=cosf( i*PI/segs );
		CVertex v( 0,y*radius,-r );
		v.normal=CVec3( 0,y,-1 ).Normalize();
		v.texcoords[0]=CVec2( 0,i/float(segs) );
		model->AddVertex( v );
		for( int j=1;j<segs2;++j ){
			float x=sinf( j*TWOPI/segs2 );
			float z=-cosf( j*TWOPI/segs2 );
			CVertex v( x*r,y*radius,z*r );
			v.normal=CVec3( x,y,z ).Normalize();
			v.texcoords[0]=CVec2( j/float(segs),i/float(segs) );
			model->AddVertex( v );
		}
		v.texcoords[0].x=2;
		model->AddVertex( v );
	}
	for( int j=0;j<segs2;++j ){
		CVertex v( 0,-radius,0 );
		v.normal=CVec3( 0,-1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),1 );
		model->AddVertex( v );
	}
	int v=0;
	for( int j=0;j<segs2;++j ){
		model->AddTriangle( v,v+segs2+1,v+segs2 );
		++v;
	}
	for( int i=1;i<segs-1;++i ){
		for( int j=0;j<segs2;++j ){
			model->AddTriangle( v,v+1,v+segs2+2 );
			model->AddTriangle( v,v+segs2+2,v+segs2+1 );
			++v;
		}
		++v;
	}
	for( int j=0;j<segs2;++j ){
		model->AddTriangle( v,v+1,v+segs2+1 );
		++v;
	}
	model->UpdateTangents();
	CMesh *mesh=App.MeshUtil()->CreateMesh( material,model );
	model->Release();
	return mesh;
}

CMesh *CMeshUtil::CreateCapsuleMesh( CMaterial *material,float radius,float length ){
	int segs=8;
	CModel *model=new CModel;
	segs=(segs+1)&~1;
	int segs2=segs*2;
	float hlength=length/2;
	for( int j=0;j<segs2;++j ){
		float ty=hlength+radius;
		CVertex v( 0,ty,0 );
		v.normal=CVec3( 0,1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),0 );
		model->AddVertex( v );
	}
	for( int i=1;i<segs;++i ){
		float r=sinf( i*PI/segs )*radius;
		float y=cosf( i*PI/segs );
		float ty=y*radius;
		if( i<segs/2 ) ty+=hlength; else ty-=hlength;
		CVertex v( 0,ty,-r );
		v.normal=CVec3( 0,y,-1 ).Normalize();
		v.texcoords[0]=CVec2( 0,i/float(segs) );
		model->AddVertex( v );
		for( int j=1;j<segs2;++j ){
			float x=sinf( j*TWOPI/segs2 );
			float z=-cosf( j*TWOPI/segs2 );
			float ty=y*radius;
			if( i<segs/2 ) ty+=hlength; else ty-=hlength;
			CVertex v( x*r,ty,z*r );
			v.normal=CVec3( x,y,z ).Normalize();
			v.texcoords[0]=CVec2( j/float(segs),i/float(segs) );
			model->AddVertex( v );
		}
		v.texcoords[0].x=2;
		model->AddVertex( v );
	}
	for( int j=0;j<segs2;++j ){
		float ty=-hlength-radius;
		CVertex v( 0,ty,0 );
		v.normal=CVec3( 0,-1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),1 );
		model->AddVertex( v );
	}
	int v=0;
	for( int j=0;j<segs2;++j ){
		model->AddTriangle( v,v+segs2+1,v+segs2 );
		++v;
	}
	for( int i=1;i<segs-1;++i ){
		for( int j=0;j<segs2;++j ){
			model->AddTriangle( v,v+1,v+segs2+2 );
			model->AddTriangle( v,v+segs2+2,v+segs2+1 );
			++v;
		}
		++v;
	}
	for( int j=0;j<segs2;++j ){
		model->AddTriangle( v,v+1,v+segs2+1 );
		++v;
	}
	model->UpdateTangents();
	CMesh *mesh=App.MeshUtil()->CreateMesh( material,model );
	model->Release();
	return mesh;
}

CMesh *CMeshUtil::CreateCylinderMesh( CMaterial *material,float radius,float length ){
	int segs=8;
	CModel *model=new CModel;
	float hlength=length/2;
	CVertex v;
	v.position=CVec3( 0,hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 0,0 );
	model->AddVertex( v );
	v.position=CVec3( 0,-hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 0,1 );
	model->AddVertex( v );
	for( int i=1;i<segs;++i ){
		float x=sinf( i*TWOPI/segs );
		float z=-cosf( i*TWOPI/segs );
		CVertex v;
		v.position=CVec3( x*radius,hlength,z*radius );
		v.normal=CVec3( x,0,z );
		v.texcoords[0]=CVec2( i/float(segs),0 );
		model->AddVertex( v );
		v.position=CVec3( x*radius,-hlength,z*radius );
		v.normal=CVec3( x,0,z );
		v.texcoords[0]=CVec2( i/float(segs),1 );
		model->AddVertex( v );
	}
	v.position=CVec3( 0,hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 1,0 );
	model->AddVertex( v );
	v.position=CVec3( 0,-hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 1,1 );
	model->AddVertex( v );
	for( int i=0;i<segs;++i ){
		model->AddTriangle( i*2,i*2+2,i*2+1 );
		model->AddTriangle( i*2+2,i*2+3,i*2+1 );
	}
	model->UpdateTangents();
	CMesh *mesh=App.MeshUtil()->CreateMesh( material,model );
	model->Release();
	return mesh;
}

CMesh *CMeshUtil::CreateBoxMesh( CMaterial *material,float width,float height,float depth ){
	CModel *model=new CModel;
	static const float faces[]={
		0,0,-1,
		-1,+1,-1,0,0,
		+1,+1,-1,1,0,
		+1,-1,-1,1,1,
		-1,-1,-1,0,1,
		1,0,0,
		+1,+1,-1,0,0,
		+1,+1,+1,1,0,
		+1,-1,+1,1,1,
		+1,-1,-1,0,1,
		0,0,1,
		+1,+1,+1,0,0,
		-1,+1,+1,1,0,
		-1,-1,+1,1,1,
		+1,-1,+1,0,1,
		-1,0,0,
		-1,+1,+1,0,0,
		-1,+1,-1,1,0,
		-1,-1,-1,1,1,
		-1,-1,+1,0,1,
		0,1,0,
		-1,+1,+1,0,0,
		+1,+1,+1,1,0,
		+1,+1,-1,1,1,
		-1,+1,-1,0,1,
		0,-1,0,
		-1,-1,-1,0,0,
		+1,-1,-1,1,0,
		+1,-1,+1,1,1,
		-1,-1,+1,0,1
	};
	float hwidth=width/2,hheight=height/2,hdepth=depth/2;
	int n=0,t=0;
	for( int i=0;i<6;++i ){
		CVec3 normal=CVec3( faces[t+0],faces[t+1],faces[t+2] );
		t+=3;
		for( int j=0;j<4;++j ){
			CVertex v( faces[t+0]*hwidth,faces[t+1]*hheight,faces[t+2]*hdepth );
			v.normal=normal;
			v.texcoords[0]=CVec2( faces[t+3],faces[t+4] );
			model->AddVertex( v );
			t+=5;
		}
		model->AddTriangle( n,n+1,n+2 );
		model->AddTriangle( n,n+2,n+3 );
		n+=4;
	}
	model->UpdateTangents();
	CMesh *mesh=App.MeshUtil()->CreateMesh( material,model );
	model->Release();
	return mesh;
}
