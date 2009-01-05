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
#include "odephysics.h"

static const int ODE_MAX_CONTACTS=100;

static dJointGroupID odeContactGroup;
static dContact odeContacts[ODE_MAX_CONTACTS];

static void OdeCallback( void *data,dGeomID geom1,dGeomID geom2 ){
	((COdePhysics*)data)->OdeCallBack( geom1,geom2 );
}

static COdePhysics *OdePhysics(){
	return (COdePhysics*)App.World()->Physics();
}

void COdePhysics::OdeCallBack( dGeomID geom1,dGeomID geom2 ){

	int n=dCollide( geom1,geom2,ODE_MAX_CONTACTS,&odeContacts[0].geom,sizeof(dContact) );
	if( !n ) return;

	dBodyID odeBody1=dGeomGetBody( geom1 );
	dBodyID odeBody2=dGeomGetBody( geom2 );
		
	//spotted in ode demos!
	//if( odeBody1 && odeBody2 && dAreConnectedExcluding( odeBody1,odeBody2,dJointTypeContact ) ) return;

	COdeBody *body1=(COdeBody*)dGeomGetData( geom1 );
	COdeBody *body2=(COdeBody*)dGeomGetData( geom2 );

	const dSurfaceParameters &surface=_surfaces[body1->CollType()*32+body2->CollType()];
	
	for( int i=0;i<n;++i ){
		odeContacts[i].surface=surface;
		dJointID joint=dJointCreateContact( _odeWorld,odeContactGroup,&odeContacts[i] );
		dJointAttach( joint,odeBody1,odeBody2 );
	}
}

COdeBody::COdeMeshData::COdeMeshData( const void *verts,int n_verts,int verts_pitch,const void *tris,int n_tris,int tris_pitch ){
	char *p=_verts=new char[ n_verts * 12 ];
	for( int i=0;i<n_verts;++i ){
		memcpy( p,verts,12 );
		verts=(char*)verts+verts_pitch;
		p+=12;
	}
	p=_tris=new char[ n_tris * 12 ];
	for( int i=0;i<n_tris;++i ){
		memcpy( p,tris,12 );
		tris=(char*)tris+tris_pitch;
		p+=12;
	}
	_data=dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle( _data,_verts,12,n_verts,_tris,n_tris*3,12 );
}

COdeBody::COdeMeshData::~COdeMeshData(){
	dGeomTriMeshDataDestroy( _data );
	delete[] _verts;
	delete[] _tris;
}

COdeBody::COdeBody():_odeMeshData(0),_reset(true){
}

COdeBody::COdeBody( COdeBody *body,CCopier *copier ):_odeMeshData(0){

	dGeomID odeGeom=body->_odeGeom;
	int collType=body->_collType;
	float mass=body->_mass;
	
	switch( dGeomGetClass( odeGeom ) ){
	case dSphereClass:{
		float radius=dGeomSphereGetRadius( odeGeom );
		CreateSphere( radius,collType,mass );
		break;
		}
	case dCapsuleClass:{
		float radius,length;
		dGeomCapsuleGetParams( odeGeom,&radius,&length );
		CreateCapsule( radius,length,collType,mass );
		break;
		}
	case dCylinderClass:{
		float radius,length;
		dGeomCylinderGetParams( odeGeom,&radius,&length );
		CreateCylinder( radius,length,collType,mass );
		break;
		}
	case dBoxClass:{
		dVector3 lengths;
		dGeomBoxGetLengths( odeGeom,lengths );
		CreateBox( lengths[0],lengths[1],lengths[2],collType,mass );
		break;
		}
	case dTriMeshClass:{
		CreateSurface( body->_odeMeshData,collType,mass );
		break;
	   }
	default:
		Error( "TODO" );
	}
}

COdeBody::~COdeBody(){
	if( _odeMeshData ) _odeMeshData->Release();
	if( _odeBody ) dBodyDestroy( _odeBody );
	dGeomDestroy( _odeGeom );
}

void COdeBody::SetEnabled( bool enabled ){
	if( enabled ){
		if( _odeBody ) dBodyEnable( _odeBody );
		dGeomEnable( _odeGeom );
	}else{
		if( _odeBody ) dBodyDisable( _odeBody );
		dGeomDisable( _odeGeom );
	}
}

COdeBody *COdeBody::Create( dGeomID geom,int collType,float mass ){
	_odeGeom=geom;
	_collType=collType;
	_mass=mass;
	dGeomSetData( _odeGeom,this );
	dGeomSetCategoryBits( _odeGeom,1<<collType );
	dGeomSetCollideBits( _odeGeom,OdePhysics()->CollideBits( collType ) );

	if( mass ){
		_odeBody=dBodyCreate( OdePhysics()->OdeWorld() );

		if( _mass==-1 ){
			dBodySetGravityMode( _odeBody,0 );
			dBodySetMaxAngularSpeed( _odeBody,0 );
		}else if( mass==-2 ){
			dBodySetKinematic( _odeBody );
		}else{
//			dBodySetAutoDisableFlag( _odeBody,1 );
		}

		dGeomSetBody( _odeGeom,_odeBody );

		switch( dGeomGetClass( _odeGeom ) ){
		case dCapsuleClass:
		case dCylinderClass:{
			//PitchMatrix( -HALFPI ), 'coz ODE capsule lie on the Z axis
			static const float r[]={ 1,0,0,0, 0,0,1,0, 0,-1,0,0 };
			dGeomSetOffsetRotation( _odeGeom,r );
			break;
			}
		}

	}else{
		_odeBody=0;
		switch( dGeomGetClass( _odeGeom ) ){
		case dCapsuleClass:
		case dCylinderClass:
			Error( "TODO!" );
		}
	}
	return this;
}

COdeBody *COdeBody::CreateSphere( float radius,int collType,float mass ){
	return Create( dCreateSphere( OdePhysics()->OdeSpace(),radius ),collType,mass );
}

COdeBody *COdeBody::CreateCapsule( float radius,float length,int collType,float mass ){
	return Create( dCreateCapsule( OdePhysics()->OdeSpace(),radius,length ),collType,mass );
}

COdeBody *COdeBody::CreateCylinder( float radius,float length,int collType,float mass ){
	return Create( dCreateCylinder( OdePhysics()->OdeSpace(),radius,length ),collType,mass );
}

COdeBody *COdeBody::CreateBox( float width,float height,float depth,int collType,float mass ){
	return Create( dCreateBox( OdePhysics()->OdeSpace(),width,height,depth ),collType,mass );
}

COdeBody *COdeBody::CreateSurface( CModelSurface *surface,int collType,float mass ){
	const void *verts=&surface->Vertices()[0];
	int n_verts=surface->Vertices().size();
	int verts_pitch=sizeof(CVertex);
	const void *tris=&surface->Triangles()[0];
	int n_tris=surface->Triangles().size();
	int tris_pitch=sizeof( CTriangle );
	_odeMeshData=new COdeMeshData( verts,n_verts,verts_pitch,tris,n_tris,tris_pitch );
	return Create( dCreateTriMesh( OdePhysics()->OdeSpace(),_odeMeshData->TriMeshData(),0,0,0 ),collType,mass );
}

COdeBody *COdeBody::CreateSurface( COdeMeshData *meshData,int collType,float mass ){
	meshData->Retain();
	_odeMeshData=meshData;
	return Create( dCreateTriMesh( OdePhysics()->OdeSpace(),_odeMeshData->TriMeshData(),0,0,0 ),collType,mass );
}

void COdeBody::SetMatrix( const CMat4 &mat ){
	float r[]={
		mat.i.x,mat.j.x,mat.k.x,0,
		mat.i.y,mat.j.y,mat.k.y,0,
		mat.i.z,mat.j.z,mat.k.z,0
	};
	if( _odeBody ){
		dBodySetPosition( _odeBody,mat.t.x,mat.t.y,mat.t.z );
		dBodySetRotation( _odeBody,r );
		dBodySetLinearVel( _odeBody,0,0,0 );
		dBodySetAngularVel( _odeBody,0,0,0 );
	}else{
		dGeomSetPosition( _odeGeom,mat.t.x,mat.t.y,mat.t.z );
		dGeomSetRotation( _odeGeom,r );
	}
}

void COdeBody::SetVelocities( const CMat4 &mat ){
	CMat4 deltaMat=-Matrix() * mat;
	CVec3 dv=deltaMat.Translation();
	CQuat dq=deltaMat.Rotation();
	dBodySetLinearVel( _odeBody,dv.x,dv.y,dv.z );
	dBodySetAngularVel( _odeBody,dq.v.x,dq.v.y,dq.v.z );
}

CMat4 COdeBody::Matrix(){
	float *r,*t;
	if( _odeBody ){
		r=(float*)dBodyGetRotation( _odeBody );
		t=(float*)dBodyGetPosition( _odeBody );
	}else{
		r=(float*)dGeomGetRotation( _odeGeom );
		t=(float*)dGeomGetPosition( _odeGeom );
	}
	CMat4 mat;
	mat[0][0]=r[0];mat[1][0]=r[1];mat[2][0]=r[2];
	mat[0][1]=r[4];mat[1][1]=r[5];mat[2][1]=r[6];
	mat[0][2]=r[8];mat[1][2]=r[9];mat[2][2]=r[10];
	mat[3][0]=t[0];mat[3][1]=t[1];mat[3][2]=t[2];
	return mat;
}

void COdeBody::Validate( const CMat4 &matrix,bool modified ){
	if( _reset ){
		SetMatrix( matrix );
		_reset=false;
	}else if( modified ){
		if( _mass==-2 ){
			SetVelocities( matrix );
		}else{
			SetMatrix( matrix );
		}
	}
}

CMat4 COdeBody::Animate(){
	if( _mass<0 ){
		dBodySetLinearVel( _odeBody,0,0,0 );
	}
	return Matrix();
}

COdeJoint::COdeJoint():_odeJoint(0),_body1(0),_body2(0){
}

COdeJoint::COdeJoint( COdeJoint *joint,CCopier *copier ){
	switch( dJointGetType( joint->_odeJoint ) ){
	case dJointTypeBall:
		_odeJoint=dJointCreateBall( OdePhysics()->OdeWorld(),0 );
		break;
	default:
		Error( "TODO" );
	}
	_body1=joint->_body1 ? (COdeBody*)copier->Copy( joint->_body1 ) : 0;
	_body2=joint->_body2 ? (COdeBody*)copier->Copy( joint->_body2 ) : 0;
	Attach( _body1,_body2 );
}

COdeJoint::~COdeJoint(){
	if( _body1 ) _body1->Release();
	if( _body2 ) _body2->Release();
}

COdeJoint *COdeJoint::Create( dJointID joint ){
	_odeJoint=joint;
	return this;
}

COdeJoint *COdeJoint::CreateBall(){
	return Create( dJointCreateBall( OdePhysics()->OdeWorld(),0 ) );
}

void COdeJoint::SetMatrix( const CMat4 &mat ){
	switch( dJointGetType( _odeJoint ) ){
	case dJointTypeBall:
		dJointSetBallAnchor( _odeJoint,mat[3][0],mat[3][1],mat[3][2] );
		break;
	case dJointTypeHinge:
		dJointSetHingeAxis( _odeJoint,mat[1][0],mat[1][1],mat[1][2] );
		dJointSetHingeAnchor( _odeJoint,mat[3][0],mat[3][1],mat[3][2] );
		break;
	case dJointTypeHinge2:
		dJointSetHinge2Axis1( _odeJoint,mat[1][0],mat[1][1],mat[1][2] );
		dJointSetHinge2Axis2( _odeJoint,mat[0][0],mat[0][1],mat[0][2] );
		dJointSetHinge2Anchor( _odeJoint,mat[3][0],mat[3][1],mat[3][2] );
		break;
	default:
		Error( "TODO" );
	}
}

CMat4 COdeJoint::Matrix(){
	CMat4 mat;
	switch( dJointGetType( _odeJoint ) ){
	case dJointTypeBall:{
		dVector3 anchor,anchor2;
		dJointGetBallAnchor( _odeJoint,anchor );
		dJointGetBallAnchor2( _odeJoint,anchor2 );
		mat[3][0]=(anchor[0]+anchor2[0])/2;
		mat[3][1]=(anchor[1]+anchor2[1])/2;
		mat[3][2]=(anchor[2]+anchor2[2])/2;
		break;
		}
	case dJointTypeHinge:{
		dVector3 axis,anchor,anchor2;
		dJointGetHingeAxis( _odeJoint,axis );
		dJointGetHingeAnchor( _odeJoint,anchor );
		dJointGetHingeAnchor2( _odeJoint,anchor2 );
		//do something with axis?!?
		mat[3][0]=(anchor[0]+anchor2[0])/2;
		mat[3][1]=(anchor[1]+anchor2[1])/2;
		mat[3][2]=(anchor[2]+anchor2[2])/2;
		break;
		}
	case dJointTypeHinge2:{
		dVector3 axis1,axis2,anchor,anchor2;
		dJointGetHinge2Axis1( _odeJoint,axis1 );
		dJointGetHinge2Axis2( _odeJoint,axis2 );
		dJointGetHinge2Anchor( _odeJoint,anchor );
		dJointGetHinge2Anchor2( _odeJoint,anchor2 );
		mat[3][0]=(anchor[0]+anchor2[0])/2;
		mat[3][1]=(anchor[1]+anchor2[1])/2;
		mat[3][2]=(anchor[2]+anchor2[2])/2;
		break;
		}
	default:
		Error( "TODO" );
	}
	return mat;
}

void COdeJoint::Attach( CBody *tbody1,CBody *tbody2 ){
	COdeBody *body1=(COdeBody*)tbody1;
	COdeBody *body2=(COdeBody*)tbody2;
	if( body1 ) body1->Retain();
	if( body2 ) body2->Retain();
	if( _body1 ) _body1->Release();
	if( _body2 ) _body2->Release();
	_body1=body1;
	_body2=body2;
	dJointAttach( _odeJoint,body1 ? body1->_odeBody : 0,body2 ? body2->_odeBody : 0 );
}

void COdeJoint::Validate( const CMat4 &matrix,bool modified ){
	if( modified ) SetMatrix( matrix );
}

CMat4 COdeJoint::Animate(){
	return Matrix();
}

COdePhysics::COdePhysics(){
	_odeWorld=dWorldCreate();
	_odeSpace=dSimpleSpaceCreate( 0 );
	memset( _collideBits,0,sizeof( _collideBits ) );
	memset( _surfaces,0,sizeof( _surfaces ) );
}

COdePhysics::~COdePhysics(){
	dSpaceDestroy( _odeSpace );
	dWorldDestroy( _odeWorld );
}

void COdePhysics::SetGravity( const CVec3 &g ){
	CVec3 v=g/60.0f/60.0f;
	dWorldSetGravity( _odeWorld,v.x,v.y,v.z );
}

void COdePhysics::Update(){
	dSpaceCollide( _odeSpace,this,::OdeCallback );
	dWorldQuickStep( _odeWorld,1.0f );///60.0f );
	dJointGroupEmpty( odeContactGroup );
}

void COdePhysics::EnableCollisions( int collType1,int collType2,float friction,float bounciness,float stiffness ){
	_collideBits[collType1]|=(1<<collType2);
	_collideBits[collType2]|=(1<<collType1);
	
	dSurfaceParameters *surf=&_surfaces[collType1*32+collType2];
	
	memset( surf,0,sizeof(*surf) );

	surf->mu=friction;

	if( bounciness ){
		surf->mode|=dContactBounce;
		surf->bounce=bounciness;
	}
	if( stiffness ){
		surf->mode|=dContactSoftERP;//|dContactSoftCFM;
		surf->soft_erp=1;
//		surf->soft_cfm=0;
	}

	_surfaces[collType2*32+collType1]=*surf;
}

COdeBody *COdePhysics::CreateSphereBody( float radius,int collType,float mass ){
	return (new COdeBody)->CreateSphere( radius,collType,mass );
}

COdeBody *COdePhysics::CreateCapsuleBody( float radius,float length,int collType,float mass ){
	return (new COdeBody)->CreateCapsule( radius,length,collType,mass );
}

COdeBody *COdePhysics::CreateCylinderBody( float radius,float length,int collType,float mass ){
	return (new COdeBody)->CreateCylinder( radius,length,collType,mass );
}

COdeBody *COdePhysics::CreateBoxBody( float width,float height,float depth,int collType,float mass ){
	return (new COdeBody)->CreateBox( width,height,depth,collType,mass );
}

COdeBody *COdePhysics::CreateSurfaceBody( CModelSurface*surface,int collType,float mass ){
	return (new COdeBody)->CreateSurface( surface,collType,mass );
}

COdeJoint *COdePhysics::CreateBallJoint(){
	return (new COdeJoint)->CreateBall();
}

void odephysics_init(){
	dInitODE();
	odeContactGroup=dJointGroupCreate( 0 );
	for( int i=0;i<ODE_MAX_CONTACTS;++i ){
		odeContacts[i].surface.mu=0;//dInfinity;
	}
}
