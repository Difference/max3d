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

#ifndef OPENGLGRAPHICS_H
#define OPENGLGRAPHICS_H
/// @file

#include "graphics.h"

class GLShaderProg;
class GLFrameBuffer;

/// OpenGL graphics implementation
class COpenGLGraphics : public CGraphics{
public:
	COpenGLGraphics();

	virtual CVertexBuffer *CreateVertexBuffer( int capacity,string format );
	virtual CIndexBuffer *CreateIndexBuffer( int capacity,string format );
	virtual CTexture *CreateTexture( int width,int height,int format,int flags );
	virtual CTexture *Create3dTexture( int width,int height,int depth,int format,int flags );
	virtual CTexture *CreateCubeTexture( int size,int format,int flags );
	virtual CShader *CreateShader( string source );

	virtual void BeginScene();
	virtual void SetColorBuffer( int index,CTexture *texture );
	virtual void SetDepthBuffer( CTexture *texture );
	virtual void SetViewport( const CRect &viewport );
	virtual void SetShaderMode( int mode );
	virtual void SetWriteMask( int mask );
	virtual void SetBlendFunc( int src,int dst );
	virtual void SetDepthFunc( int func );
	virtual void SetCullMode( int mode );
	virtual void SetShader( CShader *shader );
	virtual void SetVertexBuffer( CVertexBuffer *buffer );
	virtual void SetIndexBuffer( CIndexBuffer *buffer );
	virtual void SetClipPlane( int index,const float params[4] );
	virtual void Clear();
	virtual void Render( int what,int first,int count,int instances );
	virtual void EndScene();

private:
	void ValidateShaderProg();
	void ValidateFrameBuffer();

	int _dirty;
	GLShaderProg *_shaderProg;
	vector<GLFrameBuffer*> _frameBufs;
};

#endif
