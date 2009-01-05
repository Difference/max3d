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
#include "light.h"

CLight::CLight():
_angle(HALFPI),
_range(100),
_color(CVec3(1)),
_shader(0),
_texture(0),
_shadowSize(2048){
	_shadowSplits.push_back( 1 );
	_shadowSplits.push_back( 4 );
	_shadowSplits.push_back( 16 );
	_shadowSplits.push_back( 64 );
	_shadowSplits.push_back( 256 );
}

CLight::CLight( CLight *light,CCopier *copier ):
CEntity( light,copier ),
_angle( light->_angle ),
_range( light->_range ),
_color( light->_color ),
_shader( light->_shader ),
_texture( light->_texture ),
_shadowSize( light->_shadowSize ),
_shadowSplits( light->_shadowSplits ){
	if( _shader ) _shader->Retain();
	if( _texture ) _texture->Retain();
}

CLight::~CLight(){
	if( _shader ) _shader->Release();
	if( _texture ) _texture->Release();
}

void CLight::SetAngle( float angle ){
	_angle=angle;
}

void CLight::SetRange( float range ){
	_range=range;
}

void CLight::SetColor( const CVec3 &color ){
	_color=color;
}

void CLight::SetShader( CShader *shader ){
	if( shader ) shader->Retain();
	if( _shader ) _shader->Release();
	_shader=shader;
}

void CLight::SetTexture( CTexture *texture ){
	if( texture ) texture->Retain();
	if( _texture ) _texture->Release();
	_texture=texture;
}

void CLight::SetShadowSize( int size ){
	_shadowSize=size;
}

void CLight::SetShadowSplits( const vector<float> &splits ){
	_shadowSplits=splits;
}

void CLight::OnRenderWorld(){
	App.Scene()->AddLight( this );
}

vector<float> CLight::ComputeShadowSplits( int count,float znear,float zfar,float blend ){
	vector<float> splits( count,0.0f );
	splits[0]=znear;
	for( int i=1;i<count-1;++i ){
		float t=i/float(count);
		float log=znear * powf(zfar/znear,t);
		float uniform=znear + (zfar-znear) * t;
		splits[i]=log * (1-blend) + uniform * blend;
	}
	splits[count-1]=zfar;
	return splits;
}

REGISTERTYPE( CLight )
