
#include "std.h"

#include "app.h"
#include "driver.h"

#define USE_OPENGL 1
//#define USE_DIRECT3D9 1

#if USE_OPENGL

#include "GLee.h"

static int paramId( string name ){
	static map<string,int> idMap;
	static int nextId;
	map<string,int>::iterator it=idMap.find( name );
	if( it!=idMap.end() ) return it->second;
	int id=nextId++;
	if( id==256 ) Error( "Out of paramIds" );
	idMap.insert( make_pair(name,id) );
	return id;
}

static int shaderMode( string name ){
	static map<string,int> idMap;
	static int nextId;
	map<string,int>::iterator it=idMap.find( name );
	if( it!=idMap.end() ) return it->second;
	int id=nextId++;
	if( id==32 ) Error( "Out of shaderModes" );
	idMap.insert( make_pair(name,id) );
	return id;
}

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
//	free( cstr );
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

struct GLAttrib{
	int count,type,offset;
};

class GLVertexBuffer : public CObject{
public:
	GLVertexBuffer():_locked(0){
	}
	GLVertexBuffer *Create( int capacity,string format,const void *data ){
		_capacity=capacity;
		_format=format;
		//
		int offset=0;
		for( int i=0;i<format.size();i+=2 ){
			GLAttrib attr;
			attr.count=format[i]-'0';	//1,2,3,4
			if( attr.count<1 || attr.count>4 ) Error( "Invalid vertex format: "+format );
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
		SetData( data );
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
			glEnableVertexAttribArray( i );
			glVertexAttribPointer( i,attr.count,attr.type,0,_pitch,(GLvoid*)attr.offset );
		}
		for( int i=_attribs.size();i<8;++i ){
			glDisableVertexAttribArray( i );
		}
	}

private:
	int _capacity;
	string _format;
	int _pitch;
	vector<GLAttrib> _attribs;
	GLuint _glvb;
	int _locked;
	void *_lockedp;
};

class GLIndexBuffer : public CObject{
public:
	GLIndexBuffer():_locked(0){
	}

	GLIndexBuffer *Create( int capacity,string format,const void *data ){
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
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,_pitch*_capacity,0,GL_STATIC_DRAW );
		//
		SetData( data );
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
	int _capacity;
	string _format;
	int _pitch;
	int _gltype;
	GLuint _glib;
	int _locked;
	void *_lockedp;
};

class GLFrameBuffer;

class GLTexture : public CObject{
public:
	GLTexture():_glfbo(0){
	}

	int glInternalFormat(){
		switch( _format ){
		case FORMAT_RGB8:return GL_RGB;
		case FORMAT_ARGB8:return GL_RGBA;
		case FORMAT_R16F:return GL_INTENSITY16F_ARB;
		case FORMAT_ARGB16F:return GL_RGBA16F_ARB;
		case FORMAT_DEPTHBUF:return GL_DEPTH_COMPONENT;
		case FORMAT_SHADOWMAP:return GL_DEPTH_COMPONENT;
		}
		Error("TODO");
		return 0;
	}

	int glDataFormat(){
		switch( _format ){
		case FORMAT_RGB8:return GL_BGR;
		case FORMAT_ARGB8:return GL_BGRA;
		case FORMAT_R16F:return GL_RED;
		case FORMAT_ARGB16F:return GL_BGR;
		case FORMAT_DEPTHBUF:return GL_DEPTH_COMPONENT;
		case FORMAT_SHADOWMAP:return GL_DEPTH_COMPONENT;
		}
		Error( "TODO" );
		return 0;
	}

	int BytesPerTexel(){
		switch( _format ){
		case FORMAT_RGB8:return 3;
		case FORMAT_ARGB8:return 4;
		case FORMAT_R16F:return 2;
		case FORMAT_ARGB16F:return 8;
		case FORMAT_DEPTHBUF:return 0;
		case FORMAT_SHADOWMAP:return 0;
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

	GLTexture *Create2d( int width,int height,int format,int flags,const void *data ){
		//
		_width=width;
		_height=height;
		_format=format;
		_flags=flags;
		_gltarget=GL_TEXTURE_2D;
		//
		GenTexture();
		//
		if( format==FORMAT_SHADOWMAP ){
			//shadow buffer
			glTexParameteri( _gltarget,GL_TEXTURE_COMPARE_MODE,GL_COMPARE_R_TO_TEXTURE );
			glTexParameteri( _gltarget,GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL );
		}
		//
		Set2dData( data );
		CheckGL();
		//
		return this;
	}
	
	void Set2dData( const void *data ){
		glTexImage2D( _gltarget,0,glInternalFormat(),_width,_height,0,glDataFormat(),GL_UNSIGNED_BYTE,data );
	}
	
	GLTexture *Create3d( int width,int height,int depth,int format,int flags,const void *data ){
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
		Set3dData( data );
		CheckGL();
		//
		return this;
	}
	
	void Set3dData( const void *data ){
		glTexImage3D( _gltarget,0,glInternalFormat(),_width,_height,_depth,0,glDataFormat(),GL_UNSIGNED_BYTE,data );
	}
	
	GLTexture *CreateCube( int size,int format,int flags,const void *data ){
		//
		_width=size;
		_height=size;
		_format=format;
		_flags=flags;
		_gltarget=GL_TEXTURE_CUBE_MAP;
		//
		GenTexture();
		//
		SetCubeData( data );
		CheckGL();
		//
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

	int _width,_height,_depth,_format,_flags;
	GLenum _gltarget;
	GLuint _gltex;
	GLuint _glfbo;
};

class GLFrameBuffer : public CObject{
public:
	GLFrameBuffer *Create( GLTexture *textures[4],GLTexture *depth ){
		glGenFramebuffersEXT( 1,&_glfbo );
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,_glfbo );

		for( int i=0;i<4;++i ){
			if( textures[i] ){
				_glbufs[i]=GL_COLOR_ATTACHMENT0_EXT+i;
				glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,_glbufs[i],GL_TEXTURE_2D,textures[i]->_gltex,0 );
			}else{
				_glbufs[i]=GL_NONE;
			}
		}

		if( depth ){
			glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,depth->_gltex,0 );
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

	void Bind(){
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,_glfbo );
		glDrawBuffers( 4,_glbufs );
	}

private:
	GLuint _glfbo;
	GLenum _glbufs[4];
};

static int paramSeq;

struct CParam{
	CParam():_seq(-1),_texture(0),_floatFunc(0){
	}

	bool Validate(){
		if( _seq==paramSeq ) return true;
		_seq=paramSeq;
		return false;
	}

	void Invalidate(){
		_seq=paramSeq-1;
		for( vector<CParam*>::iterator it=_defs.begin();it!=_defs.end();++it ){
			(*it)->Invalidate();
		}
	}

	void AddDef( CParam *p ){
		_defs.push_back( p );
	}

	void SetFloatFunc( int count,FloatParamFunc func ){
		_floatFunc=func;
		_floatFuncCount=count;
	}

	void SetFloatValue( int count,const float *value ){
		if( _floatFunc ) Error( "Param is read-only" );
		memcpy( _float,value,count*4 );
		Invalidate();
	}

	const float *FloatValue(){
		if( _floatFunc ) memcpy( _float,_floatFunc(),_floatFuncCount*4 );
		return _float;
	}

	void SetTextureValue( GLTexture *texture ){
		if( texture ) texture->Retain();
		if( _texture ) _texture->Release();
		_texture=texture;
		Invalidate();
	}

	GLTexture *TextureValue(){
		return _texture;
	}

	int _seq;
	float _float[16];
	GLTexture *_texture;
	vector<CParam*> _defs;
	FloatParamFunc _floatFunc;
	int _floatFuncCount;
};

static CParam params[256];

//static CParam *params[256];
//static int paramSeqs[256];

class GLParam{
public:
	GLParam( int paramId,int glsize,GLenum gltype,int glloc,int glunit ){
		_paramId=paramId;
		_glsize=glsize;
		_gltype=gltype;
		_glloc=glloc;
		_glunit=glunit;
	}

	void Bind(){
		CParam *p=&params[_paramId];

		if( p->Validate() ) return;

		switch( _gltype ){
		case GL_FLOAT:
			glUniform1fv( _glloc,1,p->FloatValue() );
			break;
		case GL_FLOAT_VEC2:
			glUniform2fv( _glloc,1,p->FloatValue() );
			break;
		case GL_FLOAT_VEC3:
			glUniform3fv( _glloc,1,p->FloatValue() );
			break;
		case GL_FLOAT_VEC4:
			glUniform4fv( _glloc,1,p->FloatValue() );
			break;
		case GL_FLOAT_MAT4:
			glUniformMatrix4fv( _glloc,1,GL_FALSE,p->FloatValue() );
			break;
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
			if( GLTexture *t=p->TextureValue() ){
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
	int _paramId;
	int _glloc;
	int _glsize;
	GLenum _gltype;
	int _glunit;
};

class GLShaderProg : public CObject{
public:
	GLShaderProg():_glprog(0){
	}

	GLShaderProg *Create( string source ){

		string segs[3];
		SplitShaderSegs( source,segs );

		string vert=segs[0]+segs[1];
		string frag=segs[0]+segs[2];

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
				++unit;
				break;
			default:
				Error( "TODO" );
			}
			
//			cout<<"Param="<<name<<" id="<<CParam::IdForName( name )<<endl;

			GLParam *p=new GLParam( paramId( name ),size,type,glGetUniformLocation( _glprog,name ),unit );

			_glparams.push_back( p );
		}
		CheckGL();
		return this;
	}

	void Bind(){
		glUseProgram( _glprog );
	}

	void Validate(){
		for( vector<GLParam*>::iterator it=_glparams.begin();it!=_glparams.end();++it ){
			(*it)->Bind();
		}
	}

private:
	GLuint _glprog;
	vector<GLParam*> _glparams;
};

struct GLShader : public CObject{
public:
	GLShader():_mask(0){
		memset( _shaders,0,sizeof(_shaders) );
	}

	GLShader *Create( string source,string header ){
		map<string,string> pmap=SplitShaderModes( source );
		for( map<string,string>::iterator it=pmap.begin();it!=pmap.end();++it ){
			int mode=shaderMode( it->first );
			_shaders[mode]=(new GLShaderProg)->Create( header+it->second );
			_mask|=(1<<mode);
		}
		return this;
	}

	int RenderMask(){
		return _mask;
	}

	bool Validate( int mode ){
		GLShaderProg *prog=_shaders[mode];
		if( !prog ) return false;
		prog->Bind();
		prog->Validate();
		return true;
	}

private:
	int _mask;
	GLShaderProg *_shaders[32];
};

static string _shaderHeader;
static int _shaderMode;
static GLFrameBuffer *_fbuffer;
static GLShader *_shader;
static GLVertexBuffer *_vbuffer;
static GLIndexBuffer *_ibuffer;

CDriver::CDriver(){
	glFrontFace( GL_CW );
}

int CDriver::Type(){
	return DRIVER_OPENGL;
}

CObject *CDriver::CreateVertexBuffer( int capacity,string format,const void *data ){
	return (new GLVertexBuffer)->Create( capacity,format,data );
}

void CDriver::SetVertexBufferData( CObject *buffer,const void *data ){
	((GLVertexBuffer*)buffer)->SetData( data );
}

void *CDriver::LockVertexBuffer( CObject *buffer ){
	return ((GLVertexBuffer*)buffer)->Lock();
}

void CDriver::UnlockVertexBuffer( CObject *buffer ){
	((GLVertexBuffer*)buffer)->Unlock();
}

void CDriver::SetVertexBufferAttrib( CObject *buffer,int vertex,int attrib,float x,float y,float z,float w ){
	((GLVertexBuffer*)buffer)->SetAttrib( vertex,attrib,x,y,z,w );
}

CObject *CDriver::CreateIndexBuffer( int capacity,string format,const void *data ){
	return (new GLIndexBuffer)->Create( capacity,format,data );
}

void CDriver::SetIndexBufferData( CObject *buffer,const void *data ){
	((GLIndexBuffer*)buffer)->SetData( data );
}

void *CDriver::LockIndexBuffer( CObject *buffer ){
	return ((GLIndexBuffer*)buffer)->Lock();
}

void CDriver::UnlockIndexBuffer( CObject *buffer ){
	((GLIndexBuffer*)buffer)->Unlock();
}

void CDriver::SetIndexBufferIndex( CObject *buffer,int index,int value ){
	((GLIndexBuffer*)buffer)->SetIndex( index,value );
}

CObject *CDriver::Create2dTexture( int width,int height,int format,int flags,const void *data ){
	return (new GLTexture)->Create2d( width,height,format,flags,data );
}

void CDriver::Set2dTextureData( CObject *texture,const void *data ){
	((GLTexture*)texture)->Set2dData( data );
}

CObject *CDriver::Create3dTexture( int width,int height,int depth,int format,int flags,const void *data ){
	return (new GLTexture)->Create3d( width,height,depth,format,flags,data );
}

void CDriver::Set3dTextureData( CObject *texture,const void *data ){
	((GLTexture*)texture)->Set3dData( data );
}

CObject *CDriver::CreateCubeTexture( int size,int format,int flags,const void *data ){
	return (new GLTexture)->CreateCube( size,format,flags,data );
}

void CDriver::SetCubeTextureData( CObject *texture,const void *data ){
	((GLTexture*)texture)->SetCubeData( data );
}

void CDriver::AddShaderHeader( string header ){
	_shaderHeader+=header;
}

CObject *CDriver::CreateShader( string source ){
	return (new GLShader)->Create( source,_shaderHeader );
}

int CDriver::ShaderMask( CObject *shader ){
	return ((GLShader*)shader)->RenderMask();
}

CObject *CDriver::CreateFrameBuffer( CObject *color0,CObject *color1,CObject *color2,CObject *color3,CObject *depth ){
	GLTexture *textures[]={
		(GLTexture*)color0,
		(GLTexture*)color1,
		(GLTexture*)color2,
		(GLTexture*)color3 };
	return (new GLFrameBuffer)->Create( textures,(GLTexture*)depth );
}

int CDriver::ParamId( string name ){
	return paramId( name );
}

int CDriver::ShaderMode( string name ){
	return shaderMode( name );
}

void CDriver::SetFloatParamFunc( string name,int count,FloatParamFunc func,const vector<string> &uses ){
	int id=paramId( name );
	params[id].SetFloatFunc( count,func );
	for( vector<string>::const_iterator it=uses.begin();it!=uses.end();++it ){
		params[ paramId(*it) ].AddDef( &params[id] );
	}
}

void CDriver::SetFloatParam( int id,int count,const float *value ){
	params[id].SetFloatValue( count,value );
}

void CDriver::SetFloatParam( string name,int count,const float *value ){
	SetFloatParam( paramId( name ),count,value );
}

const float *CDriver::FloatParam( int id ){
	return params[id].FloatValue();
}

const float *CDriver::FloatParam( string name ){
	return FloatParam( paramId( name ) );
}

void CDriver::SetTextureParam( int id,CObject *texture ){
	params[id].SetTextureValue( (GLTexture*)texture );
}

void CDriver::SetTextureParam( string name,CObject *texture ){
	SetTextureParam( paramId( name ),texture );
}

CObject *CDriver::TextureParam( int id ){
	return params[id].TextureValue();
}

CObject *CDriver::TextureParam( string name ){
	return TextureParam( paramId( name ) );
}

void CDriver::Clear(){
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );
}

void CDriver::SetFrameBuffer( CObject *fbuffer ){
	if( _fbuffer=(GLFrameBuffer*)fbuffer ){
		_fbuffer->Bind();
	}else{
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT,0 );
		glDrawBuffer( GL_BACK );
	}
}

void CDriver::SetClipPlane( int index,const float *params ){
	if( params ){
		double eq[4]={params[0],params[1],params[2],params[3]};
		glEnable( GL_CLIP_PLANE0+index );
		glClipPlane( GL_CLIP_PLANE0+index,eq );
	}else{
		glDisable( GL_CLIP_PLANE0+index );
	}
}

void CDriver::SetViewport( int x,int y,int width,int height ){
	glViewport( x,y,width,height );
}

void CDriver::SetShaderMode( int mode ){
	_shaderMode=mode;
}

void CDriver::SetWriteMask( int mask ){
	glColorMask( !!(mask & WRITEMASK_RED),!!(mask & WRITEMASK_GREEN),!!(mask & WRITEMASK_BLUE),!!(mask & WRITEMASK_ALPHA) );
	glDepthMask( !!(mask & WRITEMASK_DEPTH) );
}

void CDriver::SetBlendFunc( int src,int dst ){
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

void CDriver::SetDepthFunc( int func ){
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

void CDriver::SetCullMode( int mode ){
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
};

void CDriver::SetShader( CObject *shader ){
	if( shader ) shader->Retain();
	if( _shader ) _shader->Release();
	_shader=(GLShader*)shader;
	++paramSeq;
}

void CDriver::SetVertexBuffer( CObject *vbuffer ){
	if( vbuffer ) vbuffer->Retain();
	if( _vbuffer ) _vbuffer->Release();
	if( _vbuffer=(GLVertexBuffer*)vbuffer ) _vbuffer->Bind();
}

void CDriver::SetIndexBuffer( CObject *ibuffer ){
	if( ibuffer ) ibuffer->Retain();
	if( _ibuffer ) _ibuffer->Release();
	if( _ibuffer=(GLIndexBuffer*)ibuffer ) _ibuffer->Bind();
}

void CDriver::Render( int what,int first,int count ){
	if( _vbuffer ){
		int prim;
		switch( what ){
		case 1:prim=GL_POINTS;break;
		case 2:prim=GL_LINES;break;
		case 3:prim=GL_TRIANGLES;break;
		default:Error( "CDriver::Render - invalid primitive type" );return;
		}
		if( _shader->Validate( _shaderMode ) ){
			if( _ibuffer ){
				glDrawElements( prim,count,_ibuffer->GLType(),(void*)(first*_ibuffer->Pitch()) );
			}else{
				glDrawArrays( prim,first,count );
			}
		}
	}
}

void CDriver::BeginScene(){
}

void CDriver::EndScene(){
}

#endif

#if USE_DIRECT3D9

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

#pragma comment( lib, "dxerr.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "comctl32.lib" )

static IDirect3D9 *d3d;
static IDirect3DDevice9 *d3dDev;
static IDirect3DSurface9 *defaultRenderTarget;

static D3DPRESENT_PARAMETERS presentParams;

static void _dxTry( HRESULT hr,const char *file,int line ){
	if( hr>=0 ) return;
	cout<<DXGetErrorString( hr )<<endl;
}

#define dxTry( X ) _dxTry( X,__FILE__,__LINE__ )

static int paramId( string name ){
	static map<string,int> idMap;
	static int nextId;
	map<string,int>::iterator it=idMap.find( name );
	if( it!=idMap.end() ) return it->second;
	idMap.insert( make_pair(name,nextId) );
	return nextId++;
}

class D3D9FrameBuffer;

class D3D9Texture : public CObject{
public:
	D3DFORMAT d3dFormat(){
		switch( _format ){
//		case FORMAT_RGB8:return D3DFMT_R8G8B8;				//Not supported?!?
		case FORMAT_RGB8:return D3DFMT_X8R8G8B8;
		case FORMAT_ARGB8:return D3DFMT_A8R8G8B8;
		case FORMAT_R16F:return D3DFMT_R16F;
		case FORMAT_ARGB16F:return D3DFMT_A16B16G16R16F;
		case FORMAT_DEPTHBUF:return D3DFMT_D24S8;
		case FORMAT_SHADOWMAP:return D3DFMT_D24S8;
		}
		Error( "TODO" );
		return D3DFMT_UNKNOWN;
	}

	int BytesPerTexel(){
		switch( _format ){
//		case FORMAT_RGB8:return D3DFMT_R8G8B8;				//Not supported?!?
		case FORMAT_RGB8:return 3;
		case FORMAT_ARGB8:return 4;
		case FORMAT_R16F:return 2;
		case FORMAT_ARGB16F:return 8;
		case FORMAT_DEPTHBUF:return 0;
		case FORMAT_SHADOWMAP:return 0;
		}
		Error( "TODO" );
		return D3DFMT_UNKNOWN;
	}

	D3D9Texture *Create2d( int width,int height,int format,int flags,const void *data ){
		_width=width;
		_height=height;
		_format=format;
		_flags=flags;

		UINT levels=1;
		DWORD usage=0;
		D3DFORMAT dformat=d3dFormat();
		D3DPOOL pool=D3DPOOL_MANAGED;

		if( _flags & TEXTURE_MIPMAP ){
			levels=0;
			usage|=D3DUSAGE_AUTOGENMIPMAP;
		}
		if( _flags & TEXTURE_STATIC ){
			pool=D3DPOOL_MANAGED;
		}
		if( _flags & TEXTURE_RENDER ){
			pool=D3DPOOL_DEFAULT;
			if( _format==FORMAT_DEPTHBUF || _format==FORMAT_SHADOWMAP ){
				usage|=D3DUSAGE_DEPTHSTENCIL;
			}else{
				usage|=D3DUSAGE_RENDERTARGET;
			}
		}

		if( d3dDev->CreateTexture( width,height,levels,usage,dformat,pool,&_d3dtex,0 )<0 ){
			Error( "CreateTexture failed" );
		}

		//don't really know if this achieves same things as gl driver...?
		if( _flags & TEXTURE_FILTER ){
			_d3dmagfilter=D3DTEXF_LINEAR;
			_d3dminfilter=D3DTEXF_LINEAR;
			if( _flags & TEXTURE_MIPMAP ){
				_d3dmipfilter=D3DTEXF_LINEAR;
			}else{
				_d3dmipfilter=D3DTEXF_NONE;
			}
		}else{
			_d3dmagfilter=D3DTEXF_POINT;
			_d3dminfilter=D3DTEXF_POINT;
			if( _flags & TEXTURE_MIPMAP ){
				_d3dmipfilter=D3DTEXF_POINT;
			}else{
				_d3dmipfilter=D3DTEXF_NONE;
			}
		}
		if( _flags & TEXTURE_CLAMPS ){
			_d3daddressu=D3DTADDRESS_CLAMP;
		}else{
			_d3daddressu=D3DTADDRESS_WRAP;
		}
		if( _flags & TEXTURE_CLAMPT ){
			_d3daddressv=D3DTADDRESS_CLAMP;
		}else{
			_d3daddressv=D3DTADDRESS_WRAP;
		}

		Set2dData( data );
		return this;
	}
	
	void Set2dData( const void *data ){
		if( !data ) return;
		D3DLOCKED_RECT lock;
		if( _d3dtex->LockRect( 0,&lock,0,0 )<0 ){
			Error( "LockRect failed" );
		}
		void *bits=lock.pBits;
		int bpr=BytesPerTexel() * _width;
		if( _format=FORMAT_RGB8 ){
			for( int y=0;y<_height;++y ){
				char *d=(char*)bits;
				char *s=(char*)data;
				for( int x=0;x<_width;++x ){
					d[0]=s[0];
					d[1]=s[1];
					d[2]=s[2];
					d[3]=1;		//A?
					d+=4;
					s+=3;
				}
				bits=(char*)bits+lock.Pitch;
				data=(char*)data+((bpr+3)&~3);
			}
		}else{
			for( int y=0;y<_height;++y ){
				memcpy( bits,data,bpr );
				bits=(char*)bits+lock.Pitch;
				data=(char*)data+((bpr+3)&~3);
			}
		}
		_d3dtex->UnlockRect( 0 );
	}
	
	D3D9Texture *Create3d( int width,int height,int depth,int format,int flags,const void *data ){
		Error( "TODO" );
		return 0;
	}
	
	void Set3dData( const void *data ){
		Error( "TODO" );
	}
	
	D3D9Texture *CreateCube( int size,int format,int flags,const void *data ){
		Error( "TODO" );
		return 0;
	}

	void SetCubeData( const void *data  ){
		Error( "TODO" );
	}

	void Bind( int sampler ){
		d3dDev->SetTexture( sampler,_d3dtex );
		d3dDev->SetSamplerState( sampler,D3DSAMP_ADDRESSU,_d3daddressu );
		d3dDev->SetSamplerState( sampler,D3DSAMP_ADDRESSU,_d3daddressv );
		d3dDev->SetSamplerState( sampler,D3DSAMP_MAGFILTER,_d3dmagfilter );
		d3dDev->SetSamplerState( sampler,D3DSAMP_MINFILTER,_d3dminfilter );
		d3dDev->SetSamplerState( sampler,D3DSAMP_MIPFILTER,_d3dmipfilter );
	}
	
private:
	friend class D3D9FrameBuffer;

	int _width,_height,_depth,_format,_flags;

	IDirect3DTexture9 *_d3dtex;
	D3DTEXTUREADDRESS _d3daddressu,_d3daddressv;
	D3DTEXTUREFILTERTYPE _d3dmagfilter,_d3dminfilter,_d3dmipfilter;
};

class D3D9FrameBuffer : public CObject{
public:
	D3D9FrameBuffer *Create( D3D9Texture *textures[4],D3D9Texture *depth ){
		for( int i=0;i<4;++i ){
			_d3dtexs[i]=textures[i];
		}
		_d3ddepth=depth;
		return this;
	}

	void Bind(){
		for( int i=0;i<4;++i ){
			D3D9Texture *tex=_d3dtexs[i];
			if( !tex ){
				if( !i ){
					Error( "RenderTarget 0 cannot be null" );
				}else{
					d3dDev->SetRenderTarget( i,0 );
				}
				continue;
			}
			IDirect3DSurface9 *surf;
			if( tex->_d3dtex->GetSurfaceLevel( 0,&surf )<0 ){
				Error( "GetSurfaceLevel failed" );
			}
			d3dDev->SetRenderTarget( i,surf );
			surf->Release();
		}
		if( _d3ddepth ){
			IDirect3DSurface9 *surf;
			if( _d3ddepth->_d3dtex->GetSurfaceLevel( 0,&surf )<0 ){
				Error( "GetSurfaceLevel failed" );
			}
			d3dDev->SetDepthStencilSurface( surf );
			surf->Release();
		}else{
			d3dDev->SetDepthStencilSurface( 0 );
		}
	}

private:
	D3D9Texture *_d3dtexs[4];
	D3D9Texture *_d3ddepth;
};

class D3D9VertexBuffer : public CObject{
public:
	D3D9VertexBuffer():_locked(0){
	}
	D3D9VertexBuffer *Create( int capacity,string format,const void *data ){
		D3DVERTEXELEMENT9 vd[32];

		_capacity=capacity;
		_format=format;

		int i,offset=0;
		for( i=0;i<format.size()/2;++i ){
			int type,size;
			switch( format[i*2+1] ){
			case 'b':
				switch( format[i*2] ){
				case '4':type=D3DDECLTYPE_UBYTE4;size=4;break;
				default:
					cout<<format<<endl;
					Error( "Error" );
				}
				break;
			case 'f':
				switch( format[i*2] ){
				case '1':type=D3DDECLTYPE_FLOAT1;size=4;break;
				case '2':type=D3DDECLTYPE_FLOAT2;size=8;break;
				case '3':type=D3DDECLTYPE_FLOAT3;size=12;break;
				case '4':type=D3DDECLTYPE_FLOAT4;size=16;break;
				default:
					cout<<format<<endl;
					Error( "Error" );
				}
				break;
			default:
				cout<<format<<endl;
				Error( "TODO" );
			}
			vd[i].Stream=0;
			vd[i].Offset=offset;
			vd[i].Type=type;
			vd[i].Method=D3DDECLMETHOD_DEFAULT;
			vd[i].Usage=i>0 ? D3DDECLUSAGE_TEXCOORD : D3DDECLUSAGE_POSITION;
			vd[i].UsageIndex=i>0 ? i-1 : 0;

			offset+=size;
		}
		_pitch=offset;

		vd[i].Stream=0xff;
		vd[i].Offset=0;
		vd[i].Type=D3DDECLTYPE_UNUSED;
		vd[i].Method=0;
		vd[i].Usage=0;
		vd[i].UsageIndex=0;

		if( d3dDev->CreateVertexBuffer( _pitch*_capacity,0,0,D3DPOOL_MANAGED,&_d3dvb,0 )<0 ){
			Error( "CreateVertexBuffer failed" );
		}

		if( d3dDev->CreateVertexDeclaration( vd,&_d3dvd )<0 ){
			Error( "CreateVertexDeclaration failed" );
		}

		SetData( data );
		return this;
	}
	
	void SetData( const void *data ){
		void *p=Lock();
		if( data ) memcpy( p,data,_pitch*_capacity );
		Unlock();
	}

	void *Lock(){
		if( !_locked++ ){
			if( _d3dvb->Lock( 0,0,&_lockedp,0 )<0 ){
				Error( "Lock failed" );
			}
		}
		return _lockedp;
	}
	
	void Unlock(){
		if( !--_locked ){
			_d3dvb->Unlock();
		}
	}
	
	void Bind(){
		d3dDev->SetVertexDeclaration( _d3dvd );
		d3dDev->SetStreamSource( 0,_d3dvb,0,_pitch );
	}

	int Capacity(){
		return _capacity;
	}

private:
	int _capacity;
	string _format;
	int _pitch;
	int _locked;
	void *_lockedp;
	IDirect3DVertexBuffer9 *_d3dvb;
	IDirect3DVertexDeclaration9 *_d3dvd;
};

class D3D9IndexBuffer : public CObject{
public:
	D3D9IndexBuffer::D3D9IndexBuffer():_locked(0){
	}
	D3D9IndexBuffer *Create( int capacity,string format,const void *data ){
		_capacity=capacity;
		_format=format;

		D3DFORMAT d3dformat;
		if( format=="1s" ){
			_pitch=2;
			d3dformat=D3DFMT_INDEX16;
		}else if( format=="1i" ){
			_pitch=4;
			d3dformat=D3DFMT_INDEX32;
		}else{
			Error( "TODO" );
		}
		if( d3dDev->CreateIndexBuffer( _pitch*_capacity,0,d3dformat,D3DPOOL_MANAGED,&_d3dib,0 )<0 ){
			Error( "CreateIndexBuffer failed" );
		}
		SetData( data );
		return this;
	}

	void SetData( const void *data ){
		void *p=Lock();
		if( data ) memcpy( p,data,_pitch * _capacity );
		Unlock();
	}

	void *Lock(){
		if( !_locked++ ){
			_d3dib->Lock( 0,0,&_lockedp,0 );
		}
		return _lockedp;
	}

	void Unlock(){
		if( !--_locked ){
			_d3dib->Unlock();
		}
	}

	void SetIndex( int index,int value ){
		Lock();
		switch( _pitch ){
		case 2:((short*)_lockedp)[index]=value;break;
		case 4:((int*)_lockedp)[index]=value;break;
		}
		Unlock();
	}
		
	void Bind(){
		d3dDev->SetIndices( _d3dib );
	}

private:
	int _capacity;
	string _format;
	int _pitch;
	int _locked;
	void *_lockedp;
	IDirect3DIndexBuffer9 *_d3dib;
};

struct CParam{
	CParam():seq(-1),texture(0){}
	int seq;
	float value[16];
	D3D9Texture *texture;
};

static CParam params[256];
static int paramSeq;

class D3D9Param{
public:
	D3D9Param( const char *name,bool vs,int regSet,int regIndex,int regCount ):_id(paramId(name)),_vs(vs),_regSet(regSet),_regIndex(regIndex),_regCount(regCount){
	}

	void Bind(){
		CParam *p=&params[_id];

		switch( _regSet ){
		case D3DXRS_BOOL:
		case D3DXRS_INT4:
			Error( "TODO" );
		case D3DXRS_FLOAT4:
			if( _vs ){
				d3dDev->SetVertexShaderConstantF( _regIndex,p->value,_regCount );
			}else{
				d3dDev->SetPixelShaderConstantF( _regIndex,p->value,_regCount );
			}
			break;
		case D3DXRS_SAMPLER:
			if( p->texture ){
				p->texture->Bind( _regIndex );
			}
			break;
		default:
			Error( "TODO" );
		}
	}
private:
	int _id;
	bool _vs;
	int _regSet,_regIndex,_regCount;
};

class D3D9Shader : public CObject{
public:
	D3D9Shader *Create( string source ){
		int i0=source.find( "//@common" );
		int i1=source.find( "//@vertex" );
		int i2=source.find( "//@fragment" );
		if( i2==string::npos ) i2=source.find( "//@pixel" );
		int i3=source.size();

		string comm,vert,frag;
		if( i0!=string::npos ) comm=source.substr( i0,i1 );
		if( i1!=string::npos && i2!=string::npos && i2>i1 ) vert=source.substr( i1,i2-i1 );
		if( i2!=string::npos && i3!=string::npos && i3>i2 ) frag=source.substr( i2,i3-i2 );
		vert=comm+vert;
		frag=comm+frag;

		for( int i=0;i<2;++i ){

			string src=i ? frag : vert;
			string prof=i ? "ps_3_0" : "vs_3_0";

			ID3DXBuffer *shader=0,*errs=0;
			ID3DXConstantTable *consts=0;

			if( D3DXCompileShader( src.c_str(),src.size(),0,0,"main",prof.c_str(),0,&shader,&errs,&consts )<0 ){
				istringstream in( src );
				string t;
				int n=0;
				while( getline( in,t ) ){
					cout<<++n<<':'<<t<<endl;
				}
				char *p=(char*)errs->GetBufferPointer();
				cout<<"D3DXCompileShader failed:"<<p<<endl;
				Error( "" );
			}

			//check out constants
//			cout<<"***** "<<(i ? "Pixel Shader" : "Vertex Shader")<<" Constants *****"<<endl;
			D3DXCONSTANTTABLE_DESC consts_desc;
			consts->GetDesc( &consts_desc );
//			cout<<"Constants="<<consts_desc.Constants<<endl;
			for( int k=0;k<consts_desc.Constants;++k ){
				D3DXHANDLE handle=consts->GetConstant( 0,k );
				D3DXCONSTANT_DESC desc;
				UINT count=1;
				if( consts->GetConstantDesc( handle,&desc,&count )<0 ){
					Error( "GetConstantDesc failed." );
				}
				if( count!=1 ){
					Warning( "Multiple shader constant weirdness" );
					continue;
				}
//				cout<<"Const "<<k<<":\nname="<<desc.Name<<endl;
//				cout<<"regset="<<desc.RegisterSet<<endl;
//				cout<<"regindex="<<desc.RegisterIndex<<endl;
//				cout<<"regcount="<<desc.RegisterCount<<endl;
				const char *name=desc.Name;
				int regSet=desc.RegisterSet;
				int regIndex=desc.RegisterIndex;
				int regCount=desc.RegisterCount;
				_params.push_back( D3D9Param( name,i==0,regSet,regIndex,regCount ) );
			}

			DWORD *pFun=(DWORD*)shader->GetBufferPointer();
			if( i ){
				if( d3dDev->CreatePixelShader( pFun,&_d3dps )<0 ){
					Error( "CreatePixelShader failed" );
				}
			}else{
				if( d3dDev->CreateVertexShader( pFun,&_d3dvs )<0 ){
					Error( "CreateVertexShader failed" );
				}
			}
		}
		return this;
	}

	void Bind(){
		d3dDev->SetVertexShader( _d3dvs );
		d3dDev->SetPixelShader( _d3dps );
	}

	void Validate(){
		for( int i=0;i<8;++i ){
			d3dDev->SetTexture( i,0 );
		}
		for( vector<D3D9Param>::iterator it=_params.begin();it!=_params.end();++it ){
			it->Bind();
		}
	}

private:
	vector<D3D9Param> _params;
	IDirect3DVertexShader9 *_d3dvs;
	IDirect3DPixelShader9 *_d3dps;
};

static D3D9FrameBuffer *_fbuffer;
static D3D9Shader *_shader;
static D3D9VertexBuffer *_vbuffer;
static D3D9IndexBuffer *_ibuffer;

CDriver::CDriver(){
}

int CDriver::Type(){
	return DRIVER_DIRECT3D9;
}

CObject *CDriver::CreateVertexBuffer( int capacity,string format,const void *data ){
	return (new D3D9VertexBuffer)->Create( capacity,format,data );
}

void CDriver::SetVertexBufferData( CObject *buffer,const void *data ){
	((D3D9VertexBuffer*)buffer)->SetData( data );
}

void *CDriver::LockVertexBuffer( CObject *buffer ){
	return ((D3D9VertexBuffer*)buffer)->Lock();
}

void CDriver::UnlockVertexBuffer( CObject *buffer ){
	((D3D9VertexBuffer*)buffer)->Unlock();
}

void CDriver::SetVertexBufferAttrib( CObject *buffer,int vertex,int attrib,float x,float y,float z,float w ){
	Error( "TODO" );
//	((D3D9VertexBuffer*)buffer)->SetAttrib( vertex,attrib,x,y,z,w );
}

CObject *CDriver::CreateIndexBuffer( int capacity,string format,const void *data ){
	return (new D3D9IndexBuffer)->Create( capacity,format,data );
}

void CDriver::SetIndexBufferData( CObject *buffer,const void *data ){
	((D3D9IndexBuffer*)buffer)->SetData( data );
}

void *CDriver::LockIndexBuffer( CObject *buffer ){
	return ((D3D9IndexBuffer*)buffer)->Lock();
}

void CDriver::UnlockIndexBuffer( CObject *buffer ){
	((D3D9IndexBuffer*)buffer)->Unlock();
}

void CDriver::SetIndexBufferIndex( CObject *buffer,int index,int value ){
	Error( "TODO" );
//	((D3D9IndexBuffer*)buffer)->SetIndex( index,value );
}

CObject *CDriver::Create2dTexture( int width,int height,int format,int flags,const void *data ){
	return (new D3D9Texture)->Create2d( width,height,format,flags,data );
}

void CDriver::Set2dTextureData( CObject *texture,const void *data ){
	((D3D9Texture*)texture)->Set2dData( data );
}

CObject *CDriver::Create3dTexture( int width,int height,int depth,int format,int flags,const void *data ){
	return (new D3D9Texture)->Create3d( width,height,depth,format,flags,data );
}

void CDriver::Set3dTextureData( CObject *texture,const void *data ){
	((D3D9Texture*)texture)->Set3dData( data );
}

CObject *CDriver::CreateCubeTexture( int size,int format,int flags,const void *data ){
	return (new D3D9Texture)->CreateCube( size,format,flags,data );
}

void CDriver::SetCubeTextureData( CObject *texture,const void *data ){
	((D3D9Texture*)texture)->SetCubeData( data );
}

CObject *CDriver::CreateShader( string source ){
	return (new D3D9Shader)->Create( source );
}

CObject *CDriver::CreateFrameBuffer( CObject *color0,CObject *color1,CObject *color2,CObject *color3,CObject *depth ){
	D3D9Texture *textures[]={
		(D3D9Texture*)color0,
		(D3D9Texture*)color1,
		(D3D9Texture*)color2,
		(D3D9Texture*)color3 };
	return (new D3D9FrameBuffer)->Create( textures,(D3D9Texture*)depth );
}

int CDriver::ParamId( string name ){
	return paramId( name );
}

void CDriver::SetFloatParam( int id,int count,const float *value ){
	memcpy( params[id].value,value,count*4 );
	params[id].seq=paramSeq-1;
}

void CDriver::SetTextureParam( int id,CObject *texture ){
	if( texture ) texture->Retain();
	if( params[id].texture ) params[id].texture->Release();
	params[id].texture=(D3D9Texture*)texture;
	params[id].seq=paramSeq-1;
}

void CDriver::SetFloatParam( string name,int count,const float *value ){
	SetFloatParam( paramId( name ),count,value );
}

void CDriver::SetTextureParam( string name,CObject *texture ){
	SetTextureParam( paramId( name ),texture );
}

void CDriver::Clear(){
	d3dDev->Clear( 0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0,1,0 );
}

void CDriver::SetFrameBuffer( CObject *fbuffer ){
	if( _fbuffer=(D3D9FrameBuffer*)fbuffer ){
		_fbuffer->Bind();
	}else{
		d3dDev->SetRenderTarget( 0,defaultRenderTarget );
		d3dDev->SetDepthStencilSurface( 0 );
	}
}

void CDriver::SetViewport( int x,int y,int width,int height ){
	D3DVIEWPORT9 vp={x,y,width,height,0,1};
	d3dDev->SetViewport( &vp );
}

void CDriver::SetRenderMode( int mode ){
	switch( mode ){
	case RENDERMODE_AMBIENT:
		//
		d3dDev->SetRenderState( D3DRS_COLORWRITEENABLE,15 );
		d3dDev->SetRenderState( D3DRS_ALPHABLENDENABLE,FALSE );
		d3dDev->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
		d3dDev->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ZERO );
		//
		d3dDev->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		d3dDev->SetRenderState( D3DRS_ZENABLE,D3DZB_TRUE );
		d3dDev->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );
		//
		d3dDev->SetRenderState( D3DRS_CULLMODE,D3DCULL_CCW );
		//
		break;
	case RENDERMODE_SHADOWMAP:
		//
		d3dDev->SetRenderState( D3DRS_COLORWRITEENABLE,0 );
		d3dDev->SetRenderState( D3DRS_ALPHABLENDENABLE,FALSE );
		d3dDev->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_ZERO );
		d3dDev->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_DESTCOLOR );
		//
		d3dDev->SetRenderState( D3DRS_ZWRITEENABLE,TRUE );
		d3dDev->SetRenderState( D3DRS_ZENABLE,D3DZB_TRUE );
		d3dDev->SetRenderState( D3DRS_ZFUNC,D3DCMP_LESSEQUAL );
		//
		d3dDev->SetRenderState( D3DRS_CULLMODE,D3DCULL_CW );
		//
		break;
	case RENDERMODE_ADDITIVE:
		//
		d3dDev->SetRenderState( D3DRS_COLORWRITEENABLE,15 );
		d3dDev->SetRenderState( D3DRS_ALPHABLENDENABLE,TRUE );
		d3dDev->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
		d3dDev->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ONE );
		//
		d3dDev->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );
		d3dDev->SetRenderState( D3DRS_ZENABLE,D3DZB_FALSE );
		//
		d3dDev->SetRenderState( D3DRS_CULLMODE,D3DCULL_NONE );
		//
		break;
	case RENDERMODE_REPLACE:
		//
		d3dDev->SetRenderState( D3DRS_COLORWRITEENABLE,15 );
		d3dDev->SetRenderState( D3DRS_ALPHABLENDENABLE,FALSE );
		d3dDev->SetRenderState( D3DRS_SRCBLEND,D3DBLEND_ONE );
		d3dDev->SetRenderState( D3DRS_DESTBLEND,D3DBLEND_ZERO );
		//
		d3dDev->SetRenderState( D3DRS_ZWRITEENABLE,FALSE );
		d3dDev->SetRenderState( D3DRS_ZENABLE,D3DZB_FALSE );
		//
		d3dDev->SetRenderState( D3DRS_CULLMODE,D3DCULL_NONE );
		//
		break;
	default:
		Error( "TODO" );
	}
}

void CDriver::SetShader( CObject *shader ){
	if( shader ) shader->Retain();
	if( _shader ) _shader->Release();
	if( _shader=(D3D9Shader*)shader ) _shader->Bind();
	++paramSeq;
}

void CDriver::SetVertexBuffer( CObject *vbuffer ){
	if( vbuffer ) vbuffer->Retain();
	if( _vbuffer ) _vbuffer->Release();
	if( _vbuffer=(D3D9VertexBuffer*)vbuffer ) _vbuffer->Bind();
}

void CDriver::SetIndexBuffer( CObject *ibuffer ){
	if( ibuffer ) ibuffer->Retain();
	if( _ibuffer ) _ibuffer->Release();
	if( _ibuffer=(D3D9IndexBuffer*)ibuffer ) _ibuffer->Bind();
}

void CDriver::Render( int what,int first,int count ){

	D3DPRIMITIVETYPE prim;
	switch( what ){
	case 1:prim=D3DPT_POINTLIST;break;
	case 2:prim=D3DPT_LINELIST;first/=2;count/=2;break;
	case 3:prim=D3DPT_TRIANGLELIST;first/=3;count/=3;break;
	default:Error( "CDriver::Render - invalid primitive type" );return;
	}

	_shader->Validate();

	if( _ibuffer ){
		HRESULT hr=d3dDev->DrawIndexedPrimitive( prim,0,0,_vbuffer->Capacity(),first,count );
		if( hr<0 ){
			cout<<DXGetErrorDescription( hr )<<endl;
			cout<<_vbuffer->Capacity()<<' '<<first<<' '<<count<<endl;
			Error( "DrawIndexedPrimitive failed" );
		}
	}else{
		if( d3dDev->DrawPrimitive( prim,first,count )<0 ){
			Error( "DrawPrimitive failed" );
		}
	}
}

void CDriver::BeginScene(){
	d3dDev->BeginScene();
}

void CDriver::EndScene(){
	d3dDev->EndScene();
	d3dDev->Present( 0,0,0,0 );
}

static class CCDriverInit : public CInit{
public:
	CCDriverInit():CInit( "d3d9driver" ){
	}

	void OnInit(){
		d3d=Direct3DCreate9( 0x900 );
		if( !d3d ){
			Error( "Direct3DCreate9 failed" );
		}

		HWND hwnd=GetActiveWindow();
		if( !hwnd ){
			Error( "No HWND" );
		}

		D3DPRESENT_PARAMETERS &pp=presentParams;
		pp.BackBufferWidth=1024;
		pp.BackBufferHeight=768;
		pp.BackBufferFormat=D3DFMT_UNKNOWN;
		pp.BackBufferCount=1;
		pp.MultiSampleType=D3DMULTISAMPLE_NONE;
		pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
		pp.hDeviceWindow=hwnd;
		pp.Windowed=true;
		pp.EnableAutoDepthStencil=true;
		pp.AutoDepthStencilFormat=D3DFMT_D24S8;
		pp.FullScreen_RefreshRateInHz=0;
		pp.PresentationInterval=1;
		if( d3d->CreateDevice( 0,D3DDEVTYPE_HAL,hwnd,D3DCREATE_PUREDEVICE|D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&d3dDev )<0 ){
			Error( "CreateDevice failed" );
		}

		if( d3dDev->GetRenderTarget( 0,&defaultRenderTarget )<0 ){
			Error( "GetRenderTarget failed" );
		}

		if( !defaultRenderTarget ){
			Error( "GetRenderTarget failed" );
		}
	}
}_init;

#endif

/*
static class CDriverInit : public CInit{
public:
	CDriverInit():CInit( "driver" ){
	}
	void OnInit(){

	}
};
*/
