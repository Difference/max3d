
#include "std.h"

#include "app.h"
#include "entityutil.h"
#include "meshutil.h"

#include <assimp.hpp>
#include <aiScene.h>
#include <aiPostProcess.h>
//#include <defaultlogger.h>

static CVec3 ccolor( const aiColor4D &color ){
	return CVec3( color.r,color.g,color.b );
}

static string cstring( const aiString &str ){
	return string( str.data,str.length );
}

static CMaterial *cmaterial( const aiMaterial *mat,CShader *shader ){
	CMaterial *cmat=new CMaterial;
	
	cmat->SetShader( shader );
	
	aiColor4D color;
	aiString path;

	if( aiGetMaterialTexture( mat,aiTextureType_DIFFUSE,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "DiffuseMap",App.TextureUtil()->LoadTexture( cstring( path ) ) );
	}else if( aiGetMaterialColor( mat,AI_MATKEY_COLOR_DIFFUSE,&color )==AI_SUCCESS ){
		cmat->SetColor( "DiffuseColor",ccolor( color ) );
	}

	if( aiGetMaterialTexture( mat,aiTextureType_SPECULAR,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "SpecularMap",App.TextureUtil()->LoadTexture( cstring( path ) ) );
	}else if( aiGetMaterialColor( mat,AI_MATKEY_COLOR_SPECULAR,&color )==AI_SUCCESS ){
		cmat->SetColor( "SpecularColor",ccolor( color ) );
	}
	
	if( aiGetMaterialTexture( mat,aiTextureType_EMISSIVE,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "EmissiveMap",App.TextureUtil()->LoadTexture( cstring( path ) ) );
	}else if( aiGetMaterialColor( mat,AI_MATKEY_COLOR_EMISSIVE,&color )==AI_SUCCESS ){
		cmat->SetColor( "EmissiveColor",ccolor( color ) );
	}
	
	if( aiGetMaterialTexture( mat,aiTextureType_NORMALS,0,&path,0,0,0,0,0 )==AI_SUCCESS ){
		cmat->SetTexture( "NormalMap",App.TextureUtil()->LoadTexture( cstring( path ) ) );
	}

	return cmat;
}

static CModel *cmodel( const aiMesh *mesh ){
	CModel *model=new CModel;

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
		model->AddVertex( v );
	}

	aiFace *face=mesh->mFaces;
	for( int i=0;i<mesh->mNumFaces;++i ){
		model->AddTriangle( face->mIndices[0],face->mIndices[1],face->mIndices[2] );
		++face;
	}
	
//	model->UpdateNormals();
	model->UpdateTangents();

	return model;
}

CEntity *CEntityUtil::LoadEntity( const string &path,int collType,float mass,CShader *shader ){

	if( !shader ) shader=App.ShaderUtil()->MeshShader();

	Assimp::Importer importer;

//	Assimp::DefaultLogger::get()->setLogSeverity( Assimp::Logger::VERBOSE );
	
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
		CMaterial *cmat=cmaterial( mat,shader );
		mats.push_back( cmat );
	}

	CEntity *entity=new CEntity;
	
	CModel *physModel=(collType || mass) ? new CModel : 0;

	for( int i=0;i<scene->mNumMeshes;++i ){
		aiMesh *mesh=scene->mMeshes[i];

		CModel *model=cmodel( mesh );

		if( physModel ) physModel->Append( model );

		CMaterial *mat=mats[ mesh->mMaterialIndex ];

		entity->AddSurface( App.MeshUtil()->CreateMesh( mat,model ) );

		model->Release();
	}

	if( physModel ){
		CBody *body=App.World()->Physics()->CreateMesh( 
				&physModel->Vertices()[0],physModel->Vertices().size(),sizeof(CVertex),
				&physModel->Triangles()[0],physModel->Triangles().size(),sizeof(CTriangle),collType,mass );
		entity->SetBody( body );
		physModel->Release();
	}

	for( int i=0;i<mats.size();++i ){
		mats[i]->Release();
	}

	return entity;
}

CEntity *CEntityUtil::CreateSphere( CMaterial *material,float radius,int collType,float mass ){
	CEntity *entity=new CEntity;
	entity->AddSurface( App.MeshUtil()->CreateSphereMesh( material,radius ) );
	if( collType || mass ) entity->SetBody( App.World()->Physics()->CreateSphere( radius,collType,mass ) );
	return entity;
}

CEntity *CEntityUtil::CreateCapsule( CMaterial *material,float radius,float length,int collType,float mass ){
	CEntity *entity=new CEntity;
	entity->AddSurface( App.MeshUtil()->CreateCapsuleMesh( material,radius,length ) );
	if( collType || mass ) entity->SetBody( App.World()->Physics()->CreateCapsule( radius,length,collType,mass ) );
	return entity;
}

CEntity *CEntityUtil::CreateCylinder( CMaterial *material,float radius,float length,int collType,float mass ){
	CEntity *entity=new CEntity;
	entity->AddSurface( App.MeshUtil()->CreateCylinderMesh( material,radius,length ) );
	if( collType || mass ) entity->SetBody( App.World()->Physics()->CreateCylinder( radius,length,collType,mass ) );
	return entity;
}

CEntity *CEntityUtil::CreateBox( CMaterial *material,float width,float height,float depth,int collType,float mass ){
	CEntity *entity=new CEntity;
	entity->AddSurface( App.MeshUtil()->CreateBoxMesh( material,width,height,depth ) );
	if( collType || mass ) entity->SetBody( App.World()->Physics()->CreateBox( width,height,depth,collType,mass ) );
	return entity;
}
