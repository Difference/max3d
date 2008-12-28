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
#include "bulletphysics.h"

#ifdef USE_BULLET

static CBulletPhysics *BulletPhysics(){
	return (CBulletPhysics*)App.World()->Physics();
}

CBulletBody::CBulletBody(){
}

CBulletBody *CBulletBody::Create( btCollisionShape *shape,int collType,float mass ){
	_btShape=shape;
	_collType=collType;
	_mass=mass;
	_btBody=new btRigidBody( fabs(_mass),0,_btShape,btVector3(0,0,0) );
	_btBody->setFriction( 0 );
	_btBody->setRestitution( 0 );
	_btBody->setDamping( 0,0 );
	if( _mass<0 ) _btBody->setAngularFactor( 0 );
	BulletPhysics()->BulletWorld()->addRigidBody( _btBody );
	return this;
}

CBulletBody::~CBulletBody(){
}

void CBulletBody::SetEnabled( bool enabled ){
	throw( "TODO" );
}

CBulletBody *CBulletBody::CreateSphere( float radius,int collType,float mass ){
	return Create( new btSphereShape( radius ),collType,mass );
}

CBulletBody *CBulletBody::CreateCapsule( float radius,float length,int collType,float mass ){
	return Create( new btCapsuleShape( radius,length ),collType,mass );
}

CBulletBody *CBulletBody::CreateCylinder( float radius,float length,int collType,float mass ){
	Error( "TODO!" );
	return 0;
//	return Create( new btCylinderShape( radius,length ),collType,mass );
}

CBulletBody *CBulletBody::CreateBox( float width,float height,float depth,int collType,float mass ){
	return Create( new btBoxShape( btVector3( width/2,height/2,depth/2 ) ),collType,mass );
}

CBulletBody *CBulletBody::CreateMesh( const void *verts,int n_verts,int verts_pitch,const void *tris,int n_tris,int tris_pitch,int collType,float mass ){
	btTriangleMesh *mesh=new btTriangleMesh();
	btVector3 v0,v1,v2;
	for( int i=0;i<n_tris;++i ){
		int *p=(int*)tris;
		memcpy( &v0,(char*)verts+p[0]*verts_pitch,12 );
		memcpy( &v1,(char*)verts+p[1]*verts_pitch,12 );
		memcpy( &v2,(char*)verts+p[2]*verts_pitch,12 );
		mesh->addTriangle( v0,v1,v2 );
		tris=(char*)tris+tris_pitch;
	}
	return Create( new btBvhTriangleMeshShape( mesh,false ),collType,mass );
}

void CBulletBody::SetMatrix( const CMat4 &m ){
	btMatrix3x3 r( m.i.x,m.i.y,m.i.z,m.j.x,m.j.y,m.j.z,m.k.x,m.k.y,m.k.z );
	btVector3 c( m.t.x,m.t.y,m.t.z );
	btTransform t( r,c );
	_btBody->setWorldTransform( t );
	_btBody->setLinearVelocity( btVector3( 0,0,0 ) );
	_btBody->clearForces();
}

CMat4 CBulletBody::Matrix(){
	const btTransform &t=_btBody->getWorldTransform();
	const btMatrix3x3 &r=t.getBasis();
	const btVector3 &c=t.getOrigin();
	CMat4 m;
	memcpy( &m.i,&r[0],12 );
	memcpy( &m.j,&r[1],12 );
	memcpy( &m.k,&r[2],12 );
	memcpy( &m.t,&c,12 );
	return m;
}

CBulletBody *CBulletBody::OnCopy( CCopier *copier ){
	return (new CBulletBody)->Create( _btShape,_collType,_mass );	//share that shape, baby!
}

void CBulletBody::OnAttach( CEntity *entity ){
}

void CBulletBody::OnValidate( CEntity *entity ){
	if( entity->MatrixModified() ) SetMatrix( entity->Matrix() );
	entity->LockMatrix();
}

void CBulletBody::OnAnimate( CEntity *entity ){
	if( _mass<0 ) _btBody->setLinearVelocity( btVector3( 0,0,0 ) );
	entity->SetMatrix( Matrix() );
	entity->UnlockMatrix();
}

CBulletPhysics::CBulletPhysics(){

	btVector3 worldAabbMin( -10000,-10000,-10000 );
	btVector3 worldAabbMax( 10000,10000,10000 );
	int maxProxies=1024;

	_broadphase=new btAxisSweep3( worldAabbMin,worldAabbMax,maxProxies );
	_collisionConfiguration=new btDefaultCollisionConfiguration();
	_dispatcher=new btCollisionDispatcher( _collisionConfiguration );
	_solver=new btSequentialImpulseConstraintSolver;
	_dynamicsWorld=new btDiscreteDynamicsWorld( _dispatcher,_broadphase,_solver,_collisionConfiguration );
}

CBulletPhysics::~CBulletPhysics(){
	delete _dynamicsWorld;
	delete _solver;
	delete _dispatcher;
	delete _collisionConfiguration;
	delete _broadphase;
}

CBulletBody *CBulletPhysics::CreateSphere( float radius,int collType,float mass ){
	return (new CBulletBody)->CreateSphere( radius,collType,mass );
}

CBulletBody *CBulletPhysics::CreateCapsule( float radius,float length,int collType,float mass ){
	return (new CBulletBody)->CreateCapsule( radius,length,collType,mass );
}

CBulletBody *CBulletPhysics::CreateCylinder( float radius,float length,int collType,float mass ){
	return (new CBulletBody)->CreateCylinder( radius,length,collType,mass );
}

CBulletBody *CBulletPhysics::CreateBox( float width,float height,float depth,int collType,float mass ){
	return (new CBulletBody)->CreateBox( width,height,depth,collType,mass );
}

CBulletBody *CBulletPhysics::CreateMesh( const void *verts,int n_verts,int verts_pitch,const void *tris,int n_tris,int tris_pitch,int collType,float mass ){
	return (new CBulletBody)->CreateMesh( verts,n_verts,verts_pitch,tris,n_tris,tris_pitch,collType,mass );
}

void CBulletPhysics::SetGravity( const CVec3 &g ){
	_dynamicsWorld->setGravity( btVector3( g.x,g.y,g.z ) );
}

void CBulletPhysics::EnableCollisions( int collType1,int collType2,float friction,float bounciness ){
	Error( "TODO!" );
}

void CBulletPhysics::Update(){
	_dynamicsWorld->stepSimulation( 1/60.f,10 );
}

#endif
