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
#include "material.h"

CMaterial::CMaterial(){
	SetColor( "DiffuseColor",CVec3(1) );
	SetTexture( "DiffuseMap",App.TextureUtil()->WhiteTexture() );
	SetTexture( "SpecularMap",App.TextureUtil()->BlackTexture() );
	SetTexture( "EmissiveMap",App.TextureUtil()->BlackTexture() );
	SetTexture( "NormalMap",App.TextureUtil()->FlatTexture() );
}

CMaterial::~CMaterial(){
	for( map<CParam*,CTexture*>::iterator it=_textures.begin();it!=_textures.end();++it ){
		it->second->Release();
	}
}

void CMaterial::SetFloat( string name,float value ){
	_floats[CParam::ForName(name)]=value;
}

float CMaterial::Float( string name ){
	map<CParam*,float>::iterator it=_floats.find(CParam::ForName(name));
	return it==_floats.end() ? 0 : it->second;
}

void CMaterial::SetColor( string name,const CVec3 &value ){
	_colors[CParam::ForName(name)]=value;
}

CVec3 CMaterial::Color( string name ){
	map<CParam*,CVec3>::iterator it=_colors.find(CParam::ForName(name));
	return it==_colors.end() ? CVec3( 0) : it->second;
}

void CMaterial::SetTexture( string name,CTexture *texture ){
	if( texture ){
		texture->Retain();
		if( CTexture *t=Texture( name ) ) t->Release();
		_textures[CParam::ForName(name)]=texture;
	}else{
		if( CTexture *t=Texture( name ) ) t->Release();
		_textures.erase(CParam::ForName(name));
	}
}

CTexture *CMaterial::Texture( string name ){
	map<CParam*,CTexture*>::iterator it=_textures.find(CParam::ForName(name));
	return it==_textures.end() ? 0 : it->second;
}

void CMaterial::Bind(){
	for( map<CParam*,float>::iterator it=_floats.begin();it!=_floats.end();++it ){
		it->first->SetFloatValue( 1,&it->second );
	}
	for( map<CParam*,CVec3>::iterator it=_colors.begin();it!=_colors.end();++it ){
		it->first->SetFloatValue( 3,&it->second.x );
	}
	for( map<CParam*,CTexture*>::iterator it=_textures.begin();it!=_textures.end();++it ){
		it->first->SetTextureValue( it->second );
	}
}

void CMaterial::OnRead( CStream *stream ){
	int n_floats=stream->ReadInt();
	for( int i=0;i<n_floats;++i ){
		string name=stream->ReadString();
		SetFloat( name,stream->ReadFloat() );
	}
	int n_colors=stream->ReadInt();
	for( int i=0;i<n_colors;++i ){
		string name=stream->ReadString();
		SetColor( name,CVec3::Read( stream ) );
	}
	int n_textures=stream->ReadInt();
	for( int i=0;i<n_textures;++i ){
		string name=stream->ReadString();
//		SetTexture( name,App.TextureUtil()->ReadTexture( stream ) );
	}
}

void CMaterial::OnWrite( CStream *stream ){
	stream->WriteInt( _floats.size() );
	for( map<CParam*,float>::iterator it=_floats.begin();it!=_floats.end();++it ){
		stream->WriteString( it->first->Name() );
		stream->WriteFloat( it->second );
	}
	stream->WriteInt( _colors.size() );
	for( map<CParam*,CVec3>::iterator it=_colors.begin();it!=_colors.end();++it ){
		stream->WriteString( it->first->Name() );
		it->second.Write( stream );
	}
	stream->WriteInt( _textures.size() );
	for( map<CParam*,CTexture*>::iterator it=_textures.begin();it!=_textures.end();++it ){
		stream->WriteString( it->first->Name() );
//		App.TextureUtil()->WriteTexture( it->second,stream );
	}
}

REGISTERTYPE( CMaterial )
