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

#ifndef LIGHT_H
#define LIGHT_H

#include "entity.h"

#include "graphics.h"

class CLight : public CEntity{
public:
	CLight();
	~CLight();
	
	void SetShader( CShader *shader );
	CShader *Shader(){ return _shader; }
	
	void SetAngle( float angle );
	float Angle(){ return _angle; }
	
	void SetRange( float range );
	float Range(){ return _range; }

	void SetColor( const CVec3 &color );
	const CVec3 &Color(){ return _color; }

	void SetTexture( CTexture *texture );
	CTexture *Texture(){ return _texture; }
	
	void SetShadowSize( int size );
	int ShadowSize(){ return _shadowSize; }

	void SetShadowSplits( const vector<float> &splits );
	const vector<float> &ShadowSplits(){ return _shadowSplits; }
	
	virtual void OnRenderWorld();
	
	static vector<float> ComputeShadowSplits( int count,float znear,float zfar,float blend );

private:
	CLight( CLight *light,CCopier *copier );
	
	CLight *OnCopy( CCopier *copier ){ return new CLight( this,copier ); }

	float _angle;
	float _range;
	CVec3 _color;
	CShader *_shader;
	CTexture *_texture;
	int _shadowSize;
	vector<float> _shadowSplits;
};

#endif
