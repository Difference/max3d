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

#ifndef SHADERUTIL_H
#define SHADERUTIL_H

#include "graphics.h"

class CShaderUtil{
public:
	CShaderUtil();
	
	CShader *ModelShader(){ return _modelShader; }
	CShader *SpriteShader(){ return _spriteShader; }
	CShader *TerrainShader(){ return _terrainShader; }
	CShader *MirrorShader(){ return _mirrorShader; }
	
	CShader *SpotLightShader(){ return _spotLightShader; }
	CShader *PointLightShader(){ return _pointLightShader; }
	CShader *DistantLightShader(){ return _distantLightShader; }
	
private:
	CShader *_modelShader;
	CShader *_spriteShader;
	CShader *_terrainShader;
	CShader *_mirrorShader;

	CShader *_spotLightShader;
	CShader *_pointLightShader;
	CShader *_distantLightShader;
};

#endif
