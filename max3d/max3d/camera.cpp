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
#include "camera.h"

CCamera::CCamera(){
	SetViewport( CRect( 0,0,App.Graphics()->WindowWidth(),App.Graphics()->WindowHeight() ) );
	float aspect=float( App.Graphics()->WindowWidth() )/float( App.Graphics()->WindowHeight() );
	SetProjectionMatrix( CMat4::PerspectiveMatrix( PI/2,aspect,.15f,256.0f ) );
}

CCamera::CCamera( CCamera *camera,CCopier *copier ):
CEntity( camera,copier ){
	SetViewport( camera->Viewport() );
	SetProjectionMatrix( camera->ProjectionMatrix() );
}

void CCamera::SetViewport( const CRect &viewport ){
	_viewport=viewport;
}

void CCamera::SetProjectionMatrix( const CMat4 &tmat ){
	_projection=tmat;
	
	CMat4 mat=tmat.Transpose();
	CVec4 l=(mat[3]+mat[0]);
	CVec4 r=(mat[3]-mat[0]);
	CVec4 b=(mat[3]+mat[1]);
	CVec4 t=(mat[3]-mat[1]);
	CVec4 n=(mat[3]+mat[2]);
	CVec4 f=(mat[3]-mat[2]);

	_frustum.planes.clear();
	_frustum.planes.push_back( CPlane( l.xyz(),l.w ).Normalize() );
	_frustum.planes.push_back( CPlane( r.xyz(),r.w ).Normalize() );
	_frustum.planes.push_back( CPlane( b.xyz(),b.w ).Normalize() );
	_frustum.planes.push_back( CPlane( t.xyz(),t.w ).Normalize() );
	_frustum.planes.push_back( CPlane( n.xyz(),n.w ).Normalize() );
	_frustum.planes.push_back( CPlane( f.xyz(),f.w ).Normalize() );
	
//	for( int i=0;i<6;++i ){
//		cout<<_frustum.planes[i]<<endl;
//	}
}

void CCamera::OnRenderWorld(){
	_renderFrustum=RenderMatrix() * Frustum();
	App.Scene()->AddCamera( this );
}

REGISTERTYPE( CCamera )
