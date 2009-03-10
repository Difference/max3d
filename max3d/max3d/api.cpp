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

/// @file
/// @brief Max3d procedural API.

#include "std.h"

#include "app.h"
#include "api.h"
#include "entity.h"
#include "camera.h"
#include "light.h"
#include "model.h"
#include "pivot.h"
#include "surface.h"
#include "material.h"
#include "sprite.h"
#include "terrain.h"
#include "mirror.h"
#include "bsptree.h"

#if _WIN32
#define API __declspec(dllexport)
#else
#define API __attribute__((visibility("default")))
#endif

#define _API

static float to_radians=PI/180.0f;

static const char *c_str( const string &t ){
	static int sz;static char *buf;
	if( t.size()+1>sz ){ delete[] buf;sz=t.size()+1;buf=new char[sz]; }
	strcpy( buf,t.c_str() );return buf;
}

extern "C"{	//these will need to be CDECL too for lua...

//***** App API *****
//

/// Initalize Max3d - must be called before any other commands
API void m3dInitMax3D( void *importer,int flags ){
	App.Init( (TObjectImporter)importer,flags );
}

API void m3dUseDegrees(){
	to_radians=PI/180.0f;
}

API void m3dUseRadians(){
	to_radians=1.0f;
}

API void m3dSetObjectImportPath( CObject *obj,const char *path ){
	obj->SetImportPath( path );
}

API const char *m3dGetObjectImportPath( CObject *obj ){
	return c_str( obj->ImportPath() );
}

//***** Object API *****
_API const char *m3dMax3dObjectType( CObject *obj ){
	return c_str( obj->TypeName() );
}

_API void m3dSaveMax3dObject( CObject *obj,const char *path ){
	if( obj ) obj->Write( path );
}

_API CObject *m3dLoadMax3dObject( const char *path ){
	return CObject::Read( path );
}

//***** Resource API ******
API void m3dFlushResources(){
	CResource::FlushResources();
}

API void m3dRetainResource( CResource *obj ){
	obj->Retain();
}

API void m3dReleaseResource( CResource *obj ){
	obj->Release();
}

//***** Texture API *****
API CTexture *m3dBlackTexture(){
	return App.TextureUtil()->BlackTexture();
}

API CTexture *m3dWhiteTexture(){
	return App.TextureUtil()->WhiteTexture();
}

API CTexture *m3dCreateTexture( int width,int height,int format,int flags ){
	return App.Graphics()->CreateTexture( width,height,format,flags );
}

API void m3dSetTextureData( CTexture *texture,const void *data ){
	texture->SetData( data );
}

API CTexture *m3dCreate3dTexture( int width,int height,int depth,int format,int flags ){
	return App.Graphics()->Create3dTexture( width,height,depth,format,flags );
}

API void m3dSet3dTextureData( CTexture *texture,const void *data ){
	texture->Set3dData( data );
}

API CTexture *m3dCreateCubeTexture( int size,int format,int flags ){
	return App.Graphics()->CreateCubeTexture( size,format,flags );
}

API void m3dSetCubeTextureData( CTexture *texture,const void *data ){
	texture->SetCubeData( data );
}

//***** Shader API *****
API CShader *m3dCreateShader( const char *source ){
	return App.Graphics()->CreateShader( source );
}

//***** Material API *****
API CMaterial *m3dCreateMaterial(){
	return new CMaterial;
}

API void m3dSetMaterialName( CMaterial *material,const char *name ){
	material->SetName( name );
}

API const char *m3dGetMaterialName( CMaterial *material ){
	return c_str( material->Name() );
}

API void m3dSetMaterialFloat( CMaterial *material,const char *name,float value ){
	material->SetFloat( name,value );
}

API void m3dSetMaterialColor( CMaterial *material,const char *name,float red,float green,float blue ){
	material->SetColor( name,CVec3( red,green,blue ) );
}

API void m3dSetMaterialTexture( CMaterial *material,const char *name,CTexture *texture ){
	material->SetTexture( name,texture );
}

//***** Surface API *****
API CModelSurface *m3dCreateSurface( CMaterial *material,CModel *model ){
	CModelSurface *surface=new CModelSurface;
	if( material ) surface->SetMaterial( material );
	if( model ) model->AddSurface( surface );
	return surface;
}

API void m3dSetSurfaceShader( CModelSurface *surface,CShader *shader ){
	surface->SetShader( shader );
}

API CShader *m3dGetSurfaceShader( CModelSurface *surface ){
	return surface->Shader();
}

API void m3dSetSurfaceMaterial( CModelSurface *surface,CMaterial *material ){
	surface->SetMaterial( material );
}

API CMaterial *m3dGetSurfaceMaterial( CModelSurface *surface ){
	return surface->Material();
}

API void m3dClearSurface( CModelSurface *surface ){
	surface->Clear();
}

API void m3dAddSurfaceVertex( CModelSurface *surface,float x,float y,float z,float s0,float t0 ){
	CVertex vertex;
	vertex.position=CVec3( x,y,z );
	vertex.texcoords[0]=CVec2( s0,t0 );
	surface->AddVertex( vertex );
}

API void m3dAddSurfaceTriangle( CModelSurface *surface,int vertex0,int vertex1,int vertex2 ){
	surface->AddTriangle( vertex0,vertex1,vertex2 );
}

//***** Entity API *****
API void m3dDestroyEntity( CEntity *entity ){
	delete entity;
}

API CEntity *m3dCopyEntity( CEntity *entity ){
	CEntity *copy=(CEntity*)entity->Copy();
	copy->SetVisible( true );
	return copy;
}

API void m3dShowEntity( CEntity *entity ){
	entity->SetVisible( true );
}

API void m3dHideEntity( CEntity *entity ){
	entity->SetVisible( false );
}

API void m3dSetEntityParent( CEntity *entity,CEntity *parent ){
	entity->SetParent( parent );
}

API CEntity *m3dEntityParent( CEntity *entity ){
	return entity->Parent();
}

API float m3dEntityMatrixElement( CEntity *entity,int row,int column ){
	return entity->WorldMatrix()[row][column];
}

// Translation stuff...
API void m3dSetEntityTranslation( CEntity *entity,float x,float y,float z ){
	CVec3 v( x,y,z );
	entity->SetTranslation( v );
}

API void m3dMoveEntity( CEntity *entity,float x,float y,float z ){
	CVec3 v( x,y,z );
	entity->SetTranslation( entity->Translation() + CMat4::RotationMatrix( entity->Rotation() ) * v );
}

API float m3dEntityX( CEntity *entity ){
	return entity->Translation().x;
}

API float m3dEntityY( CEntity *entity ){
	return entity->Translation().y;
}

API float m3dEntityZ( CEntity *entity ){
	return entity->Translation().z;
}

// Rotation stuff...
API void m3dSetEntityRotation( CEntity *entity,float yaw,float pitch,float roll ){
	CQuat rot=CQuat::YawPitchRollQuat( CVec3( yaw,pitch,roll ) * to_radians );
	entity->SetRotation( rot );
}

API void m3dTurnEntity( CEntity *entity,float yaw,float pitch,float roll ){
	CQuat rot=CQuat::YawPitchRollQuat( CVec3( yaw,pitch,roll ) * to_radians );
	entity->SetRotation( entity->Rotation() * rot );
}

API float m3dEntityYaw( CEntity *entity ){
	return entity->Rotation().Yaw()/to_radians;
}

API float m3dEntityPitch( CEntity *entity ){
	return entity->Rotation().Pitch()/to_radians;
}

API float m3dEntityRoll( CEntity *entity ){
	return entity->Rotation().Roll()/to_radians;
}

// Scale stuff
API void m3dSetEntityScale( CEntity *entity,float x,float y,float z ){
	CVec3 v( x,y,z );
	entity->SetScale( v );
}

//***** Model API *****
API CModel *m3dLoadModel( const char *path,int collType,float mass ){
	CModel *model=App.ModelUtil()->ImportModel( path,collType,mass );
	if( model ) model->SetVisible( true );
	return model;
}

API CModel *m3dCreateModel(){
	CModel *model=new CModel;
	model->SetVisible( true );
	return model;
}

API CModel *m3dCreateSphere( CMaterial *material,float radius,int collType,float mass ){
	CModel *model=App.ModelUtil()->CreateSphere( material,radius,collType,mass );
	model->SetVisible( true );
	return model;
}

API CModel *m3dCreateCapsule( CMaterial *material,float radius,float length,int collType,float mass ){
	CModel *model=App.ModelUtil()->CreateCapsule( material,radius,length,collType,mass );
	model->SetVisible( true );
	return model;
}

API CModel *m3dCreateCylinder( CMaterial *material,float radius,float length,int collType,float mass ){
	CModel *model=App.ModelUtil()->CreateCylinder( material,radius,length,collType,mass );
	model->SetVisible( true );
	return model;
}

API CModel *m3dCreateBox( CMaterial *material,float width,float height,float depth,int collType,float mass ){
	CModel *model=App.ModelUtil()->CreateBox( material,width,height,depth,collType,mass );
	model->SetVisible( true );
	return model;
}

API int m3dCountModelSurfaces( CModel *model ){
	return model->Surfaces().size();
}

API CModelSurface *m3dGetModelSurface( CModel *model,int index ){
	return model->Surfaces()[ index ];
}

API void m3dUpdateModelNormals( CModel *model ){
	model->UpdateNormals();
}

API void m3dUpdateModelTangents( CModel *model ){
	model->UpdateTangents();
}

API void m3dScaleModel( CModel *model,float x,float y,float z ){
	model->Scale( CVec3( x,y,z ) );
}

API void m3dScaleModelSurfaces( CModel *model,float x,float y,float z ){
	model->TransformSurfaces( CMat4::ScaleMatrix( CVec3( x,y,z ) ) );
}

API void m3dScaleModelTexCoords( CModel *model,float s_scale,float t_scale ){
	model->ScaleTexCoords( s_scale,t_scale );
}

API void m3dResetModelTransform( CModel *model ){
	model->ResetTransform();
}

API void m3dFlipModel( CModel *model ){
	model->Flip();
}

API void m3dSplitModelEdges( CModel *model,float maxlength ){
	model->SplitEdges( maxlength );
}

//***** Pivot API *****
API CEntity *m3dCreatePivot(){
	CEntity *pivot=new CPivot;
	pivot->SetVisible( true );
	return pivot;
}

//***** Camera API *****
API CCamera *m3dCreateCamera(){
	CCamera *camera=new CCamera;
	camera->SetVisible( true );
	return camera;
}

API void m3dSetCameraViewport( CCamera *camera,int x,int y,int width,int height ){
	camera->SetViewport( CRect( x,y,width,height ) );
}

API void m3dSetCameraOrtho( CCamera *camera,float left,float right,float bottom,float top,float zNear,float zFar ){
	camera->SetProjectionMatrix( CMat4::OrthoMatrix( left,right,bottom,top,zNear,zFar ) );
}

API void m3dSetCameraFrustum( CCamera *camera,float left,float right,float bottom,float top,float zNear,float zFar ){
	camera->SetProjectionMatrix( CMat4::FrustumMatrix( left,right,bottom,top,zNear,zFar ) );
}

API void m3dSetCameraPerspective( CCamera *camera,float fovy,float aspect,float zNear,float zFar ){
	camera->SetProjectionMatrix( CMat4::PerspectiveMatrix( fovy * to_radians,aspect,zNear,zFar ) );
}

static CVec3 projectedPoint;

API int m3dCameraProject( CCamera *camera,float x,float y,float z ){
	CVec4 t=camera->ProjectionMatrix() * camera->InverseWorldMatrix() * CVec4( x,y,z,1 );
	
	projectedPoint=t.xyz()/t.w;
	
	projectedPoint=projectedPoint/2.0f+.5f;
	
	projectedPoint.xy()*=CVec2( camera->Viewport().width,camera->Viewport().height );
	
	return projectedPoint.z>0;
}

API float m3dProjectedPoint( int coord ){
	return projectedPoint[coord];
}

static float pickedTime;
static CVec3 pickedPoint;
static CVec3 pickedNormal;
static CEntity *pickedEntity;

API CEntity *m3dCameraPick( CCamera *camera,float viewport_x,float viewport_y,int collType ){
	
	//not quite right - should go through inverse proj matrix for complete generality
	//and all round sexiness.
	
	float farz=camera->FarZ();
	
	CVec3 o=camera->WorldMatrix().t.xyz();
	
	float ix=camera->ProjectionMatrix().i.x;
	float jy=camera->ProjectionMatrix().j.y;
	
	CVec3 v=camera->WorldMatrix() * CVec3(
		(viewport_x/camera->Viewport().width*2-1)/ix*farz,
		(viewport_y/camera->Viewport().height*2-1)/jy*farz,
		farz );

	CLine ray( o,v-o );
	
//	cout<<ray<<endl;

	if( CBody *body=App.World()->Physics()->TraceRay( ray,collType,&pickedTime,&pickedPoint,&pickedNormal ) ){
		pickedEntity=(CEntity*)body->Data();
	}else{
		pickedEntity=0;
	}
	return pickedEntity;
}

API float m3dPickedPoint( int coord ){
	return pickedPoint[coord];
}

API float m3dPickedNormal( int coord ){
	return pickedNormal[coord];
}

API CEntity *m3dPickedEntity(){
	return pickedEntity;
}

API float m3dPickedTime(){
	return pickedTime;
}

//***** Light API *****
API CLight *m3dCreateSpotLight(){
	CLight *light=new CLight;
	light->SetShader( App.ShaderUtil()->SpotLightShader() );
	light->SetVisible( true );
	return light;
}

API CLight *m3dCreatePointLight(){
	CLight *light=new CLight;
	light->SetShader( App.ShaderUtil()->PointLightShader() );
	light->SetVisible( true );
	return light;
}

API CLight *m3dCreateDistantLight(){
	CLight *light=new CLight;
	light->SetShader( App.ShaderUtil()->DistantLightShader() );
	light->SetVisible( true );
	return light;
}

API void m3dSetLightAngle( CLight *light,float angle ){
	light->SetAngle( angle );
}

API void m3dSetLightRange( CLight *light,float range ){
	light->SetRange( range );
}

API void m3dSetLightColor( CLight *light,float red,float green,float blue ){
	light->SetColor( CVec3(red,green,blue) );
}

API void m3dSetLightTexture( CLight *light,CTexture *texture ){
	light->SetTexture( texture );
}

API void m3dSetLightShadowSize( CLight *light,int size ){
	light->SetShadowSize( size );
}

API void m3dSetLightShadowSplits( CLight *light,int splitCount,float znear,float zfar,float blend ){
	light->SetShadowSplits( CLight::ComputeShadowSplits( splitCount,znear,zfar,blend ) );
}

API void m3dSetLightShadowSplitsTable( CLight *light,int floatCount,const void *floats ){
	vector<float> splits( (const float*)floats,((const float*)floats)+floatCount );
	light->SetShadowSplits( splits );
}

//***** Sprite API *****
API CSprite *m3dCreateSprite( CMaterial *material ){
	CSprite *sprite=new CSprite;
	sprite->SetMaterial( material );
	sprite->SetVisible( true );
	return sprite;
}

//***** Terrain API *****
API CTerrain *m3dCreateTerrain( CMaterial *material,int xsize,int zsize,float width,float height,float depth ){
	CTerrain *terrain=new CTerrain;
	terrain->SetMaterial( material );
	terrain->SetData( xsize,zsize,width,height,depth,0 );
	terrain->SetVisible( true );
	return terrain;
}

API void m3dSetTerrainHeight( CTerrain *terrain,float height,int x,int z ){
	terrain->SetHeight( height,x,z );
}

//***** Mirror *****
API CMirror *m3dCreateMirror(){
	CMirror *mirror=new CMirror;
	mirror->SetVisible( true );
	return mirror;
}

API void m3dSetMirrorSize( CMirror *mirror,float width,float height ){
	mirror->SetSize( width,height );
}

API void m3dSetMirrorResolution( CMirror *mirror,int width,int height ){
	mirror->SetResolution( width,height );
}

//***** Physics *****
API void m3dCreateSphereBody( CEntity *entity,float radius,int collType,float mass ){
	CBody *body=App.World()->Physics()->CreateSphereBody( radius,collType,mass );
	entity->SetBody( body );
}

API void m3dCreateCapsuleBody( CEntity *entity,float radius,float length,int collType,float mass ){
	CBody *body=App.World()->Physics()->CreateCapsuleBody( radius,length,collType,mass );
	entity->SetBody( body );
}

API void m3dCreateCylinderBody( CEntity *entity,float radius,float length,int collType,float mass ){
	CBody *body=App.World()->Physics()->CreateCylinderBody( radius,length,collType,mass );
	entity->SetBody( body );
}

API void m3dCreateBoxBody( CEntity *entity,float width,float height,float depth,int collType,float mass ){
	CBody *body=App.World()->Physics()->CreateBoxBody( width,height,depth,collType,mass );
	entity->SetBody( body );
}

API void m3dCreateSurfaceBody( CEntity *entity,CModelSurface *surface,int collType,float mass ){
	CBody *body=App.World()->Physics()->CreateSurfaceBody( surface,collType,mass );
	entity->SetBody( body );
}

API void m3dCreateModelBody( CEntity *entity,CModel *model,int collType,float mass ){
	CBody *body=App.ModelUtil()->CreateModelBody( model,collType,mass );
	entity->SetBody( body );
}

API void m3dCreateTerrainBody( CEntity *entity,CTerrain *terrain,int collType,float mass ){
	CBody *body=App.World()->Physics()->CreateSurfaceBody( terrain->Surface(),collType,mass );
	entity->SetBody( body );
}

API void m3dCreateBallJoint( CEntity *entity,CEntity *body1,CEntity *body2 ){
	CJoint *joint=App.World()->Physics()->CreateBallJoint();
	joint->Attach( body1 ? body1->Body() : 0,body2 ? body2->Body() : 0 );
	entity->SetJoint( joint );
}

//***** Animator ******
API void m3dCreateAnimator( CEntity *entity ){
	CAnimator *animator=new CAnimator;
	entity->SetAnimator( animator );
}

API void m3dSetAnimationKey( CEntity *entity,int seq,float time,CEntity *keyEntity,int flags ){
	entity->Animator()->SetKey( seq,keyEntity,time,flags );
}

API void m3dSetAnimatorTime( CEntity *entity,int seq,float time ){
	entity->Animator()->Animate( seq,time );
}

//***** World API *****//
API void m3dEnableShadows(){
	App.Scene()->SetShadowsEnabled( true );
}

API void m3dDisableShadows(){
	App.Scene()->SetShadowsEnabled( false );
}

API void m3dEnableCollisions( int collType1,int collType2,float friction,float bounciness,float stiffness ){
	App.World()->Physics()->EnableCollisions( collType1,collType2,friction,bounciness,stiffness );
}

API void m3dSetGravity( float x,float y,float z ){
	App.World()->Physics()->SetGravity( CVec3( x,y,z ) );
}

API void m3dSetClearColor( float r,float g,float b ){
	App.World()->SetClearColor( CVec3( r,g,b ) );
}

API void m3dSetAmbientColor( float r,float g,float b ){
	App.World()->SetAmbientColor( CVec3( r,g,b ) );
}

API void m3dUpdateWorld(){
	App.World()->Update();
}

API void m3dRenderWorld(){
	App.World()->Render();
}

//***** RenderPass API *****
API void m3dAddRenderPass( CShader *shader,CMaterial *material ){
	CRenderPass *pass=new CRenderPass;
	pass->SetShader( shader );
	pass->SetMaterial( material );
	App.Scene()->AddRenderPass( pass );
}

API void m3dClearRenderPasses(){
	App.Scene()->ClearRenderPasses();
}

//***** BSPTree API *****//

_API CBSPTree *CreateModelBSP( CModel *model ){
	return new CBSPTree( model );
}

_API int m3dCountBSPNodes( CBSPTree *tree ){
	vector<CBSPNode*> nodes;
	tree->Root()->EnumNodes( nodes );
	return nodes.size();
}

_API int m3dCountBSPLeaves( CBSPTree *tree ){
	vector<CBSPNode*> leaves;
	tree->Root()->EnumLeaves( leaves );
	return leaves.size();
}

_API CModel *CreateBSPModel( CBSPTree *tree ){
	CModel *model=tree->BuildModel();
	model->SetVisible( true );
	return model;
}

}
