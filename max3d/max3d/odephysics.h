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

#ifndef ODEPHYSICS_H
#define ODEPHYSICS_H

#include "physics.h"

#include <ode/ode.h>

#undef LoadString

//ODE driver...
class COdeBody : public CBody{
public:
	void SetEnabled( bool enabled );

	void Validate( const CMat4 &matrix,bool modified );
	CMat4 Animate();

private:
	class COdeMeshData : public CResource{
	public:
		COdeMeshData( const void *verts,int n_verts,int verts_pitch,const void *tris,int n_tris,int tris_pitch );
		~COdeMeshData();

		dTriMeshDataID TriMeshData(){ return _data; }

	private:
		dTriMeshDataID _data;
		char *_verts,*_tris;
	};

	friend class COdeJoint;
	friend class COdePhysics;

	COdeBody();
	~COdeBody();
	COdeBody( COdeBody *body,CCopier *copier );
	COdeBody *Create( dGeomID geom,int collType,float mass );
	COdeBody *CreateSphere( float radius,int collType,float mass );
	COdeBody *CreateCapsule( float radius,float length,int collType,float mass );
	COdeBody *CreateCylinder( float radius,float length,int collType,float mass );
	COdeBody *CreateBox( float width,float height,float depth,int collType,float mass );
	COdeBody *CreateSurface( CModelSurface *surface,int collType,float mass );
	COdeBody *CreateSurface( COdeMeshData *meshData,int collType,float mass );

	COdeBody *OnCopy( CCopier *copier ){ return new COdeBody( this,copier ); }

	void SetMatrix( const CMat4 &m );
	void SetVelocities( const CMat4 &m );
	CMat4 Matrix();

	bool _reset;
	dGeomID _odeGeom;
	dBodyID _odeBody;
	COdeMeshData *_odeMeshData;
};

class COdeJoint : public CJoint{
public:
	void Attach( CBody *body1,CBody *body2 );
	
	void Validate( const CMat4 &matrix,bool modified );
	CMat4 Animate();
	
private:
	friend class COdePhysics;

	COdeJoint();
	~COdeJoint();
	COdeJoint( COdeJoint *joint,CCopier *copier );
	COdeJoint *Create( dJointID joint );
	COdeJoint *CreateBall();
	void SetMatrix( const CMat4 &m );
	CMat4 Matrix();
	
	COdeJoint *OnCopy( CCopier *copier ){ return new COdeJoint( this,copier ); }
	
	dJointID _odeJoint;
	COdeBody *_body1,*_body2;
};

class COdePhysics : public CPhysics{
public:
	//PUBLIC
	COdePhysics();
	~COdePhysics();

	COdeBody *CreateSphereBody( float radius,int collType,float mass );
	COdeBody *CreateCapsuleBody( float radius,float length,int collType,float mass );
	COdeBody *CreateCylinderBody( float radius,float length,int collType,float mass );
	COdeBody *CreateBoxBody( float width,float height,float depth,int collType,float mass );
	COdeBody *CreateSurfaceBody( CModelSurface *surface,int collType,float mass );

	COdeJoint *CreateBallJoint();
	
	void SetGravity( const CVec3 &gravity );
	void EnableCollisions( int colltype1,int collType2,float friction,float bounciness,float stiffness );
	void Update();
	CBody *TraceRay( const CLine &ray,int collType,float *time,CVec3 *point,CVec3 *normal );

	//PRIVATE
	dWorldID OdeWorld(){ return _odeWorld; }
	dSpaceID OdeSpace(){ return _odeSpace; }
	int CollideBits( int collType ){ return _collideBits[collType]; }

	void OdeUpdateCallback( dGeomID geom1,dGeomID geom2 );
	void OdeTraceRayCallback( dGeomID geom1,dGeomID geom2 );

private:
	dWorldID _odeWorld;
	dSpaceID _odeSpace;
	int _collideBits[32];
	dSurfaceParameters _surfaces[32*32];	//lazy!
	dContactGeom _rayContact;
};

#endif
