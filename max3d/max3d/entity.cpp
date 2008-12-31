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
#include "entity.h"

//construction
CEntity::CEntity():
_parent(0),_head(0),_tail(0),_succ(0),_pred(0),
_scale(1),
_body(0),
_joint(0),
_animator(0),
_flags(0){
}

CEntity::CEntity( CEntity *entity,CCopier *copier ):
_parent(0),_head(0),_tail(0),_succ(0),_pred(0),
_scale(1),
_body(0),
_joint(0),
_animator(0),
_flags(0){
	SetTRS( entity->Translation(),entity->Rotation(),entity->Scale() );
	
	if( entity->_body ) SetBody( (CBody*)copier->Copy( entity->_body ) );
	if( entity->_joint ) SetJoint( (CJoint*)copier->Copy( entity->_joint ) );
	if( entity->_animator ) SetAnimator( (CAnimator*)copier->Copy( entity->_animator ) );

	for( CEntity *child=entity->Children();child;child=child->Next() ){
		CEntity *child_copy=(CEntity*)copier->Copy( child );
		child_copy->SetParent( this );
	}
}

void CEntity::OnRenderWorld(){
}

CEntity::~CEntity(){
	while( Children() ) delete Children();
	SetVisible( false );
	SetBody( 0 );
	SetJoint( 0 );
	SetAnimator( 0 );
	SetParent( 0 );
}

//hierarchy ops
void CEntity::SetVisible( bool visible ){
	if( visible ){
		if( !Visible() ){
			_flags|=VISIBLE;
			App.World()->AddEntity( this );
		}
	}else{
		if( Visible() ){
			_flags&=~VISIBLE;
			App.World()->RemoveEntity( this );
		}
	}
	for( CEntity *child=Children();child;child=child->Next() ){
		child->SetVisible( visible );
	}
}

void CEntity::SetParent( CEntity *parent ){
	ValidateTRS();
	if( _parent ){
		if( _succ ) _succ->_pred=_pred; else _parent->_tail=_pred;
		if( _pred ) _pred->_succ=_succ; else _parent->_head=_succ;
		_succ=_pred=0;
	}
	_parent=parent;
	if( _parent ){
		if( _pred=_parent->_tail ) _pred->_succ=this; else _parent->_head=this;
		_parent->_tail=this;
	}
	InvalidateMatrix();
}

//internal state management
void CEntity::ValidateTRS(){
	if( _flags & TRS_DIRTY ){
		if( _flags & MATRIX_DIRTY ) Error( "CEntity::ValidateTRS()" );
		CMat4 matrix=_parent ? -_parent->Matrix() * _matrix : _matrix;
		_trans=matrix.Translation();
		_rot=matrix.Rotation();
		_scale=matrix.Scale();
		_flags&=~TRS_DIRTY;
	}
}

void CEntity::ValidateMatrix(){
	if( _flags & MATRIX_DIRTY ){
		if( _flags & TRS_DIRTY ) Error( "CEntity::ValidateMatrix()" );
		_matrix=CMat4::TranslationMatrix( _trans ) * CMat4::RotationMatrix( _rot ) * CMat4::ScaleMatrix( _scale );
		if( _parent ) _matrix=_parent->Matrix() * _matrix;
		_flags&=~MATRIX_DIRTY;
	}
}

void CEntity::InvalidateMatrix(){
	if( _flags & (MATRIX_DIRTY|MATRIX_LOCKED) ) return;
	ValidateTRS();
	_flags|=MATRIX_DIRTY|MATRIX_MODIFIED;
	for( CEntity *child=Children();child;child=child->Next() ){
		child->InvalidateMatrix();
	}
}

//local space
void CEntity::SetTRS( const CVec3 &v,const CQuat &q,const CVec3 &s ){
	_trans=v;
	_rot=q;
	_scale=s;
	InvalidateMatrix();
}

void CEntity::GetTRS( CVec3 &v,CQuat &q,CVec3 &s ){
	ValidateTRS();
	v=_trans;
	q=_rot;
	s=_scale;
}

void CEntity::SetTranslation( const CVec3 &v ){
	ValidateTRS();
	_trans=v;
	InvalidateMatrix();
}

CVec3 CEntity::Translation(){
	ValidateTRS();
	return _trans;
}

void CEntity::SetRotation( const CQuat &q ){
	ValidateTRS();
	_rot=q;
	InvalidateMatrix();
}

CQuat CEntity::Rotation(){
	ValidateTRS();
	return _rot;
}

void CEntity::SetScale( const CVec3 &v ){
	ValidateTRS();
	_scale=v;
	InvalidateMatrix();
}

CVec3 CEntity::Scale(){
	ValidateTRS();
	return _scale;
}

//world space
void CEntity::SetMatrix( const CMat4 &m ){
	_matrix=m;
	_flags&=~MATRIX_DIRTY;
	_flags|=TRS_DIRTY|MATRIX_MODIFIED;
	for( CEntity *child=Children();child;child=child->Next() ){
		child->InvalidateMatrix();
	}
}

CMat4 CEntity::Matrix(){
	ValidateMatrix();
	return _matrix;
}

CMat4 CEntity::InverseMatrix(){
	return -Matrix();
}

CMat4 CEntity::RenderMatrix(){
	return Matrix();
}

CMat4 CEntity::InverseRenderMatrix(){
	return -RenderMatrix();
}

//physics
void CEntity::SetBody( CBody *body ){
	if( body ) body->Retain();
	if( _body ) _body->Release();
	_body=body;
}

void CEntity::SetJoint( CJoint *joint ){
	if( joint ) joint->Retain();
	if( _joint ) _joint->Release();
	_joint=joint;
}

void CEntity::SetAnimator( CAnimator *animator ){
	if( animator ) animator->Retain();
	if( _animator ) _animator->Release();
	_animator=animator;
}

//utility
void CEntity::Move( const CVec3 &v ){
	SetTranslation( Translation() + CMat4::RotationMatrix( Rotation() ) * v );
//	SetTranslation( Translation()+Rotation()*v );	//doesn't quite work - check out CQuat*CVec3
}

void CEntity::Turn( const CQuat &q ){
	SetRotation( Rotation() * q );
}

void CEntity::SetMatrixModified( bool modified ){
	if( modified ){
		_flags|=MATRIX_MODIFIED;
	}else{
		_flags&=~MATRIX_MODIFIED;
	}
}

bool CEntity::MatrixModified(){
	return _flags & MATRIX_MODIFIED;
}

void CEntity::SetMatrixLocked( bool locked ){
	if( locked ){
		_flags|=MATRIX_LOCKED;
	}else{
		_flags&=~MATRIX_LOCKED;
	}
}

bool CEntity::MatrixLocked(){
	return _flags & MATRIX_LOCKED;
}

REGISTERTYPE( CEntity );
