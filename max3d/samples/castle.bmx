		
Strict

Import Bmx3d.Max3d

Incbin "fogshader.glsl"

Max3dGraphics 1024,768

'DisableShadows

SetClearColor 0,.5,1

SetAmbientColor .25,.25,.25'1,1,1'.01,.01,.01

Local fogShader=CreateShader( LoadString( "incbin::fogshader.glsl" ) )
Local fogMaterial=CreateMaterial()

SetMaterialFloat fogMaterial,"FogStart",25
SetMaterialFloat fogMaterial,"FogEnd",50
SetMaterialColor fogMaterial,"FogColor",.5,.75,1

AddRenderPass fogShader,fogMaterial

'collision types:
'1=player
'2=dynamic scenery
'3=lift
'4=static scenery

EnableCollisions 1,2,0,0,1
EnableCollisions 1,3,0,0,1
EnableCollisions 1,4,0,0,1

EnableCollisions 2,2,1,0,0
EnableCollisions 3,4,1,1,0
EnableCollisions 2,4,1,0,0

Local white=CreateMaterial()
SetMaterialColor white,"DiffuseColor",1,1,1

Local green=CreateMaterial()
SetMaterialColor green,"DiffuseColor",0,1,0

Local yellow=CreateMaterial()
SetMaterialColor yellow,"DiffuseColor",1,1,0

Local blue=CreateMaterial()
SetMaterialColor blue,"DiffuseColor",0,0,1

Local grass=LoadMaterial( "grass.jpg" )
Local ground=CreateBox( grass,100,1,100,4,0 )
ScaleModelTexCoords ground,20,20

Local light=CreateDistantLight()
TurnEntity light,30,60,0

Local splits#[]=[1.0,4.0,16.0,64.0,256.0]
SetLightShadowSplitsTable light,5,splits

Local castle=LoadModel( "CASTLE1.X",4,0 )
MoveEntity castle,0,.5,0

'Rem
For Local i=0 Until CountModelSurfaces( castle )
	Local surface=GetModelSurface( castle,i )
	Local material=GetSurfaceMaterial( surface )
	Select GetMaterialName( material )
	Case "x3ds_mat_castlestone"
		SetSurfaceMaterial surface,LoadMaterial( "stonefloor.jpg" )
	Case "x3ds_mat_Material__3"
	Case "x3ds_mat_shingle"
	Case "x3ds_mat_Material__8"
		SetSurfaceMaterial surface,LoadMaterial( "eurofan.png" )
	End Select
Next
'End Rem

Local player=CreateCapsule( blue,.5,2,1,-1 )
MoveEntity player,0,5,-10

Local camera=CreateCamera()
SetEntityParent camera,player
MoveEntity camera,0,1,0

'Rem
Local mirror=CreateMirror()
MoveEntity mirror,0,3.25,8.5
SetMirrorSize mirror,4,2
SetMirrorResolution mirror,512,512
'End Rem

Local yvel#

While Not KeyHit( KEY_ESCAPE )

	If KeyDown( KEY_LEFT )
		TurnEntity player,3,0,0
	Else If KeyDown( KEY_RIGHT )
		TurnEntity player,-3,0,0
	EndIf
	
	If KeyDown( KEY_A )
		MoveEntity player,0,0,.1
	Else If KeyDown( KEY_Z )
		MoveEntity player,0,0,-.1
	EndIf
	
	If KeyDown( KEY_UP )
		TurnEntity camera,0,3,0
	Else If KeyDown( KEY_DOWN )
		TurnEntity camera,0,-3,0
	EndIf

	If KeyHit( KEY_SPACE )
		yvel=.2
	EndIf

	yvel:-9.81/60/60
	Local y#=EntityY( player )
	MoveEntity player,0,yvel,0
	UpdateWorld
	yvel=EntityY( player )-y
	
	RenderWorld
	Flip

Wend
