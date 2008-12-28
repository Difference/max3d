
#ifndef DRIVER_H
#define DRIVER_H

#include "object.h"

enum{
	DRIVER_OPENGL=1,
	DRIVER_DIRECT3D9=2
};

enum{
	TEXTURE_FILTER=0x1,
	TEXTURE_MIPMAP=0x2,
	TEXTURE_CLAMPS=0x4,
	TEXTURE_CLAMPT=0x8,
	TEXTURE_RENDER=0x10,	//supports render-to
	TEXTURE_STATIC=0x20,	//texture contents managed by driver
	TEXTURE_CLAMPST=TEXTURE_CLAMPS|TEXTURE_CLAMPT
};

enum{
	FORMAT_RGB8=0x13,
	FORMAT_ARGB8=0x14,
	FORMAT_R16F=0x21,
	FORMAT_ARGB16F=0x24,
	FORMAT_DEPTHBUF=0x100,
	FORMAT_SHADOWMAP=0x200
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

typedef const float *(*FloatParamFunc)();

class CDriver{
public:
	CDriver();

	int Type();
	
	CObject *CreateVertexBuffer( int capacity,string format,const void *data );
	void SetVertexBufferData( CObject *buffer,const void *data );
	void *LockVertexBuffer( CObject *buffer );
	void UnlockVertexBuffer( CObject *buffer );
	void SetVertexBufferAttrib( CObject *buffer,int vertex,int attrib,float x,float y,float z,float w );

	CObject *CreateIndexBuffer( int capacity,string format,const void *data );
	void SetIndexBufferData( CObject *buffer,const void *data );
	void *LockIndexBuffer( CObject *buffer );
	void UnlockIndexBuffer( CObject *buffer );
	void SetIndexBufferIndex( CObject *buffer,int index,int value );
	
	CObject *Create2dTexture( int width,int height,int format,int flags,const void *data );
	void Set2dTextureData( CObject *texture,const void *data );
	
	CObject *Create3dTexture( int width,int height,int depth,int format,int flags,const void *data );
	void Set3dTextureData( CObject *texture,const void *data );
	
	CObject *CreateCubeTexture( int size,int format,int flags,const void *data );
	void SetCubeTextureData( CObject *texture,const void *data );

	void AddShaderHeader( string source );

	CObject *CreateShader( string source );
	int ShaderMask( CObject *shader );

	CObject *CreateFrameBuffer( CObject *color0,CObject *color1,CObject *color2,CObject *color3,CObject *depth );

	int ParamId( string name );
	int ShaderMode( string name );	//0,1,2...31
	
	void SetFloatParamFunc( string name,int count,FloatParamFunc func,const vector<string> &uses );

	void SetFloatParam( int id,int count,const float *value );
	void SetFloatParam( string name,int count,const float *value );
	const float *FloatParam( int id );
	const float *FloatParam( string name );

	void SetTextureParam( int id,CObject *texture );
	void SetTextureParam( string name,CObject *texture );
	CObject *TextureParam( int id );
	CObject *TextureParam( string name );

	void BeginScene();

	void SetShaderMode( int mode );
	void SetWriteMask( int mask );
	void SetBlendFunc( int src,int dst );
	void SetDepthFunc( int func );
	void SetCullMode( int mode );

	void SetFrameBuffer( CObject *buffer );
	void SetViewport( int x,int y,int width,int height );
	void SetClipPlane( int index,const float *params );
	void SetShader( CObject *shader );
	void SetVertexBuffer( CObject *buffer );
	void SetIndexBuffer( CObject *buffer );
	void Clear();
	void Render( int what,int first,int count );
	void EndScene();

};

#endif
