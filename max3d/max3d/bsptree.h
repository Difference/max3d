
#ifndef BSPTREE_H
#define BSPTREE_H

#include "model.h"

class CBSPTree;
class CBSPNode;
class CPolygon;

class CBSPTree : public CResource{
public:
	CBSPTree( CModel *model );
	
	CBSPNode *Root(){ return _root; }
	
	CBSPTree *Merge( CBSPTree *tree );
	
	void Union( CBSPTree *tree );
	void Subtract( CBSPTree *tree );
	void Intersect( CBSPTree *tree );
	
	CModel *BuildModel();
	
private:
	CBSPNode *_root;
	vector<CModelSurface*> _surfaces;
	CModelSurface *_hpSurface;
};

class CBSPNode{
public:
	CBSPNode( int kind );	//create leaf poly
	CBSPNode( CPolygon *poly );
	CBSPNode( const CPlane &plane,CBSPNode *kid0=0,CBSPNode *kid1=0 );
	
	void InsertPlane( CPolygon *poly );
	void InsertLeaf( CPolygon *poly );
	void InsertPolygon( CPolygon *poly );

	void EnumNodes( vector<CBSPNode*> &nodes );
	void EnumLeaves( vector<CBSPNode*> &leaves );
	
	bool IsLeaf(){ return !!_kind; }
	
	const vector<CPolygon*> &Polygons(){ return _polys; }
	
	CPolygon *SubHp();
	void Split( CPolygon *subhp,CBSPNode *bits[2] );
	CBSPNode *Merge( CBSPNode *node );
	
	void BuildModel();
	
private:
	int _kind;
	CPlane _plane;
	CBSPNode *_kids[2];
	vector<CPolygon*> _polys;
};

const float POLYGON_PLANAR_EPSILON=.001f;

class CPolygon{
public:
	CPolygon( CModelSurface *surface,const CPlane &plane );
	CPolygon( CModelSurface *surface,const vector<int> &vertices );
	CPolygon( CModelSurface *surface,const vector<int> &vertices,const CPlane &plane );
	CPolygon( CModelSurface *surface,const CPlane &plane,const CBox &box );

	bool Validate();

	void AddVertex( const CVertex &v );
	
	CModelSurface *Surface(){ return _surface; }
	const vector<int> &Vertices(){ return _vertices; }
	const CPlane &Plane(){ return _plane; }

	const CVertex &Vertex( int index ){ return _surface->Vertices()[ _vertices[index] ]; }

	//bit 0=vertices in front of plane
	//bit 1=vertices behind plane
	int Classify( const CPlane &plane );

	int Split( const CPlane &plane,CPolygon *bits[2] );
	
private:
	vector<int> _vertices;
	CModelSurface *_surface;
	CPlane _plane;
};

#endif
