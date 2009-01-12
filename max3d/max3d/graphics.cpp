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

#include "graphics.h"

CGraphics::CGraphics():
_depthBuffer(0),
_vertexBuffer(0),
_indexBuffer(0),
_shader(0),
_shaderMode(0){
	memset( _colorBuffers,0,sizeof(_colorBuffers) );
}

int CShader::ModeForName( string name ){
	static map<string,int> idMap;
	static int nextId;
	name=toupper( name );
	map<string,int>::iterator it=idMap.find( name );
	if( it!=idMap.end() ) return it->second;
	int id=nextId++;
	if( id==32 ) Error( "Out of shaderModes" );
	idMap.insert( make_pair(name,id) );
	return id;
}

CParam *CParam::ForName( string name ){
	static map<string,CParam*> _params;
	map<string,CParam*>::iterator it=_params.find( name );
	if( it!=_params.end() ) return it->second;
	CParam *p=new CParam( name );
	_params.insert( make_pair(name,p) );
	return p;
}

CParam::CParam( string name ):_name(name),_seq(0),_count(0),_floats(0),_texture(0),_floatFunc(0){
}

void CParam::Invalidate(){
	_seq=0;
	for( vector<CParam*>::iterator it=_defs.begin();it!=_defs.end();++it ){
		(*it)->Invalidate();
	}
}

void CParam::SetFloatValue( int count,const float *value ){
	if( _floatFunc ) Error( "Attempt to write read-only param '"+_name+"'" );
	if( count!=_count ){
		_count=count;
		if( _floats ) delete[] _floats;
		_floats=new float[_count];
	}
	memcpy( _floats,value,_count*4 );
	Invalidate();
}

const float *CParam::FloatValue(){
	if( _floatFunc ) memcpy( _floats,_floatFunc(),_count*4 );
	return _floats;
}

void CParam::SetTextureValue( CTexture *value ){
	CResource::Assign( &_texture,value );
	Invalidate();
}

CTexture *CParam::TextureValue(){
	return _texture;
}

void CParam::SetFloatFunc( int count,FloatParamFunc func,const vector<string> &uses ){
	if( count!=_count ){
		_count=count;
		if( _floats ) delete[] _floats;
		_floats=new float[_count];
	}
	_floatFunc=func;
	for( vector<string>::const_iterator it=uses.begin();it!=uses.end();++it ){
		ForName( *it )->_defs.push_back( this );
	}
}

FloatParamFunc CParam::FloatFunc(){
	return _floatFunc;
}
