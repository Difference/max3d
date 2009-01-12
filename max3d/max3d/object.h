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

#ifndef OBJECT_H
#define OBJECT_H

class CCopier;
class CStream;
class CObject;

//Object creator function
typedef CObject *(*CObjectCtor)();

/// Base object class - copy ability
class CObject{
public:
	CObject();
	virtual ~CObject();

	virtual CObject *OnCopy( CCopier *copier );
	
	virtual void OnRead( CStream *stream );
	virtual void OnWrite( CStream *stream );
	
	void SetImportPath( string path );
	string ImportPath(){ return _importPath; }
	
	CObject *Copy();
	void Write( string path );
	static CObject *Read( string path );

	string TypeName();
	
	static CObject *Create( string typeName );

	static void RegisterType( string typeName,const type_info &info,CObjectCtor ctor );
	
private:
	string _importPath;
};

/// Object copier
class CCopier{
public:
	CObject *Copy( CObject *object );

private:
	map<CObject*,CObject*> _copies;
};

class CStream{
public:
	CStream();
	~CStream();
	
	bool Open( string path,int mode );
	void Close();
	
	bool Ok();

	int ReadInt();
	float ReadFloat();
	string ReadString();
	CObject *ReadObject();
	void ReadData( void *data,int bytes );
	
	void WriteInt( int n );
	void WriteFloat( float n );
	void WriteString( string t );
	void WriteObject( CObject *obj );
	void WriteData( const void *data,int bytes );
	
private:
	FILE *_fp;
	int _objId;
	map<int,CObject*> _idMap;
	map<CObject*,int> _objMap;
};

//Ugly but fun!
#define REGISTERTYPE( X ) static struct X ## Type__{\
	X ## Type__(){ CObject::RegisterType( # X,typeid( X ),Create ); }\
	static CObject *Create(){ return new X; }\
}X ## TypeInst__;

#endif
