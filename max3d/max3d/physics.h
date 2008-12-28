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

#include "resource.h"

//would prefer a different rep here...
class CModelSurface;

class CBody : public CResource{
public:
	virtual void SetEnabled( bool enabled )=0;
	
	virtual void Validate( const CMat4 &matrix,bool modified )=0;
	virtual CMat4 Animate()=0;
	
	int CollType(){ return _collType; }
	float Mass(){ return _mass; }
	
protected:	
	int _collType;
	float _mass;
};

class CJoint : public CResource{
public:
	virtual void Attach( CBody *body1,CBody *body2 )=0;
	
	virtual void Validate( const CMat4 &matrix,bool modified )=0;
	virtual CMat4 Animate()=0;
};

class CPhysics : public CResource{
public:
	virtual CBody *CreateSphereBody( float radius,int collType,float mass )=0;
	virtual CBody *CreateCapsuleBody( float radius,float length,int collType,float mass )=0;
	virtual CBody *CreateCylinderBody( float radius,float length,int collType,float mass )=0;
	virtual CBody *CreateBoxBody( float width,float height,float depth,int collType,float mass )=0;
	virtual CBody *CreateSurfaceBody( CModelSurface *surface,int collType,float mass )=0;

	virtual CJoint *CreateBallJoint()=0;
	
	virtual void SetGravity( const CVec3 &gravity )=0;
	virtual void EnableCollisions( int colltype1,int collType2,float friction,float bounciness,float stiffness )=0;
	virtual void Update()=0;
};

#endif
