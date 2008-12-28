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

#ifndef PHYSICS_H
#define PHYSICS_H

#ifdef USE_BULLET

#include "physics.h"

#include <btBulletDynamicsCommon.h>

#undef LoadString

class CBulletBody : public CBody{
public:
	void SetEnabled( bool enabled );
	void OnAttach( CEntity *entity );
	void OnValidate( CEntity *entity );
	void OnAnimate( CEntity *entity );

	void SetMatrix( const CMat4 &m );
	CMat4 Matrix();

private:
	friend class CBulletPhysics;

	CBulletBody();
	~CBulletBody();

	CBulletBody *Create( btCollisionShape *shape,int collType,float mass );

	CBulletBody *CreateSphere( float radius,int collType,float mass );
	CBulletBody *CreateCapsule( float radius,float length,int collType,float mass );
	CBulletBody *CreateCylinder( float radius,float length,int collType,float mass );
	CBulletBody *CreateBox( float width,float height,float depth,int collType,float mass );
	CBulletBody *CreateMesh( const void *verts,int n_verts,int verts_pitch,const void *tris,int n_tris,int tris_pitch,int collType,float mass );

	CBulletBody *OnCopy( CCopier *copier );

	btCollisionShape *_btShape;
	int _collType;
	float _mass;
	btRigidBody *_btBody;
};

class CBulletPhysics : public CPhysics{
public:
	//PUBLIC
	CBulletPhysics();
	~CBulletPhysics();

	CBulletBody *CreateSphere( float radius,int collType,float mass );
	CBulletBody *CreateCapsule( float radius,float length,int collType,float mass );
	CBulletBody *CreateCylinder( float radius,float length,int collType,float mass );
	CBulletBody *CreateBox( float width,float height,float depth,int collType,float mass );
	CBulletBody *CreateMesh( const void *verts,int n_verts,int verts_pitch,const void *tris,int n_tris,int tris_pitch,int collType,float mass );

	void SetGravity( const CVec3 &gravity );
	void EnableCollisions( int colltype1,int collType2,float friction,float bounciness );
	void Update();

	//PRIVATE
	btDynamicsWorld *BulletWorld(){ return _dynamicsWorld; }

private:
	btAxisSweep3 *_broadphase;
	btDefaultCollisionConfiguration *_collisionConfiguration;
	btCollisionDispatcher *_dispatcher;
	btSequentialImpulseConstraintSolver *_solver;
	btDiscreteDynamicsWorld *_dynamicsWorld;
};

#endif

#endif
