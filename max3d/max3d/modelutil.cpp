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
#include "modelutil.h"

#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>

static CVec3 ccolor( const aiColor4D &color ){
	return CVec3( color.r,color.g,color.b );
}

static string cstring( const aiString &str ){
	return string( str.data,str.length );
}

static CMaterial *cmaterial( const aiMaterial *mat ){
	CMaterial *cmat=new CMaterial;
	
	aiColor4D color;
	aiString path;

	if( aiGetMaterialTexture( mat,aiTextureType_DIFFUSE,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "DiffuseMap",(CTexture*)App.ImportObject( "CTexture",cstring( path ) ) );
	}else if( aiGetMaterialColor( mat,AI_MATKEY_COLOR_DIFFUSE,&color )==AI_SUCCESS ){
		cmat->SetColor( "DiffuseColor",ccolor( color ) );
	}

	if( aiGetMaterialTexture( mat,aiTextureType_SPECULAR,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "SpecularMap",(CTexture*)App.ImportObject( "CTexture",cstring( path ) ) );
	}else if( aiGetMaterialColor( mat,AI_MATKEY_COLOR_SPECULAR,&color )==AI_SUCCESS ){
		cmat->SetColor( "SpecularColor",ccolor( color ) );
	}
	
	if( aiGetMaterialTexture( mat,aiTextureType_EMISSIVE,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "EmissiveMap",(CTexture*)App.ImportObject( "CTexture",cstring( path ) ) );
	}else if( aiGetMaterialColor( mat,AI_MATKEY_COLOR_EMISSIVE,&color )==AI_SUCCESS ){
		cmat->SetColor( "EmissiveColor",ccolor( color ) );
	}
	
	if( aiGetMaterialTexture( mat,aiTextureType_NORMALS,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "NormalMap",(CTexture*)App.ImportObject( "CTexture",cstring( path ) ) );
	}

	return cmat;
}

static CModelSurface *csurface( const aiMesh *mesh ){
	CModelSurface *surf=new CModelSurface;

	const CVec3 scale( .035,.035,.035 );

	aiVector3D *mv=mesh->mVertices;
	aiVector3D *mn=mesh->mNormals;
	aiVector3D *mt=mesh->mTangents;
	aiVector3D *mb=mesh->mBitangents;
	aiVector3D *mc=mesh->mTextureCoords[0];

	for( int i=0;i<mesh->mNumVertices;++i ){
		float tw=1.0;
		if( mn && mt && mb ){
			CVec3 norm=*(CVec3*)mn;
			CVec3 tang=*(CVec3*)mt;
			CVec3 bitang=*(CVec3*)mb;
			if( norm.Dot( tang.Cross(bitang) )<0.0f ) tw=-1.0f;
		}
		
		CVertex v;
		v.position=scale * *(CVec3*)mv++;
		if( mn ) v.normal=*(CVec3*)mn++;
		if( mt ) v.tangent=CVec4( *(CVec3*)mt++,tw );
		if( mc ) v.texcoords[0]=*(CVec2*)mc++;
		surf->AddVertex( v );
	}

	aiFace *face=mesh->mFaces;
	for( int i=0;i<mesh->mNumFaces;++i ){
		surf->AddTriangle( face->mIndices[0],face->mIndices[1],face->mIndices[2] );
		++face;
	}
	
	surf->UpdateNormals();
	surf->UpdateTangents();

	return surf;
}

CBody *CModelUtil::CreateModelBody( CModel *model,int collType,float mass ){
	CModelSurface *physSurf=new CModelSurface;
	for( vector<CModelSurface*>::const_iterator it=model->Surfaces().begin();it!=model->Surfaces().end();++it ){
		physSurf->AddSurface( *it );
	}
	CBody *body=App.World()->Physics()->CreateSurfaceBody( physSurf,collType,mass );
	physSurf->Release();
	return body;
}

CModel *CModelUtil::ImportModel( const string &path,int collType,float mass ){
	Assimp::Importer importer;

	int flags=
	aiProcess_GenNormals |
	aiProcess_GenSmoothNormals |
	aiProcess_Triangulate |
	aiProcess_CalcTangentSpace |
	aiProcess_JoinIdenticalVertices |
	aiProcess_ImproveCacheLocality |
	aiProcess_RemoveRedundantMaterials |
	aiProcess_PreTransformVertices |
	0;

	int i=path.find_last_of( '.' );
	if( i!=string::npos ){
		string ext=path.substr( i+1 );
		if( ext=="x" || ext=="X" ){
			flags|=aiProcess_ConvertToLeftHanded;
		}
	}
   
	const aiScene *scene=importer.ReadFile( path,flags );

	string err=importer.GetErrorString();
	if( err.size() ) Warning( "aiImporter error:"+err );
	if( !scene ) return 0;

	vector<CMaterial*> mats;
	
	for( int i=0;i<scene->mNumMaterials;++i ){
		aiMaterial *mat=scene->mMaterials[i];
		CMaterial *cmat=cmaterial( mat );
		mats.push_back( cmat );
	}
	
	CModel *model=new CModel;
	
	for( int i=0;i<scene->mNumMeshes;++i ){
		aiMesh *mesh=scene->mMeshes[i];

		CModelSurface *surf=csurface( mesh );
	
		surf->SetMaterial( mats[mesh->mMaterialIndex] );
	
		model->AddSurface( surf );

		surf->Release();
	}

	for( int i=0;i<mats.size();++i ){
		mats[i]->Release();
	}
	
	if( collType || mass ) model->SetBody( CreateModelBody( model,collType,mass ) );

	return model;
}

CModel *CModelUtil::CreateSphere( CMaterial *material,float radius,int collType,float mass ){
	int segs=8;
	CModelSurface *surf=new CModelSurface;
	surf->SetMaterial( material );
	int segs2=segs*2;
	for( int j=0;j<segs2;++j ){
		CVertex v( 0,radius,0 );
		v.normal=CVec3( 0,1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),0 );
		surf->AddVertex( v );
	}
	for( int i=1;i<segs;++i ){
		float r=sinf( i*PI/segs )*radius;
		float y=cosf( i*PI/segs );
		CVertex v( 0,y*radius,-r );
		v.normal=CVec3( 0,y,-1 ).Normalize();
		v.texcoords[0]=CVec2( 0,i/float(segs) );
		surf->AddVertex( v );
		for( int j=1;j<segs2;++j ){
			float x=sinf( j*TWOPI/segs2 );
			float z=-cosf( j*TWOPI/segs2 );
			CVertex v( x*r,y*radius,z*r );
			v.normal=CVec3( x,y,z ).Normalize();
			v.texcoords[0]=CVec2( j/float(segs),i/float(segs) );
			surf->AddVertex( v );
		}
		v.texcoords[0].x=2;
		surf->AddVertex( v );
	}
	for( int j=0;j<segs2;++j ){
		CVertex v( 0,-radius,0 );
		v.normal=CVec3( 0,-1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),1 );
		surf->AddVertex( v );
	}
	int v=0;
	for( int j=0;j<segs2;++j ){
		surf->AddTriangle( v,v+segs2+1,v+segs2 );
		++v;
	}
	for( int i=1;i<segs-1;++i ){
		for( int j=0;j<segs2;++j ){
			surf->AddTriangle( v,v+1,v+segs2+2 );
			surf->AddTriangle( v,v+segs2+2,v+segs2+1 );
			++v;
		}
		++v;
	}
	for( int j=0;j<segs2;++j ){
		surf->AddTriangle( v,v+1,v+segs2+1 );
		++v;
	}
	surf->UpdateTangents();
	CModel *model=new CModel;
	model->AddSurface( surf );
	surf->Release();
	if( collType || mass ) model->SetBody( App.World()->Physics()->CreateSphereBody( radius,collType,mass ) );
	return model;
}

CModel *CModelUtil::CreateCapsule( CMaterial *material,float radius,float length,int collType,float mass ){
	int segs=8;
	CModelSurface *surf=new CModelSurface;
	surf->SetMaterial( material );
	segs=(segs+1)&~1;
	int segs2=segs*2;
	float hlength=length/2;
	for( int j=0;j<segs2;++j ){
		float ty=hlength+radius;
		CVertex v( 0,ty,0 );
		v.normal=CVec3( 0,1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),0 );
		surf->AddVertex( v );
	}
	for( int i=1;i<segs;++i ){
		float r=sinf( i*PI/segs )*radius;
		float y=cosf( i*PI/segs );
		float ty=y*radius;
		if( i<segs/2 ) ty+=hlength; else ty-=hlength;
		CVertex v( 0,ty,-r );
		v.normal=CVec3( 0,y,-1 ).Normalize();
		v.texcoords[0]=CVec2( 0,i/float(segs) );
		surf->AddVertex( v );
		for( int j=1;j<segs2;++j ){
			float x=sinf( j*TWOPI/segs2 );
			float z=-cosf( j*TWOPI/segs2 );
			float ty=y*radius;
			if( i<segs/2 ) ty+=hlength; else ty-=hlength;
			CVertex v( x*r,ty,z*r );
			v.normal=CVec3( x,y,z ).Normalize();
			v.texcoords[0]=CVec2( j/float(segs),i/float(segs) );
			surf->AddVertex( v );
		}
		v.texcoords[0].x=2;
		surf->AddVertex( v );
	}
	for( int j=0;j<segs2;++j ){
		float ty=-hlength-radius;
		CVertex v( 0,ty,0 );
		v.normal=CVec3( 0,-1,0 );
		v.texcoords[0]=CVec2( (j+1)/float(segs),1 );
		surf->AddVertex( v );
	}
	int v=0;
	for( int j=0;j<segs2;++j ){
		surf->AddTriangle( v,v+segs2+1,v+segs2 );
		++v;
	}
	for( int i=1;i<segs-1;++i ){
		for( int j=0;j<segs2;++j ){
			surf->AddTriangle( v,v+1,v+segs2+2 );
			surf->AddTriangle( v,v+segs2+2,v+segs2+1 );
			++v;
		}
		++v;
	}
	for( int j=0;j<segs2;++j ){
		surf->AddTriangle( v,v+1,v+segs2+1 );
		++v;
	}

	surf->UpdateTangents();
	CModel *model=new CModel;
	model->AddSurface( surf );
	surf->Release();
	if( collType || mass ) model->SetBody( App.World()->Physics()->CreateCapsuleBody( radius,length,collType,mass ) );
	return model;
}

CModel *CModelUtil::CreateCylinder( CMaterial *material,float radius,float length,int collType,float mass ){
	int segs=8;
	CModelSurface *surf=new CModelSurface;
	surf->SetMaterial( material );
	float hlength=length/2;
	CVertex v;
	v.position=CVec3( 0,hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 0,0 );
	surf->AddVertex( v );
	v.position=CVec3( 0,-hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 0,1 );
	surf->AddVertex( v );
	for( int i=1;i<segs;++i ){
		float x=sinf( i*TWOPI/segs );
		float z=-cosf( i*TWOPI/segs );
		CVertex v;
		v.position=CVec3( x*radius,hlength,z*radius );
		v.normal=CVec3( x,0,z );
		v.texcoords[0]=CVec2( i/float(segs),0 );
		surf->AddVertex( v );
		v.position=CVec3( x*radius,-hlength,z*radius );
		v.normal=CVec3( x,0,z );
		v.texcoords[0]=CVec2( i/float(segs),1 );
		surf->AddVertex( v );
	}
	v.position=CVec3( 0,hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 1,0 );
	surf->AddVertex( v );
	v.position=CVec3( 0,-hlength,-radius );
	v.normal=CVec3( 0,0,-1 );
	v.texcoords[0]=CVec2( 1,1 );
	surf->AddVertex( v );
	for( int i=0;i<segs;++i ){
		surf->AddTriangle( i*2,i*2+2,i*2+1 );
		surf->AddTriangle( i*2+2,i*2+3,i*2+1 );
	}
	
	surf->UpdateTangents();
	CModel *model=new CModel;
	model->AddSurface( surf );
	surf->Release();
	if( collType || mass ) model->SetBody( App.World()->Physics()->CreateCylinderBody( radius,length,collType,mass ) );
	return model;
}

CModel *CModelUtil::CreateBox( CMaterial *material,float width,float height,float depth,int collType,float mass ){
	CModelSurface *surf=new CModelSurface;
	surf->SetMaterial( material );
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
			surf->AddVertex( v );
			t+=5;
		}
		surf->AddTriangle( n,n+1,n+2 );
		surf->AddTriangle( n,n+2,n+3 );
		n+=4;
	}
	
	surf->UpdateTangents();
	CModel *model=new CModel;
	model->AddSurface( surf );
	surf->Release();
	if( collType || mass ) model->SetBody( App.World()->Physics()->CreateBoxBody( width,height,depth,collType,mass ) );
	return model;
}
