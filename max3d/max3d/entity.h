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

#ifndef ENTITY_H
#define ENTITY_H

#include "physics.h"
#include "animator.h"

class CCamera;

class CEntity : public CObject{
public:
	//construction
	CEntity();
	~CEntity();
	
	//Rendering
	virtual void OnRenderWorld();
	
	//hierarchy ops
	void SetVisible( bool visible );
	bool Visible(){ return !!(_flags & VISIBLE); }
	
	void SetParent( CEntity *parent );
	CEntity *Parent(){ return _parent; }
	CEntity *Children(){ return _head; }
	CEntity *Next(){ return _succ; }

	//local space
	void SetTRS( const CVec3 &v,const CQuat &q,const CVec3 &s );
	void GetTRS( CVec3 &v,CQuat &q,CVec3 &s );

	void SetTranslation( const CVec3 &v );
	CVec3 Translation();

	void SetRotation( const CQuat &q );
	CQuat Rotation();

	void SetScale( const CVec3 &v );
	CVec3 Scale();
	
	//world space
	void SetMatrix( const CMat4 &m );
	CMat4 Matrix();
	CMat4 InverseMatrix();
	CMat4 RenderMatrix();
	CMat4 InverseRenderMatrix();

	//physics
	void SetBody( CBody *body );
	CBody *Body(){ return _body; }
	
	void SetJoint( CJoint *joint );
	CJoint *Joint(){ return _joint; }

	void SetAnimator( CAnimator *animator );
	CAnimator *Animator(){ return _animator; }
	
	//utility
	void Move( const CVec3 &v );
	void Turn( const CQuat &q );

	//scary stuff for bodies...
	void SetMatrixModified( bool modified );
	bool MatrixModified();

	void SetMatrixLocked( bool locked );
	bool MatrixLocked();
	
protected:
	CEntity( CEntity *entity,CCopier *copier );
	
private:
	enum{
		TRS_DIRTY=1,
		MATRIX_DIRTY=2,
		VALIDATE=4,
		ANIMATE=8,
		MATRIX_MODIFIED=16,
		MATRIX_LOCKED=32,
		VISIBLE=64
	};

	void ValidateTRS();
	void ValidateMatrix();
	void InvalidateMatrix();
	
	int _flags;

	CEntity *_parent;
	CEntity *_head;
	CEntity *_tail;
	CEntity *_succ;
	CEntity *_pred;

	CVec3 _trans;
	CQuat _rot;
	CVec3 _scale;
	CMat4 _matrix;
	
	CBody *_body;
	CJoint *_joint;
	CAnimator *_animator;
};

#endif
