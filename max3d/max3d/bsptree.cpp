
#include "std.h"

#include "bsptree.h"

CBSPTree::CBSPTree( CModel *model ):
_root(0){

	for( int i=0;i<model->Surfaces().size();++i ){
		CModelSurface *surf=(CModelSurface*)model->Surfaces()[i]->Copy();
		cout<<"Adding surface, verts="<<surf->Vertices().size()<<", tris="<<surf->Triangles().size()<<endl;
		_surfaces.push_back( surf );
	}
	
	vector<CPolygon*> polys;
	
	for( int i=0;i<_surfaces.size();++i ){
		CModelSurface *surf=_surfaces[i];
		
		vector<int> verts(3);

		for( vector<CTriangle>::const_iterator it=surf->Triangles().begin();it!=surf->Triangles().end();++it ){
			const CTriangle &tri=*it;
			verts[0]=tri.vertices[0];
			verts[1]=tri.vertices[1];
			verts[2]=tri.vertices[2];
			CPolygon *poly=new CPolygon( surf,verts );
			polys.push_back( new CPolygon( surf,verts ) );
		}
	}
	
	if( !polys.size() ){
		_root=new CBSPNode( CPlane( CVec3(0,1,0),0 ) );
		return;
	}
	
	_root=new CBSPNode( polys[0]->Plane() );

	for( int i=1;i<polys.size();++i ){
		_root->InsertPlane( polys[i] );
	}

	for( int i=0;i<polys.size();++i ){
		_root->InsertLeaf( polys[i] );
	}

	for( int i=0;i<polys.size();++i ){
//		_root->InsertPolygon( polys[i] );
	}

	CPlane plane( CVec3(0,1,0),-5 );
	
//	_root=new CBSPNode( plane,new CBSPNode( 1 ),0 );
	
	_hpSurface=new CModelSurface;
	CPolygon *poly=new CPolygon( _hpSurface,plane,CBox( CVec3(-50),CVec3(50) ) );
	CBSPNode *bits[2];
	_root->Split( poly,bits );
	_root=new CBSPNode( plane,bits[0],0 );

	for( int i=0;i<polys.size();++i ){
		_root->InsertPolygon( polys[i] );
	}
}

CModel *CBSPTree::BuildModel(){
	for( int i=0;i<_surfaces.size();++i ){
		_surfaces[i]->ClearTriangles();
	}

	_root->BuildModel();

	CModel *model=new CModel;
	for( int i=0;i<_surfaces.size();++i ){
		CModelSurface *surf=(CModelSurface*)_surfaces[i]->Copy();
		cout<<"Building surface, verts="<<surf->Vertices().size()<<", tris="<<surf->Triangles().size()<<endl;
		model->AddSurface( surf );
	}
	return model;
}

CBSPNode::CBSPNode( int kind ):
_kind(kind){
	_kids[0]=_kids[1]=0;
}

CBSPNode::CBSPNode( CPolygon *poly ):
_kind(1){
	_kids[0]=_kids[1]=0;
	_polys.push_back( poly );
}

CBSPNode::CBSPNode( const CPlane &plane,CBSPNode *kid0,CBSPNode *kid1 ):
_kind(0),
_plane(plane){
	_kids[0]=kid0;
	_kids[1]=kid1;
}

void CBSPNode::InsertPlane( CPolygon *poly ){
	if( IsLeaf() ){
		Error( "Can't insert plane into a leaf" );
		return;
	}
	if( _plane==poly->Plane() ) return;
	CPolygon *bits[2];
	poly->Split( _plane,bits );
	for( int i=0;i<2;++i ){
		if( !bits[i] ) continue;
		if( _kids[i] ){
			_kids[i]->InsertPlane( bits[i] );
		}else{
			_kids[i]=new CBSPNode( bits[i]->Plane() );
		}
	}
}

void CBSPNode::InsertLeaf( CPolygon *poly ){
	if( IsLeaf() ){
		return;
	}
	CPolygon *bits[2];
	poly->Split( _plane,bits );
	for( int i=0;i<2;++i ){
		if( !bits[i] ) continue;
		if( _kids[i] ){
			_kids[i]->InsertLeaf( bits[i] );
		}else{
			_kids[i]=new CBSPNode( 1 );
		}
	}
}

void CBSPNode::InsertPolygon( CPolygon *poly ){
	if( IsLeaf() ){
		_polys.push_back( poly );
		return;
	}
	CPolygon *bits[2];
	poly->Split( _plane,bits );
	for( int i=0;i<2;++i ){
		if( !bits[i] ) continue;
		if( _kids[i] ){
			_kids[i]->InsertPolygon( bits[i] );
		}
	}
}

void CBSPNode::EnumNodes( vector<CBSPNode*> &nodes ){
	if( IsLeaf() ){
		return;
	}
	nodes.push_back( this );
	for( int i=0;i<2;++i ){
		if( _kids[i] ) _kids[i]->EnumNodes( nodes );
	}
}

void CBSPNode::EnumLeaves( vector<CBSPNode*> &leaves ){
	if( IsLeaf() ){
		leaves.push_back( this );
		return;
	}
	for( int i=0;i<2;++i ){
		if( _kids[i] ) _kids[i]->EnumLeaves( leaves );
	}
}

void CBSPNode::Split( CPolygon *subhp,CBSPNode *bits[2] ){
	if( IsLeaf() ){
		bits[0]=bits[1]=this;
		return;
	}

	CPolygon *pbits[2];
	subhp->Split( _plane,pbits );

	bits[0]=bits[1]=0;

	if( pbits[0] && pbits[1] ){
		CBSPNode *bits0[2]={0,0};
		CBSPNode *bits1[2]={0,0};
		if( _kids[0] ) _kids[0]->Split( pbits[0],bits0 );
		if( _kids[1] ) _kids[1]->Split( pbits[1],bits1 );
		bits[0]=new CBSPNode( _plane,bits0[0],bits1[0] );
		bits[1]=new CBSPNode( _plane,bits0[1],bits1[1] );
	}else if( pbits[0] && !pbits[1] ){
		if( _kids[0] ) _kids[0]->Split( pbits[0],bits );
		if( subhp->Plane().n.Dot( _plane.n )>0 ){
			bits[1]=new CBSPNode( _plane,bits[1],_kids[1] );
		}else{
			bits[0]=new CBSPNode( _plane,bits[0],_kids[1] );
		}
	}else if( !pbits[0] && pbits[1] ){
		if( _kids[1] ) _kids[1]->Split( pbits[1],bits );
		if( subhp->Plane().n.Dot( _plane.n )>0 ){
			bits[0]=new CBSPNode( _plane,_kids[0],bits[0] );
		}else{
			bits[1]=new CBSPNode( _plane,_kids[0],bits[1] );
		}
	}else{
		if( subhp->Plane().n.Dot( _plane.n )>0 ){
			bits[0]=_kids[0];
			bits[1]=_kids[1];
		}else{
			bits[0]=_kids[1];
			bits[1]=_kids[0];
		}
	}
}

CBSPNode *CBSPNode::Merge( CBSPNode *node ){
}

void CBSPNode::BuildModel(){
	if( IsLeaf() ){
		for( int i=0;i<_polys.size();++i ){
			CPolygon *poly=_polys[i];
			int v0=poly->Vertices()[0];
			int v1=poly->Vertices()[1];
			for( int j=2;j<poly->Vertices().size();++j ){
				int v2=poly->Vertices()[j];
				poly->Surface()->AddTriangle( v0,v1,v2 );
				v1=v2;
			}
		}
		return;
	}
	for( int i=0;i<2;++i ){
		if( _kids[i] ) _kids[i]->BuildModel();
	}
}

CPolygon::CPolygon( CModelSurface *surface,const CPlane &plane ):
_surface(surface),
_plane(plane){
	//Don't forget to add vertices!
}

CPolygon::CPolygon( CModelSurface *surface,const vector<int> &vertices ):
_surface(surface),
_vertices(vertices){
	Assert( vertices.size()>=3 );
	_plane=CPlane::TrianglePlane( Vertex(0).position,Vertex(1).position,Vertex(2).position );
}

CPolygon::CPolygon( CModelSurface *surface,const vector<int> &vertices,const CPlane &plane ):
_surface(surface),
_vertices(vertices),
_plane(plane){
	Assert( vertices.size()>=3 );
}

CPolygon::CPolygon( CModelSurface *surface,const CPlane &plane,const CBox &box ):
_surface(surface),
_plane(plane){
	CVec3 v[4];
	if( fabs(plane.n.x)>fabs(plane.n.y) && fabs(plane.n.x)>fabs(plane.n.z) ){
		if( plane.n.x>0 ){
			v[0]=box.Corner(3);v[1]=box.Corner(7);v[2]=box.Corner(5);v[3]=box.Corner(1);
		}else{
			v[0]=box.Corner(6);v[1]=box.Corner(2);v[2]=box.Corner(0);v[3]=box.Corner(4);
		}
		for( int i=0;i<4;++i ) v[i].x=plane.SolveX( v[i].y,v[i].z );
	}else if( fabs(plane.n.y)>fabs(plane.n.z) ){
		if( plane.n.y>0 ){
			v[0]=box.Corner(6);v[1]=box.Corner(7);v[2]=box.Corner(3);v[3]=box.Corner(2);
		}else{
			v[0]=box.Corner(0);v[1]=box.Corner(1);v[2]=box.Corner(5);v[3]=box.Corner(4);
		}
		for( int i=0;i<4;++i ) v[i].y=plane.SolveY( v[i].x,v[i].z );
	}else{
		if( plane.n.z>0 ){
			v[0]=box.Corner(7);v[1]=box.Corner(6);v[2]=box.Corner(4);v[3]=box.Corner(5);
		}else{
			v[0]=box.Corner(2);v[1]=box.Corner(3);v[2]=box.Corner(1);v[3]=box.Corner(0);
		}
		for( int i=0;i<4;++i ) v[i].z=plane.SolveZ( v[i].x,v[i].y );
	}
	for( int i=0;i<4;++i ){
		CVertex tv=CVertex( v[i] );
		AddVertex( tv );
	}
}

bool CPolygon::Validate(){
	bool valid=true;
	for( int i=0;i<_vertices.size();++i ){
		{
			float d=_plane.Distance( Vertex(i).position );
			if( fabs(d)>POLYGON_PLANAR_EPSILON/2 ){
				cout<<"*** Nonplanar polygon, distance="<<d<<" ***"<<endl;
				valid=false;
			}
		}
		
		{
			int i2=i<_vertices.size()-1 ? i+1 : 0;
			float d=Vertex(i).position.Distance( Vertex(i2).position );
			if( d<POLYGON_PLANAR_EPSILON ){
				cout<<"*** Degenerate edge, length="<<d<<" ***"<<endl;
				valid=false;
			}
		}
	}
	return valid;
}

void CPolygon::AddVertex( const CVertex &v ){
	int index=_surface->Vertices().size();
	_surface->AddVertex( v );
	_vertices.push_back( index );
}

int CPolygon::Classify( const CPlane &plane ){
	int nfront=0,nback=0,nplanar=0;

	for( int i=0;i<_vertices.size();++i ){
		
		float d=plane.Distance( Vertex( i ).position );

		if( d>POLYGON_PLANAR_EPSILON ){
			++nfront;
		}else if( d<-POLYGON_PLANAR_EPSILON ){
			++nback;
		}else{
			++nplanar;
		}
	}
	return (nfront ? 1 : 0) | (nback ? 2 : 0);
}

int CPolygon::Split( const CPlane &plane,CPolygon *bits[2] ){
	int nfront=0,nback=0,nplanar=0;

	for( int i=0;i<_vertices.size();++i ){
		
		float d=plane.Distance( Vertex( i ).position );

		if( d>POLYGON_PLANAR_EPSILON ){
			++nfront;
		}else if( d<-POLYGON_PLANAR_EPSILON ){
			++nback;
		}else{
			++nplanar;
		}
	}
	
	bits[0]=bits[1]=0;

	if( !nfront && !nback ){
		if( plane.n.Dot( _plane.n )>0 ){
			bits[0]=this;
			return 1;
		}
		bits[1]=this;
		return 2;
	}
	
	if( !nback ){
		bits[0]=this;
		return 1;
	}

	if( !nfront ){
		bits[1]=this;
		return 2;
	}
	
	nfront+=2;
	nback+=2;
	
	vector<int> front,back;
	front.reserve( nfront );
	back.reserve( nback );
	
	float d=plane.Distance( Vertex( _vertices.size()-1 ).position );
	int f=d>POLYGON_PLANAR_EPSILON ? 1 : (d<-POLYGON_PLANAR_EPSILON ? 2 : 0);
	
	for( int i=0;i<_vertices.size();++i ){
		float pd=d;
		int pf=f;
		
		d=plane.Distance( Vertex( i ).position );
		f=d>POLYGON_PLANAR_EPSILON ? 1 : (d<-POLYGON_PLANAR_EPSILON ? 2 : 0);
		
		if( f && pf && f!=pf ){
			float t=pd/(pd-d);
			const CVertex &p=Vertex(i ? i-1 : _vertices.size()-1 );
			const CVertex &c=Vertex(i);
			CVertex sp=p.Blend( c,t );
			int v=_surface->Vertices().size();
			_surface->AddVertex( sp );
			front.push_back( v );
			back.push_back( v );
		}
		int v=_vertices[i];
		if( f==1 ){
			front.push_back( v );
		}else if( f==2 ){
			back.push_back( v );
		}else{
			front.push_back( v );
			back.push_back( v );
		}
	}
	
	if( front.size()!=nfront ){
		Warning( "Front polygon error" );
		return 0;
	}

	if( back.size()!=nback ){
		Warning( "Back polygon error" );
		return 0;
	}
	
	bits[0]=new CPolygon( _surface,front,_plane );
	bits[1]=new CPolygon( _surface,back,_plane );

	return 3;
}
