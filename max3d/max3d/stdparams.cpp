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

#include "app.h"
#include "stdparams.h"

static const char *shaderHeader=
"//@common\n"
//Constant vars
"uniform vec2 bb_WindowSize;\n"
"uniform vec2 bb_WindowScale;\n"
//"uniform sampler2DRect bb_AccumBuffer;\n"
"uniform sampler2DRect bb_ColorBuffer;\n"
"uniform sampler2DRect bb_MaterialBuffer;\n"
"uniform sampler2DRect bb_NormalBuffer;\n"
"uniform sampler2DRect bb_DepthBuffer;\n"
"uniform sampler2DRect bb_QuadTexture;\n"
"uniform sampler2D bb_ShadowBuffer;\n"
//World vars
"uniform vec3 bb_AmbientColor;\n"
//Camera vars
"uniform mat4 bb_ProjectionMatrix;\n"
"uniform mat4 bb_CameraMatrix;\n"
"uniform mat4 bb_ViewMatrix;\n"
"uniform mat4 bb_ViewProjectionMatrix;\n"
"uniform vec2 bb_ViewportSize;\n"
"uniform vec2 bb_ViewportScale;\n"
"uniform vec2 bb_FragScale;\n"
"uniform vec2 bb_FragOffset;\n"
"uniform float bb_zNear;\n"
"uniform float bb_zFar;\n"
//Light vars
"uniform mat4 bb_LightMatrix;\n"
"uniform mat4 bb_InverseLightMatrix;\n"
"uniform mat4 bb_ShadowMatrix;\n"
"uniform mat4 bb_ViewLightMatrix;\n"
"uniform mat4 bb_ViewShadowMatrix;\n"
"uniform vec3 bb_ViewSpaceLightVector;\n"
"uniform vec3 bb_ViewSpaceLightPosition;\n"
"uniform vec3 bb_ViewSpaceLightDirection;\n"
"uniform float bb_LightAngle;\n"
"uniform float bb_LightRange;\n"
"uniform vec3 bb_LightColor;\n"
"uniform sampler2D bb_LightTexture;\n"
//Shadow map vars
"uniform float bb_ShadowMapScale;\n"
"uniform float bb_ShadowNearClip;\n"
"uniform float bb_ShadowFarClip;\n"
//Model vars
"uniform mat4 bb_ModelMatrices[48];\n"
"uniform mat4 bb_ModelMatrix;\n"
"uniform mat4 bb_ModelViewMatrix;\n"
"uniform mat4 bb_ModelViewProjectionMatrix;\n"
"uniform mat4 bb_ModelLightMatrix;\n"
"uniform mat4 bb_ModelShadowMatrix;\n"
//Vertex attributes
"//@vertex\n"
"attribute vec4 bb_Vertex;\n"
"attribute vec3 bb_Normal;\n"
"attribute vec4 bb_Tangent;\n"
"attribute vec4 bb_TexCoords0;\n"
"attribute vec4 bb_TexCoords1;\n"
"attribute vec4 bb_Weights;\n"
"attribute vec4 bb_Bones;\n";

static const float *ViewMatrix(){
	static CMat4 _mat;
//	cout<<"Validating ViewMatrix"<<endl;
	_mat=-App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	return (float*)&_mat;
}

static const float *ViewProjectionMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_ProjectionMatrix" ) * App.Graphics()->Mat4Param( "bb_ViewMatrix" );
	return (float*)&_mat;
}

static const float *InverseLightMatrix(){
	static CMat4 _mat;
	_mat=-App.Graphics()->Mat4Param( "bb_LightMatrix" );
	return (float*)&_mat;
}

static const float *ShadowMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_ShadowProjectionMatrix" ) * App.Graphics()->Mat4Param( "bb_InverseLightMatrix" );
	return (float*)&_mat;
}

static const float *ViewShadowMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_ShadowMatrix" ) * App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	return (float*)&_mat;
}

static const float *ViewLightMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_InverseLightMatrix" ) * App.Graphics()->Mat4Param( "bb_CameraMatrix" );
	return (float*)&_mat;
}

static const float *ViewSpaceLightVector(){
	static CVec3 _vec;
	_vec=(App.Graphics()->Mat4Param( "bb_ViewMatrix" ) * App.Graphics()->Mat4Param( "bb_LightMatrix" ).k).xyz();
	_vec=-_vec.Normalize();
	return (float*)&_vec;
}

static const float *ViewSpaceLightPosition(){
	static CVec3 _vec;
	_vec=(App.Graphics()->Mat4Param( "bb_ViewMatrix" ) * App.Graphics()->Mat4Param( "bb_LightMatrix" ).t).xyz();
	return (float*)&_vec;
}

static const float *ViewSpaceLightDirection(){
	static CVec3 _vec;
	_vec=(App.Graphics()->Mat4Param( "bb_ViewMatrix" ) * App.Graphics()->Mat4Param( "bb_LightMatrix" ).k).xyz();
	_vec=_vec.Normalize();
	return (float*)&_vec;
}

static const float *ModelViewMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_ViewMatrix" ) * App.Graphics()->Mat4Param( "bb_ModelMatrix" );
	return (float*)&_mat;
}

static const float *ModelViewProjectionMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_ViewProjectionMatrix" ) * App.Graphics()->Mat4Param( "bb_ModelMatrix" );
	return (float*)&_mat;
}

static const float *ModelLightMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_InverseLightMatrix" ) * App.Graphics()->Mat4Param( "bb_ModelMatrix" );
	return (float*)&_mat;
}

static const float *ModelShadowMatrix(){
	static CMat4 _mat;
	_mat=App.Graphics()->Mat4Param( "bb_ShadowMatrix" ) * App.Graphics()->Mat4Param( "bb_ModelMatrix" );
	return (float*)&_mat;
}

void stdparams_init(){
	App.Graphics()->AppendShaderHeader( shaderHeader );

	vector<string> uses;

	uses.clear();
	uses.push_back( "bb_CameraMatrix" );
	CParam::ForName( "bb_ViewMatrix" )->SetFloatFunc( 16,ViewMatrix,uses );

	uses.clear();
	uses.push_back( "bb_ViewMatrix" );
	uses.push_back( "bb_ProjectionMatrix" );
	CParam::ForName( "bb_ViewProjectionMatrix" )->SetFloatFunc( 16,ViewProjectionMatrix,uses );

	uses.clear();
	uses.push_back( "bb_LightMatrix" );
	CParam::ForName( "bb_InverseLightMatrix" )->SetFloatFunc( 16,InverseLightMatrix,uses );

	uses.clear();
	uses.push_back( "bb_InverseLightMatrix" );
	uses.push_back( "bb_ShadowProjectionMatrix" );
	CParam::ForName( "bb_ShadowMatrix" )->SetFloatFunc( 16,ShadowMatrix,uses );

	uses.clear();
	uses.push_back( "bb_CameraMatrix" );
	uses.push_back( "bb_InverseLightMatrix" );
	CParam::ForName( "bb_ViewLightMatrix" )->SetFloatFunc( 16,ViewLightMatrix,uses );

	uses.clear();
	uses.push_back( "bb_CameraMatrix" );
	uses.push_back( "bb_ShadowMatrix" );
	CParam::ForName( "bb_ViewShadowMatrix" )->SetFloatFunc( 16,ViewShadowMatrix,uses );

	uses.clear();
	uses.push_back( "bb_ViewMatrix" );
	uses.push_back( "bb_LightMatrix" );
	CParam::ForName( "bb_ViewSpaceLightVector" )->SetFloatFunc( 3,ViewSpaceLightVector,uses );

	uses.clear();
	uses.push_back( "bb_ViewMatrix" );
	uses.push_back( "bb_LightMatrix" );
	CParam::ForName( "bb_ViewSpaceLightPosition" )->SetFloatFunc( 3,ViewSpaceLightPosition,uses );

	uses.clear();
	uses.push_back( "bb_ViewMatrix" );
	uses.push_back( "bb_LightMatrix" );
	CParam::ForName( "bb_ViewSpaceLightDirection" )->SetFloatFunc( 3,ViewSpaceLightDirection,uses );

	uses.clear();
	uses.push_back( "bb_ModelMatrix" );
	uses.push_back( "bb_ViewMatrix" );
	CParam::ForName( "bb_ModelViewMatrix" )->SetFloatFunc( 16,ModelViewMatrix,uses );

	uses.clear();
	uses.push_back( "bb_ModelMatrix" );
	uses.push_back( "bb_ViewProjectionMatrix" );
	CParam::ForName( "bb_ModelViewProjectionMatrix" )->SetFloatFunc( 16,ModelViewProjectionMatrix,uses );

	uses.clear();
	uses.push_back( "bb_ModelMatrix" );
	uses.push_back( "bb_InverseLightMatrix" );
	CParam::ForName( "bb_ModelLightMatrix" )->SetFloatFunc( 16,ModelLightMatrix,uses );

	uses.clear();
	uses.push_back( "bb_ModelMatrix" );
	uses.push_back( "bb_ShadowMatrix" );
	CParam::ForName( "bb_ModelShadowMatrix" )->SetFloatFunc( 16,ModelShadowMatrix,uses );
}
