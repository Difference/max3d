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

#include "object.h"

//maps type names to ctors
static map<string,CObjectCtor> *_ctorMap;

//maps typeid names to plain typenames
static map<string,string> *_typeidNameMap;

CObject::CObject(){
}

CObject::~CObject(){
}

CObject *CObject::OnCopy( CCopier *copier ){
	const type_info &info=typeid( *this );
	Error( "Object of type "+string( info.name() )+" cannot be copied" );
	return 0;
}

void CObject::OnRead( CStream *stream ){
	const type_info &info=typeid( *this );
	Error( "Object of type "+string( info.name() )+" cannot be read" );
}

void CObject::OnWrite( CStream *stream ){
	const type_info &info=typeid( *this );
	Error( "Object of type "+string( info.name() )+" cannot be written" );
}

CObject *CObject::Copy(){
	CCopier copier;
	return copier.Copy( this );
}

void CObject::Write( string path ){
	CStream stream;
	if( !stream.Open( path,2 ) ) return;
	stream.WriteObject( this );
}

CObject *CObject::Read( string path ){
	CStream stream;
	if( !stream.Open( path,1 ) ) return 0;
	return stream.ReadObject();
}

string CObject::TypeName(){
	const type_info &info=typeid(*this);
	map<string,string>::iterator it=_typeidNameMap->find( string( info.name() ) );
	if( it==_typeidNameMap->end() ) return "";
	return it->second;
}

CObject *CObject::Create( string typeName ){
	map<string,CObjectCtor>::iterator it=_ctorMap->find( typeName );
	if( it==_ctorMap->end() ){
		Error( "Unknown typename: "+typeName );
	}
	CObjectCtor ctor=it->second;
	return ctor();
}

void CObject::RegisterType( string typeName,const type_info &info,CObjectCtor ctor ){
	if( !_ctorMap ) _ctorMap=new map<string,CObjectCtor>;
	if( !_typeidNameMap ) _typeidNameMap=new map<string,string>;
	_ctorMap->insert( make_pair( typeName,ctor ) );
	_typeidNameMap->insert( make_pair( string( info.name() ),typeName ) );
}

CObject *CCopier::Copy( CObject *obj ){
	if( !obj ) return 0;
	map<CObject*,CObject*>::iterator it=_copies.find( obj );
	if( it!=_copies.end() ) return it->second;
	CObject *copy=obj->OnCopy( this );
	_copies.insert( make_pair(obj,copy) );
	return copy;
}


CStream::CStream():_fp(0){
}

CStream::~CStream(){
	Close();
}

bool CStream::Open( string path,int mode ){
	Close();
	const char *t;
	switch( mode ){
	case 1:t="rb";break;
	case 2:t="wb";break;
	default:Error( "Invalid Open mode" );
	}

	_fp=fopen( path.c_str(),t );
	if( !_fp ) return false;
	
	_objId=0;
	return true;
}

void CStream::Close(){
	if( _fp ){
		fclose( _fp );
		_fp=0;
	}
}

int CStream::ReadInt(){
	int n;
	if( fread( &n,4,1,_fp )!=1 ){
		//TODO: handle error
	}
	return n;
}

float CStream::ReadFloat(){
	float n;
	if( fread( &n,4,1,_fp )!=1 ){
		//TODO: handle error
	}
	return n;
}

string CStream::ReadString(){
	int n;
	if( fread( &n,4,1,_fp )!=1 ){
		//TODO: handle error
	}
	char *buf=new char[n];
	if( fread( buf,n,1,_fp )!=1 ){
		//TODO: handle error
	}
	string t( buf,n );
	delete[] buf;
	return t;
}

CObject *CStream::ReadObject(){
	int id=ReadInt();
	if( !id ) return 0;
	map<int,CObject*>::iterator it=_idMap.find( id );
	if( it!=_idMap.end() ){
		return it->second;
	}
	string typeName=ReadString();
	CObject *obj=CObject::Create( typeName );
	_idMap.insert( make_pair( id,obj ) );
	obj->OnRead( this );
	return obj;
}

void CStream::ReadData( void *data,int bytes ){
	if( fread( data,bytes,1,_fp )!=1 ){
	}
}

void CStream::WriteInt( int n ){
	if( fwrite( &n,4,1,_fp )!=1 ){
	}
}

void CStream::WriteFloat( float n ){
	if( fwrite( &n,4,1,_fp )!=1 ){
	}
}

void CStream::WriteString( string t ){
	int n=t.size();
	if( fwrite( &n,4,1,_fp )!=1 ){
	}
	if( fwrite( t.data(),n,1,_fp )!=1 ){
	}
}

void CStream::WriteObject( CObject *obj ){
	if( !obj ){
		WriteInt( 0 );
		return;
	}
	map<CObject*,int>::iterator it=_objMap.find( obj );
	if( it!=_objMap.end() ){
		WriteInt( it->second );
		return;
	}
	++_objId;
	_objMap.insert( make_pair(obj,_objId) );
	WriteInt( _objId );
	WriteString( obj->TypeName() );
	obj->OnWrite( this );
}

void CStream::WriteData( const void *data,int bytes ){
	if( fwrite( data,bytes,1,_fp )!=1 ){
	}
}
