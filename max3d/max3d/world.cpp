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

#include "GLee.h"

#include "std.h"

#include "app.h"
#include "world.h"
#include "scene.h"

#include "odephysics.h"

CWorld::CWorld(){

	_clear=CVec3( 0.0f,0.75f,1.0f );
	_ambient=CVec3( .25f,.25f,.25f );

	_physics=new COdePhysics;

	_physics->SetGravity( CVec3( 0,-9.81f,0 ) );
}

void CWorld::SetClearColor( const CVec3 &color ){
	_clear=color;
}

void CWorld::SetAmbientColor( const CVec3 &color ){
	_ambient=color;
}

bool CWorld::AddEntity( CEntity *entity ){
//	Log( "Adding "+entity->Name()+" to world" );
	if( _entities.insert( entity ).second ){
		if( CBody *body=entity->Body() ) body->SetEnabled( true );
		return true;
	}
	return false;
}

bool CWorld::RemoveEntity( CEntity *entity ){
//	Log( "Removing "+entity->Name()+" from world" );
	if( _entities.erase( entity ) ){
		if( CBody *body=entity->Body() ) body->SetEnabled( false );
		return true;
	}
	return false;
}

void CWorld::Update(){
	set<CEntity*>::iterator it;
	
	//lock bodies/joints
	for( it=_entities.begin();it!=_entities.end();++it ){
		CEntity *entity=*it;
		if( entity->Body() || entity->Joint() ){
			entity->SetMatrixLocked( true );
		}
	}
	
	//validate bodies
	for( it=_entities.begin();it!=_entities.end();++it ){
		CEntity *entity=*it;
		if( CBody *body=entity->Body() ){
			body->Validate( entity->WorldMatrix(),entity->MatrixModified() );
		}
	}

	//validate joints
	for( it=_entities.begin();it!=_entities.end();++it ){
		CEntity *entity=*it;
		if( CJoint *joint=entity->Joint() ){
			joint->Validate( entity->WorldMatrix(),entity->MatrixModified() );
		}
	}
	
	//Run physics
	_physics->Update();

	//animate everything
	for( it=_entities.begin();it!=_entities.end();++it ){
		CEntity *entity=*it;
		if( CBody *body=entity->Body() ){
			entity->SetWorldMatrix( body->Animate() );
		}else if( CJoint *joint=entity->Joint() ){
			entity->SetWorldMatrix( joint->Animate() );
		}
	}

	//clear matrix modified flag for everything
	for( it=_entities.begin();it!=_entities.end();++it ){
		CEntity *entity=*it;
		entity->SetMatrixLocked( false );
		entity->SetMatrixModified( false );
	}
}

void CWorld::Render(){

	App.Scene()->Clear();

	for( set<CEntity*>::iterator it=_entities.begin();it!=_entities.end();++it ){
		CEntity *entity=*it;
		entity->OnRenderWorld();
	}
	
	App.Graphics()->BeginScene();
	
	App.Graphics()->SetVec4Param( "bb_ClearColor",CVec4( ClearColor(),1.0f ) );
	
	App.Graphics()->SetVec3Param( "bb_AmbientColor",AmbientColor() );
	
	for( vector<CCamera*>::const_iterator it=App.Scene()->Cameras().begin();it!=App.Scene()->Cameras().end();++it ){
		CCamera *camera=*it;
		App.Scene()->RenderCamera( camera );
	}
	
	App.Graphics()->EndScene();
	
	for( vector<CSurface*>::const_iterator it=App.Scene()->Surfaces().begin();it!=App.Scene()->Surfaces().end();++it ){
		CSurface *surface=*it;
		surface->OnClearInstances();
	}

	App.Scene()->Clear();
}
