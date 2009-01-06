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

/// @file
#include "GLee.h"

#include "std.h"

#include "app.h"
#include "openglgraphics.h"

static bool gpu_instancing;

enum{
	DIRTY_SHADERPROG=1,
	DIRTY_FRAMEBUFFER=2
};

//utility stuff
static void CheckGL(){
	int err=glGetError();
	if( err==GL_NO_ERROR ) return;
	const char *str;
	switch( err ){
	case GL_INVALID_ENUM:str="INVALID ENUM";break;
	case GL_INVALID_VALUE:str="INVALID VALUE";break;
	case GL_INVALID_OPERATION:str="INVALID OPERATION";break;
	case GL_STACK_OVERFLOW:str="STACK OVERFLOW";break;
	case GL_STACK_UNDERFLOW:str="STACK UNDERFLOW";break;
	case GL_OUT_OF_MEMORY:str="OUT OF MEMORY";break;
	case GL_TABLE_TOO_LARGE:str="TABLE TOO LARGE";break;
	default:str="UNKNOWN";
	}
    Error( string("GL Error: ")+str );
}

static string ShaderInfoLog( int what ){
	char buf[1024];
	buf[0]=0;
	glGetShaderInfoLog( what,1023,0,buf );
	buf[1023]=0;
	return buf;
}

static void SplitShaderSegs( const string &source,string segs[3] ){
	istringstream in( source );
	string t;
	int seg=-1;
	while( getline( in,t ) ){
		if( t.find( "//@common" )==0 ){
			seg=0;
		}else if( t.find( "//@vertex" )==0 ){
			seg=1;
		}else if( t.find( "//@fragment" )==0 ){
			seg=2;
		}else if( seg!=-1 ){
			segs[seg]+=t+"\n";
		}
	}
}

static map<string,string> SplitShaderModes( const string &source ){
	istringstream in( source );
	map<string,string> pmap;
	vector<string> modes;
	string t;
	while( getline( in,t ) ){
		if( t.find( "//@mode " )==0 ){
			vector<string> toks;
			Tokenize( t,toks," ,\r\n" );
			modes.clear();
			for( int i=1;i<toks.size();++i ){
				modes.push_back( toks[i] );
			}
		}else{
			t+="\n";
			for( vector<string>::iterator it=modes.begin();it!=modes.end();++it ){
				pmap[*it]+=t;
			}
		}
	}
	return pmap;
}

static GLuint CompileShader( int target,const string &source ){
	char *cstr=strdup( source.c_str() );
	GLuint shader=glCreateShader( target );
	glShaderSource( shader,1,(const GLchar**)&cstr,0 );
	//free( cstr );
	glCompileShader( shader );
	GLint p;
	glGetShaderiv( shader,GL_COMPILE_STATUS,&p );
	if( !p ){
		cout<<"SHADER COMPILE ERROR"<<endl;
		istringstream in( source );
		string t;
		int n=0;
		while( getline( in,t ) ){
			cout<<++n<<':'<<t<<endl;
		}
		cout<<ShaderInfoLog( shader )<<endl;
		Error( "" );
	}
	return shader;
}

// VertexBuffer implementation

struct GLAttrib{
	int count,type,offset;
};

/// OpenGL implementation of vertex buffers
class GLVertexBuffer : public CVertexBuffer{
public:

	GLVertexBuffer():_locked(0){
	}

	GLVertexBuffer *Create( int capacity,string format ){
		_capacity=capacity;
		_format=format;
		//
		int offset=0;
		for( int i=0;i<format.size();i+=2 ){
			GLAttrib attr;
			attr.count=format[i]-'0';	//0,1,2,3,4
			if( attr.count<0 || attr.count>4 ) Error( "Invalid vertex format: "+format );
			int sz;
			switch( format[i+1] ){
			case 'b':attr.type=GL_BYTE;sz=1;break;
			case 's':attr.type=GL_SHORT;sz=2;break;
			case 'i':attr.type=GL_INT;sz=4;break;
			case 'f':attr.type=GL_FLOAT;sz=4;break;
			default:Error( "Invalid vertex format: "+format );
			}
			attr.offset=offset;
			offset+=attr.count*sz;
			_attribs.push_back( attr );
		}
		_pitch=offset;
		glGenBuffers( 1,&_glvb );
		glBindBuffer( GL_ARRAY_BUFFER,_glvb );
		glBufferData( GL_ARRAY_BUFFER,_pitch*_capacity,0,GL_DYNAMIC_DRAW );
		//
		CheckGL();
		return this;
	}
	
	void SetData( const void *data ){
		glBindBuffer( GL_ARRAY_BUFFER,_glvb );
		glBufferData( GL_ARRAY_BUFFER,_pitch*_capacity,data,GL_DYNAMIC_DRAW );
	}

	void *Lock(){
		if( !_locked++ ){
			glBindBuffer( GL_ARRAY_BUFFER,_glvb );
			_lockedp=glMapBuffer( GL_ARRAY_BUFFER,GL_READ_WRITE );
			if( !_lockedp ) Error( "Failed to lock vertex buffer" );
		}
		return _lockedp;
	}
	
	void Unlock(){
		if( !--_locked ){
			glUnmapBuffer( GL_ARRAY_BUFFER );
		}
	}
	
	void SetAttrib( int vertex,int attrib,float x,float y,float z,float w ){
		const GLAttrib &glattrib=_attribs[attrib];
		if( glattrib.type!=GL_FLOAT ) Error( "TODO" );
		void *p=(char*)_lockedp + (vertex*_pitch) + glattrib.offset;
		float v[]={x,y,z,w};
		Lock();
		memcpy( _lockedp,&v,glattrib.count*4 );
		Unlock();
	}

	void Bind(){
		glBindBuffer( GL_ARRAY_BUFFER,_glvb );
		for( int i=0;i<_attribs.size();++i ){
			GLAttrib &attr=_attribs[i];
			if( attr.count ){
				glEnableVertexAttribArray( i );
				glVertexAttribPointer( i,attr.count,attr.type,0,_pitch,(GLvoid*)attr.offset );
			}else{
				glDisableVertexAttribArray( i );
			}
		}
		for( int i=_attribs.size();i<8;++i ){
			glDisableVertexAttribArray( i );
		}
	}
private:
	int _pitch;
	vector<GLAttrib> _attribs;
	GLuint _glvb;
	int _locked;
	void *_lockedp;
};

/// OpenGL implementation of index buffers
class GLIndexBuffer : public CIndexBuffer{
public:

	GLIndexBuffer():_locked(0){
	}

	GLIndexBuffer *Create( int capacity,string format ){
		_capacity=capacity;
		_format=format;
		//
		if( format=="1s" ){
			_pitch=2;
			_gltype=GL_UNSIGNED_SHORT;
		}else if( format=="1i" ){
			_pitch=4;
			_gltype=GL_UNSIGNED_INT;
		}else{
			Error( "Invalid index buffer format" );
		}
		glGenBuffers( 1,&_glib );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,_glib );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,_pitch*_capacity,0,GL_DYNAMIC_DRAW );
		//
		CheckGL();
		return this;
	}
	
	void SetData( const void *data ){
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,_glib );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,_pitch*_capacity,data,GL_DYNAMIC_DRAW );
	}
	
	int Pitch(){
		return _pitch;
	}
	
	int GLType(){
		return _gltype;
	}
	
	void *Lock(){
		if( !_locked++ ){
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,_glib );
			_lockedp=glMapBuffer( GL_ELEMENT_ARRAY_BUFFER,GL_READ_WRITE );
			if( !_lockedp ) Error( "Failed to lock index buffer" );
		}
		return _lockedp;
	}
	
	void Unlock(){
		if( !--_locked ){
			glUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
		}
	}

	void SetIndex( int index,int value ){
		if( _format!="1i" ) Error( "TODO" );
		Lock();
		((int*)_lockedp)[index]=value;
		Unlock();
	}
		
	void Bind(){
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER,_glib );
	}
private:
	int _pitch;
	int _gltype;
	GLuint _glib;
	int _locked;
	void *_lockedp;
};

class GLFrameBuffer;

// OpenGL implementation of textures
class GLTexture : public CTexture{
public:

	int glInternalFormat(){
		switch( _format ){
		case FORMAT_A8:return GL_ALPHA8;
		case FORMAT_I8:return GL_INTENSITY8;
		case FORMAT_L8:return GL_LUMINANCE8;
		case FORMAT_LA8:return GL_LUMINANCE8_ALPHA8;
		case FORMAT_RGB8:return GL_RGB8;
		case FORMAT_RGBA8:return GL_RGBA8;
		case FORMAT_RGB10A2:return GL_RGB10_A2;
		case FORMAT_RGBA16F:return GL_RGBA16F_ARB;
		case FORMAT_DEPTH:return GL_DEPTH_COMPONENT;
		}
		Error("TODO");
		return 0;
	}

	int glDataFormat(){
		switch( _format ){
		case FORMAT_A8:return GL_ALPHA;
		case FORMAT_I8:return GL_INTENSITY;
		case FORMAT_L8:return GL_LUMINANCE;
		case FORMAT_LA8:return GL_LUMINANCE_ALPHA;
		case FORMAT_RGB8:return GL_RGB;
		case FORMAT_RGBA8:return GL_RGBA;
		case FORMAT_RGB10A2:return GL_RGBA;
		case FORMAT_RGBA16F:return GL_RGBA;
		case FORMAT_DEPTH:return GL_DEPTH_COMPONENT;
		}
		Error( "TODO" );
		return 0;
	}

	int BytesPerTexel(){
		switch( _format ){
		case FORMAT_A8:return 1;
		case FORMAT_I8:return 1;
		case FORMAT_L8:return 1;
		case FORMAT_LA8:return 2;
		case FORMAT_RGB8:return 3;
		case FORMAT_RGBA8:return 4;
		case FORMAT_RGB10A2:return 4;
		case FORMAT_RGBA16F:return 8;
		case FORMAT_DEPTH:return 0;
		}
		Error( "TODO" );
		return 0;
	}

	void GenTexture(){
		//
		glGenTextures( 1,&_gltex );
		glBindTexture( _gltarget,_gltex );
		//
		if( _flags & TEXTURE_FILTER ){
			glTexParameteri( _gltarget,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
			if( _flags & TEXTURE_MIPMAP ){
				glTexParameteri( _gltarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR );
			}else{
				glTexParameteri( _gltarget,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
			}
		}else{
			glTexParameteri( _gltarget,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
			if( _flags & TEXTURE_MIPMAP ){
				glTexParameteri( _gltarget,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST );
			}else{
				glTexParameteri( _gltarget,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
			}
		}
		if( _flags & TEXTURE_MIPMAP ){
			glTexParameteri( _gltarget,GL_GENERATE_MIPMAP,GL_TRUE );
		}
		if( _flags & TEXTURE_CLAMPS ){
			glTexParameteri( _gltarget,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE );
		}
		if( _flags & TEXTURE_CLAMPT ){
			glTexParameteri( _gltarget,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE );
		}
	}

	GLTexture *Create( int width,int height,int format,int flags ){
		//
		_width=width;
		_height=height;
		_format=format;
		_flags=flags;
		_gltarget=(flags & TEXTURE_RECTANGULAR) ? GL_TEXTURE_RECTANGLE_ARB : GL_TEXTURE_2D;
		//
		GenTexture();
		//
		glTexParameteri( _gltarget,GL_TEXTURE_COMPARE_MODE,GL_NONE );
		glTexParameteri( _gltarget,GL_TEXTURE_COMPARE_FUNC,GL_ALWAYS );
		//
		SetData( 0 );
		CheckGL();
		return this;
	}
	
	void SetData( const void *data ){
		glTexImage2D( _gltarget,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,data );
	}
	
	GLTexture *Create3d( int width,int height,int depth,int format,int flags ){
		//
		_width=width;
		_height=height;
		_depth=depth;
		_format=format;
		_flags=flags;
		_gltarget=GL_TEXTURE_3D;
		//
		GenTexture();
		//
		Set3dData( 0 );
		CheckGL();
		return this;
	}
	
	void Set3dData( const void *data ){
		glTexImage3D( _gltarget,0,glInternalFormat(),_width,_height,_depth,0,glDataFormat(),GL_UNSIGNED_BYTE,data );
	}
	
	GLTexture *CreateCube( int size,int format,int flags ){
		//
		_width=size;
		_height=size;
		_format=format;
		_flags=flags;
		_gltarget=GL_TEXTURE_CUBE_MAP;
		//
		GenTexture();
		//
		SetCubeData( 0 );
		CheckGL();
		return this;
	}
	
	void SetCubeData( const void *data  ){
		const char *p=(const char*)data;
		int sz=data ? ((_width * BytesPerTexel() + 3)& ~3) * _height : 0;
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,p+sz*0 );
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,p+sz*1 );
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,p+sz*2 );
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,p+sz*3 );
		glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,p+sz*4 );
		glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,p+sz*5 );
	}

	void Bind(){
		glBindTexture( _gltarget,_gltex );
	}
private:	
	friend class GLFrameBuffer;
	GLenum _gltarget;
	GLuint _gltex;
};

class GLFrameBuffer{
public:
	GLFrameBuffer *Create( CTexture *colorBuffers[4],CTexture *depthBuffer ){
		glGenFramebuffersEXT( 1,&_glfbo );
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,_glfbo );
		for( int i=0;i<4;++i ){
			_colorBuffers[i]=(GLTexture*)colorBuffers[i];
			if( GLTexture *tex=_colorBuffers[i] ){
				tex->Retain();
				_glbufs[i]=GL_COLOR_ATTACHMENT0_EXT+i;
				glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,_glbufs[i],tex->_gltarget,tex->_gltex,0 );
			}else{
				_glbufs[i]=GL_NONE;
			}
		}
		_depthBuffer=(GLTexture*)depthBuffer;
		if( GLTexture *tex=_depthBuffer ){
			tex->Retain();
			glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,tex->_gltarget,tex->_gltex,0 );
		}

		glReadBuffer( GL_NONE );
		glDrawBuffers( 4,_glbufs );

		int n=glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT );
		if( n!=GL_FRAMEBUFFER_COMPLETE_EXT ){
			char buf[256];
			sprintf( buf,"%x",n );
			Error( "glCheckFramebufferStatusEXT failed:"+string(buf) );
		}

		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,0 );

		CheckGL();
		return this;
	}
	
	GLTexture *ColorBuffer( int index){
		return _colorBuffers[index];
	}
	
	GLTexture *DepthBuffer(){
		return _depthBuffer;
	}
	
	bool Validate(){
		int i;
		for( i=0;i<4;++i ){
			if( _colorBuffers[i] && _colorBuffers[i]->Refs()==1 ) break;
		}
		
		if( i==4 && (!_depthBuffer || _depthBuffer->Refs()>1) ) return true;
	
		for( int i=0;i<4;++i ){
			if( _colorBuffers[i] ){
				_colorBuffers[i]->Release();
				_colorBuffers[i]=0;
			}
		}
		if( _depthBuffer ){
			_depthBuffer->Release();
			_depthBuffer=0;
		}
		return false;
	}
	
	void Bind(){
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,_glfbo );
		glDrawBuffers( 4,_glbufs );
	}

private:
	GLuint _glfbo;
	GLenum _glbufs[4];
	GLTexture *_colorBuffers[4];
	GLTexture *_depthBuffer;
};

static int nSets;
static int paramSeq;

class GLParam{
public:

	GLParam( CParam *param,int glsize,GLenum gltype,int glloc,int glunit ){
		_param=param;
		_glsize=glsize;
		_gltype=gltype;
		_glloc=glloc;
		_glunit=glunit;
	}

	void Bind(){

		if( _param->Validate( paramSeq ) ) return;

		++nSets;
		
		switch( _gltype ){
		case GL_FLOAT:
			glUniform1fv( _glloc,1,_param->FloatValue() );
			break;
		case GL_FLOAT_VEC2:
			glUniform2fv( _glloc,1,_param->FloatValue() );
			break;
		case GL_FLOAT_VEC3:
			glUniform3fv( _glloc,1,_param->FloatValue() );
			break;
		case GL_FLOAT_VEC4:
			glUniform4fv( _glloc,1,_param->FloatValue() );
			break;
		case GL_FLOAT_MAT4:
			{
			int n=_param->Count()/16;
			glUniformMatrix4fv( _glloc,n,GL_FALSE,_param->FloatValue() );
			break;
			}
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_2D_RECT_ARB:
		case GL_SAMPLER_2D_RECT_SHADOW_ARB:
			if( GLTexture *t=(GLTexture*)_param->TextureValue() ){
				glUniform1i( _glloc,_glunit );
				glActiveTexture( GL_TEXTURE0+_glunit );
				t->Bind();
			}else{
				cout<<"No texture!"<<endl;
			}
			break;
		default:
			Error( "TODO" );
		}
	}
private:
	CParam *_param;
	int _glloc;
	int _glsize;
	GLenum _gltype;
	int _glunit;
};

class GLShaderProg{
public:

	GLShaderProg():_glprog(0),_instanceIdLoc(-1){
	}

	GLShaderProg *Create( string source ){

		string segs[3];
		SplitShaderSegs( source,segs );

		string vert=segs[0]+segs[1];
		string frag=segs[0]+segs[2];

//		cout<<"vertex shader:"<<endl<<vert<<endl;
//		cout<<"fragment shader:"<<endl<<frag<<endl;

		GLuint vs=CompileShader( GL_VERTEX_SHADER,vert );
		GLuint fs=CompileShader( GL_FRAGMENT_SHADER,frag );

		_glprog=glCreateProgram();

		glAttachShader( _glprog,vs );
		glAttachShader( _glprog,fs );

		glBindAttribLocation( _glprog,0,"Attrib0" );
		glBindAttribLocation( _glprog,1,"Attrib1" );
		glBindAttribLocation( _glprog,2,"Attrib2" );
		glBindAttribLocation( _glprog,3,"Attrib3" );
		glBindAttribLocation( _glprog,4,"Attrib4" );
		glBindAttribLocation( _glprog,5,"Attrib5" );
		glBindAttribLocation( _glprog,6,"Attrib6" );
		glBindAttribLocation( _glprog,7,"Attrib7" );

		glBindAttribLocation( _glprog,0,"bb_Vertex" );
		glBindAttribLocation( _glprog,1,"bb_Normal" );
		glBindAttribLocation( _glprog,2,"bb_Tangent" );
		glBindAttribLocation( _glprog,3,"bb_TexCoords0" );
		glBindAttribLocation( _glprog,4,"bb_TexCoords1" );
		glBindAttribLocation( _glprog,5,"bb_Weights" );
		glBindAttribLocation( _glprog,6,"bb_Bones" );

		glLinkProgram( _glprog );

		glDeleteShader( vs );
		glDeleteShader( fs );

		GLint uniforms=0,unit=-1;
		glGetProgramiv( _glprog,GL_ACTIVE_UNIFORMS,&uniforms );
		for( int i=0;i<uniforms;++i ){
			char name[256];
			GLint size;
			GLenum type;
			name[0]=0;
			glGetActiveUniform( _glprog,i,255,0,&size,&type,name );
			name[255]=0;
			if( name[0]=='g' && name[1]=='l' && name[2]=='_' ) continue;
			
			if( !strcmp( name,"bb_InstanceID" ) ){
				_instanceIdLoc=glGetUniformLocation( _glprog,name );
				continue;
			}
			
			if( char *p=strchr( name,'[' ) ) *p=0;
			
//			cout<<"Param="<<name<<endl;

			switch( type ){
			case GL_FLOAT:
			case GL_FLOAT_VEC2:
			case GL_FLOAT_VEC3:
			case GL_FLOAT_VEC4:
			case GL_FLOAT_MAT4:
				break;
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_1D_SHADOW:
			case GL_SAMPLER_2D_SHADOW:
			case GL_SAMPLER_2D_RECT_ARB:
			case GL_SAMPLER_2D_RECT_SHADOW_ARB:
				++unit;
				break;
			default:
				Error( "TODO" );
			}
			
//			cout<<"Param="<<name<<" id="<<CParam::IdForName( name )<<endl;

			GLParam *p=new GLParam( CParam::ForName( name ),size,type,glGetUniformLocation( _glprog,name ),unit );

			_glparams.push_back( p );
		}
		CheckGL();
		return this;
	}

	void Bind(){
		glUseProgram( _glprog );
		if( !++paramSeq ) paramSeq=1;
	}

	void Validate(){
		for( vector<GLParam*>::iterator it=_glparams.begin();it!=_glparams.end();++it ){
			(*it)->Bind();
		}
	}
	
	int InstanceIdLoc(){
		return _instanceIdLoc;
	}
	
private:
	GLuint _glprog;
	vector<GLParam*> _glparams;
	int _instanceIdLoc;
};

/// OpenGL implementation of shaders
struct GLShader : public CShader{
public:

	GLShader(){
		_modeMask=0;
		memset( _shaders,0,sizeof(_shaders) );
	}

	GLShader *Create( string source,string header ){
		map<string,string> pmap=SplitShaderModes( source );
	
		string common=
		"//@common\n";
		
		if( gpu_instancing ){
			common+=
			"#version 120\n"
			"#extension GL_EXT_gpu_shader4 : enable\n"
			"#define bb_InstanceID gl_InstanceID\n";
		}else{
			common+=
			"uniform int bb_InstanceID;\n";
		}
		common+=
		"#extension GL_ARB_texture_rectangle : enable\n";

		for( map<string,string>::iterator it=pmap.begin();it!=pmap.end();++it ){
			int mode=ModeForName( it->first );
			common+="#define BB_"+toupper( it->first )+" "+(1<<mode)+"\n";
		}
		
		common+=
		"mat3 bb_mat3( mat4 mat ){\n"
		"\treturn mat3( mat[0].xyz,mat[1].xyz,mat[2].xyz );\n"
		"}\n";

		for( map<string,string>::iterator it=pmap.begin();it!=pmap.end();++it ){
			int mode=ModeForName( it->first );
			string modeDef="#define BB_MODE BB_"+toupper( it->first )+"\n";
			_shaders[mode]=(new GLShaderProg)->Create( common+modeDef+header+it->second );
			_modeMask|=(1<<mode);
		}
		return this;
	}
	
	GLShaderProg *ShaderProg( int mode ){
		return _shaders[mode];
	}
private:
	GLShaderProg *_shaders[32];
};

COpenGLGraphics::COpenGLGraphics():
_dirty(~0){
	if( !GLEE_VERSION_2_0 ){
		Error( "Max3d requires OpenGL 2.0 support" );
	}
	if( !GLEE_ARB_texture_rectangle ){
		Error( "Max3d requires OpenGL ARB_texture_rectangle extension" );
	}
	if( !GLEE_EXT_framebuffer_object ){
		Error( "Max3d requires OpenGL EXT_framebuffer_object extension" );
	}
	if( GLEE_EXT_draw_instanced ){
		gpu_instancing=true;
	}
	int vp[4];
	glGetIntegerv( GL_VIEWPORT,vp );
	_windowWidth=vp[2];
	_windowHeight=vp[3];
	glFrontFace( GL_CW );
}

CVertexBuffer *COpenGLGraphics::CreateVertexBuffer( int capacity,string format ){
	return (new GLVertexBuffer)->Create( capacity,format );
}

CIndexBuffer *COpenGLGraphics::CreateIndexBuffer( int capacity,string format ){
	return (new GLIndexBuffer)->Create( capacity,format );
}

CTexture *COpenGLGraphics::CreateTexture( int width,int height,int format,int flags ){
	return (new GLTexture)->Create( width,height,format,flags );
}

CTexture *COpenGLGraphics::Create3dTexture( int width,int height,int depth,int format,int flags ){
	return (new GLTexture)->Create3d( width,height,depth,format,flags );
}

CTexture *COpenGLGraphics::CreateCubeTexture( int size,int format,int flags ){
	return (new GLTexture)->CreateCube( size,format,flags );
}

CShader *COpenGLGraphics::CreateShader( string source ){
	return (new GLShader)->Create( source,_shaderHeader );
}

void COpenGLGraphics::BeginScene(){
	nSets=0;
}

void COpenGLGraphics::SetColorBuffer( int index,CTexture *texture ){
	if( texture ) texture->Retain();
	if( _colorBuffers[index] ) _colorBuffers[index]->Release();
	_colorBuffers[index]=texture;
	_dirty|=DIRTY_FRAMEBUFFER;
}

void COpenGLGraphics::SetDepthBuffer( CTexture *texture ){
	if( texture ) texture->Retain();
	if( _depthBuffer ) _depthBuffer->Release();
	_depthBuffer=texture;
	_dirty|=DIRTY_FRAMEBUFFER;
}

void COpenGLGraphics::SetViewport( int x,int y,int width,int height ){
	glViewport( x,y,width,height );
}

void COpenGLGraphics::SetShaderMode( int mode ){
	_shaderMode=mode;
	_dirty|=DIRTY_SHADERPROG;
}

void COpenGLGraphics::SetWriteMask( int mask ){
	glColorMask( !!(mask & WRITEMASK_RED),!!(mask & WRITEMASK_GREEN),!!(mask & WRITEMASK_BLUE),!!(mask & WRITEMASK_ALPHA) );
	glDepthMask( !!(mask & WRITEMASK_DEPTH) );
}

void COpenGLGraphics::SetBlendFunc( int src,int dst ){
	if( src==BLENDFUNC_ONE && dst==BLENDFUNC_ZERO ){
		glDisable( GL_BLEND );
		return;
	}
	int sf,df;
	for( int i=0;i<2;++i ){
		int n;
		switch( i ? dst : src ){
		case BLENDFUNC_ZERO:
			n=GL_ZERO;
			break;
		case BLENDFUNC_ONE:
			n=GL_ONE;
			break;
		case BLENDFUNC_SRCALPHA:
			n=GL_SRC_ALPHA;
			break;
		case BLENDFUNC_DSTALPHA:
			n=GL_DST_ALPHA;
			break;
		default:
			Error( "ERROR" );
		}
		(i ? df : sf)=n;
	}
	glEnable( GL_BLEND );
	glBlendFunc( sf,df );
}

void COpenGLGraphics::SetDepthFunc( int func ){
	switch( func ){
	case DEPTHFUNC_F:
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_NEVER );
		break;
	case DEPTHFUNC_LT:
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_LESS );
		break;
	case DEPTHFUNC_EQ:
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_EQUAL );
		break;
	case DEPTHFUNC_LE:
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_LEQUAL );
		break;
	case DEPTHFUNC_GT:
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_GREATER );
		break;
	case DEPTHFUNC_NE:
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_NOTEQUAL );
		break;
	case DEPTHFUNC_GE:
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_GEQUAL );
		break;
	case DEPTHFUNC_T:
		glDisable( GL_DEPTH_TEST );
		break;
	default:
		Error( "ERROR" );
	}
}

void COpenGLGraphics::SetCullMode( int mode ){
	switch( mode ){
	case CULLMODE_NONE:
		glDisable( GL_CULL_FACE );
		break;
	case CULLMODE_BACK:
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		break;
	case CULLMODE_FRONT:
		glEnable( GL_CULL_FACE );
		glCullFace( GL_FRONT );
		break;
	default:
		Error( "ERROR" );
	};
}

void COpenGLGraphics::SetShader( CShader *shader ){
	if( shader ) shader->Retain();
	if( _shader ) _shader->Release();
	_shader=shader;
	_dirty|=DIRTY_SHADERPROG;
}

void COpenGLGraphics::SetVertexBuffer( CVertexBuffer *buffer ){
	if( buffer ) buffer->Retain();
	if( _vertexBuffer ) _vertexBuffer->Release();
	if( _vertexBuffer=buffer ) ((GLVertexBuffer*)buffer)->Bind();
}

void COpenGLGraphics::SetIndexBuffer( CIndexBuffer *buffer ){
	if( buffer ) buffer->Retain();
	if( _indexBuffer ) _indexBuffer->Release();
	if( _indexBuffer=buffer ) ((GLIndexBuffer*)buffer)->Bind();
}

void COpenGLGraphics::SetClipPlane( int index,const float params[4] ){
	if( params ){
		double eq[4]={params[0],params[1],params[2],params[3]};
		glEnable( GL_CLIP_PLANE0+index );
		glClipPlane( GL_CLIP_PLANE0+index,eq );
	}else{
		glDisable( GL_CLIP_PLANE0+index );
	}
}

void COpenGLGraphics::Clear(){
	ValidateFrameBuffer();
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );
}

void COpenGLGraphics::ValidateShaderProg(){
	if( _dirty & DIRTY_SHADERPROG ){
		_shaderProg=((GLShader*)_shader)->ShaderProg( _shaderMode );
		_shaderProg->Bind();
		_dirty&=~DIRTY_SHADERPROG;
	}
}

void COpenGLGraphics::ValidateFrameBuffer(){
	if( _dirty & DIRTY_FRAMEBUFFER ){
		if( _colorBuffers[0] || _depthBuffer ){
			GLFrameBuffer *buf=0;
			for( vector<GLFrameBuffer*>::iterator it=_frameBufs.begin();it!=_frameBufs.end();++it ){
				buf=*it;
				int i;
				for( i=0;i<4;++i ){
					if( buf->ColorBuffer(i)!=_colorBuffers[i] ) break;
				}
				if( i==4 && buf->DepthBuffer()==_depthBuffer ) break;
				buf=0;
			}
			if( !buf ){
				buf=(new GLFrameBuffer)->Create( _colorBuffers,_depthBuffer );
				_frameBufs.push_back( buf );
			}
			buf->Bind();
		}else{
			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,0 );
			glDrawBuffer( GL_BACK );
			vector<GLFrameBuffer*> out;
			for( vector<GLFrameBuffer*>::iterator it=_frameBufs.begin();it!=_frameBufs.end();++it ){
				GLFrameBuffer *buf=*it;
				if( buf->Validate() ){
					out.push_back( buf );
				}else{
					delete buf;
				}
			}
			_frameBufs.swap( out );
		}
		_dirty&=~DIRTY_FRAMEBUFFER;
	}
}

void COpenGLGraphics::Render( int what,int first,int count,int instances ){
	
	if( _vertexBuffer ){
		int prim;
		switch( what ){
		case 1:prim=GL_POINTS;break;
		case 2:prim=GL_LINES;break;
		case 3:prim=GL_TRIANGLES;break;
		default:Error( "CDriver::Render - invalid primitive type" );return;
		}

		if( _dirty ){
			ValidateShaderProg();
			ValidateFrameBuffer();
		}
		
		_shaderProg->Validate();

		/*
		int vp,fp;
		CGLContextObj ctx=CGLGetCurrentContext();
		CGLGetParameter( ctx,kCGLCPGPUVertexProcessing,&vp );
		CGLGetParameter( ctx,kCGLCPGPUFragmentProcessing,&fp );
		cout<<"vp="<<vp<<", fp="<<fp<<endl;
		 */

		if( gpu_instancing ){
			if( GLIndexBuffer *ib=(GLIndexBuffer*)_indexBuffer ){
				glDrawElementsInstancedEXT( prim,count,ib->GLType(),(void*)(first*ib->Pitch()),instances );
			}else{
				glDrawArraysInstancedEXT( prim,first,count,instances );
			}
		}else{
			int instIdLoc=_shaderProg->InstanceIdLoc();
			for( int i=0;i<instances;++i ){
				if( instIdLoc!=-1 ){
					glUniform1i( instIdLoc,i );
				}
				if( GLIndexBuffer *ib=(GLIndexBuffer*)_indexBuffer ){
					glDrawElements( prim,count,ib->GLType(),(void*)(first*ib->Pitch()) );
				}else{
					glDrawArrays( prim,first,count );
				}
			}
		}
	}
}

void COpenGLGraphics::EndScene(){
//	cout<<"EndScene: "<<nSets<<" uniforms set"<<endl;
}
