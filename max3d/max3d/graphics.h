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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "resource.h"

#define USE_GL_TEXTURE_RECTANGLE_ARB

class CParam;

/// Abstract base class for vertex buffers
class CVertexBuffer : public CResource{
public:

	virtual void SetData( const void *data )=0;
	virtual void *Lock()=0;
	virtual void Unlock()=0;

	int Capacity(){ return _capacity; }
	string Format(){ return _format; }

protected:
	CVertexBuffer(){}

	int _capacity;
	string _format;
};

/// Abstract base class for index buffers
class CIndexBuffer : public CResource{
public:

	virtual void SetData( const void *data )=0;
	virtual void *Lock()=0;
	virtual void Unlock()=0;

	int Capacity(){ return _capacity; }
	string Format(){ return _format; }

protected:
	CIndexBuffer(){}

	int _capacity;
	string _format;
};

/// Abstract base class for 2d/3d/cube textures
class CTexture : public CResource{
public:

	virtual void SetData( const void *data )=0;
	virtual void Set3dData( const void *data )=0;
	virtual void SetCubeData( const void *data )=0;
	
	int Width(){ return _width; }
	int Height(){ return _height; }
	int Depth(){ return _depth; }
	int Format(){ return _format; }
	int Flags(){ return _flags; }

protected:
	CTexture(){}

	int _width;
	int _height;
	int _depth;
	int _format;
	int _flags;
};

/// Abstract base class for shader objects
class CShader : public CResource{
public:
	int ModeMask(){ return _modeMask; }
	
	const vector<CParam*> &Params(){ return _params; }
	
	static int ModeForName( string name );

protected:
	CShader(){}

	int _modeMask;
	vector<CParam*> _params;
};

/// Function to return computed float values for shader params
typedef const float *(*FloatParamFunc)();

/// Helper shader param class
class CParam{
public:

	void SetFloatValue( int count,const float *value );
	const float *FloatValue();

	void SetFloatFunc( int count,FloatParamFunc func,const vector<string> &uses );
	FloatParamFunc FloatFunc();

	void SetTextureValue( CTexture *value );
	CTexture *TextureValue();

	string Name(){ return _name; }
	
	int Seq(){ return _seq; }
	
	int Count(){ return _count; }

	bool Validate( int seq ){
		if( seq==_seq ) return true;
		_seq=seq;
		return false;
	}

	static CParam *ForName( string name );

protected:
	CParam( string name );

private:
	void Invalidate();

	string _name;
	int _seq;					//for validation
	int _count;					//how many floats
	float *_floats;				//the floats
	CTexture *_texture;			//a texture
	vector<CParam*> _defs;		//for _floatFunc
	FloatParamFunc _floatFunc;
};

/// Abstract graphics base class
class CGraphics : public CResource{
public:
	CGraphics();

	// Factory stuff
	virtual CVertexBuffer *CreateVertexBuffer( int capacity,string format )=0;
	virtual CIndexBuffer *CreateIndexBuffer( int capacity,string format )=0;
	virtual CTexture *CreateTexture( int width,int height,int format,int flags )=0;
	virtual CTexture *Create3dTexture( int width,int height,int depth,int format,int flags )=0;
	virtual CTexture *CreateCubeTexture( int size,int format,int flags )=0;
	virtual CShader *CreateShader( string source )=0;

	// Rendering stuff
	//
	virtual void BeginScene()=0;
	virtual void SetColorBuffer( int index,CTexture *texture )=0;
	virtual void SetDepthBuffer( CTexture *texture )=0;
	virtual void SetViewport( const CRect &viewport )=0;
	virtual void SetShaderMode( int mode )=0;
	virtual void SetWriteMask( int mask )=0;
	virtual void SetBlendFunc( int src,int dst )=0;
	virtual void SetDepthFunc( int func )=0;
	virtual void SetCullMode( int mode )=0;
	virtual void SetShader( CShader *shader )=0;
	virtual void SetVertexBuffer( CVertexBuffer *buffer )=0;
	virtual void SetIndexBuffer( CIndexBuffer *buffer )=0;
	virtual void SetClipPlane( int index,const float params[4] )=0;
	virtual void Clear()=0;
	virtual void Render( int what,int first,int count,int instances )=0;
	virtual void EndScene()=0;

	// Helper stuff
	//
	void SetFloatParam( const char *name,float value ){ CParam::ForName(name)->SetFloatValue( 1,&value ); }
	void SetVec2Param( const char *name,const CVec2 &value ){ CParam::ForName(name)->SetFloatValue( 2,&value.x ); }
	void SetVec3Param( const char *name,const CVec3 &value ){ CParam::ForName(name)->SetFloatValue( 3,&value.x ); }
	void SetVec4Param( const char *name,const CVec4 &value ){ CParam::ForName(name)->SetFloatValue( 4,&value.x ); }
	void SetMat4Param( const char *name,const CMat4 &value ){ CParam::ForName(name)->SetFloatValue( 16,&value.i.x ); }
	void SetTextureParam( const char *name,CTexture *value ){ CParam::ForName(name)->SetTextureValue( value); }
	void SetFloatParamFunc( const char *name,int count,FloatParamFunc func,const vector<string> &uses ){ CParam::ForName(name)->SetFloatFunc( count,func,uses ); }

	float FloatParam( const char *name ){ return *CParam::ForName(name)->FloatValue(); }
	const CVec2 &Vec2Param( const char *name ){ return (const CVec2&)*CParam::ForName(name)->FloatValue(); }
	const CVec3 &Vec3Param( const char *name ){ return (const CVec3&)*CParam::ForName(name)->FloatValue(); }
	const CVec4 &Vec4Param( const char *name ){ return (const CVec4&)*CParam::ForName(name)->FloatValue(); }
	const CMat4 &Mat4Param( const char *name ){ return (const CMat4&)*CParam::ForName(name)->FloatValue(); }
	CTexture *TextureParam( const char *name ){ return CParam::ForName(name)->TextureValue(); }

	void AppendShaderHeader( string header ){ _shaderHeader+=header; }
	CTexture *ColorBuffer( int index ){ return _colorBuffers[index]; }
	CTexture *DepthBuffer(){ return _depthBuffer; }
	const CRect &Viewport(){ return _viewport; }
	CVertexBuffer *VertexBuffer(){ return _vertexBuffer; }
	CIndexBuffer *IndexBuffer(){ return _indexBuffer; }
	CShader *Shader(){ return _shader; }
	int ShaderMode(){ return _shaderMode; }
	int WindowWidth(){ return _windowWidth; }
	int WindowHeight(){ return _windowHeight; }

protected:
	string _shaderHeader;
	CTexture *_colorBuffers[4];
	CTexture *_depthBuffer;
	CRect _viewport;
	CVertexBuffer *_vertexBuffer;
	CIndexBuffer *_indexBuffer;
	CShader *_shader;
	int _shaderMode;
	int _windowWidth;
	int _windowHeight;
};

enum{
	TEXTURE_FILTER=0x1,
	TEXTURE_MIPMAP=0x2,
	TEXTURE_CLAMPS=0x4,
	TEXTURE_CLAMPT=0x8,
	TEXTURE_RENDER=0x10,	//supports render-to
	TEXTURE_STATIC=0x20,	//texture contents managed by driver
	TEXTURE_RECTANGULAR=0x40,
	TEXTURE_CLAMPST=TEXTURE_CLAMPS|TEXTURE_CLAMPT
};

enum{
	FORMAT_A8=1,
	FORMAT_I8=2,
	FORMAT_L8=3,
	FORMAT_LA8=4,
	FORMAT_RGB8=5,
	FORMAT_RGBA8=6,
	FORMAT_RGB10A2=7,
	FORMAT_RGBA16F=16,
	FORMAT_DEPTH=32,
};

enum{
	WRITEMASK_RED=1,
	WRITEMASK_GREEN=2,
	WRITEMASK_BLUE=4,
	WRITEMASK_ALPHA=8,
	WRITEMASK_DEPTH=16
};

enum{
	BLENDFUNC_ZERO=0,
	BLENDFUNC_ONE=1,
	BLENDFUNC_SRCALPHA=2,
	BLENDFUNC_DSTALPHA=3
};

enum{
	DEPTHFUNC_F=0,
	DEPTHFUNC_LT=1,
	DEPTHFUNC_EQ=2,
	DEPTHFUNC_LE=3,
	DEPTHFUNC_GT=4,
	DEPTHFUNC_NE=5,
	DEPTHFUNC_GE=6,
	DEPTHFUNC_T=7
};

enum{
	CULLMODE_NONE=0,
	CULLMODE_BACK=1,
	CULLMODE_FRONT=2
};

#endif
