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

#ifndef APP_H
#define APP_H

#include "graphics.h"
#include "scene.h"
#include "world.h"
#include "textureutil.h"
#include "shaderutil.h"
#include "modelutil.h"

typedef CObject *(*TObjectImporter)( const char *type,const char *path );

class CApp{
public:
	CApp();

	//Startup
	void Init( TObjectImporter importer );

	//graphics driver
	CGraphics *Graphics(){ return _graphics; }
	
	//Current Scene
	CScene *Scene(){ return _scene; }

	//Current world
	CWorld *World(){ return _world; }
	
	//txeture utilities
	CTextureUtil *TextureUtil(){ return _textureUtil; }

	//shader utilities
	CShaderUtil *ShaderUtil(){ return _shaderUtil; }

	//Model utilities
	CModelUtil *ModelUtil(){ return _modelUtil; }

	//Import an object
	CObject *ImportObject( string type,string path ){ return _importer( type.c_str(),path.c_str() ); }
	
private:
	TObjectImporter _importer;
	CGraphics *_graphics;
	CScene *_scene;
	CWorld *_world;
	CTextureUtil *_textureUtil;
	CShaderUtil *_shaderUtil;
	CModelUtil *_modelUtil;
};

extern CApp App;

#endif
