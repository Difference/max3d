		
Strict

Import Bmx3d.Max3d

Max3dGraphics 1024,768

DisableShadows

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

Local mossy=CreateMaterial()
SetMaterialTexture mossy,"DiffuseMap",LoadTexture( "eurofan_d.png" )
SetMaterialTexture mossy,"NormalMap",LoadTexture( "eurofan_local.png" )
SetMaterialTexture mossy,"SpecularMap",LoadTexture( "eurofan_s.png" )
Local ground=CreateBox( mossy,9.5,1,9.5,4,0 )
HideEntity ground

For Local x=-100 To 100 Step 10
	For Local z=-100 To 100 Step 10
		Local copy=CopyEntity( ground )
		MoveEntity copy,x,0,z
		If Abs(x)=100 Or Abs(z)=100 MoveEntity copy,0,1.5,0
	Next
Next
DestroyEntity ground

Local light=CreateDistantLight()
TurnEntity light,0,45,0

Local castle=LoadModel( "CASTLE1.X",4,0 )
MoveEntity castle,0,.5,0

Local player=CreateCapsule( blue,.5,2,1,-1 )
MoveEntity player,0,5,-10

Local camera=CreateCamera()
SetEntityParent camera,player
MoveEntity camera,0,1,0

Local mirror=CreateMirror()
MoveEntity mirror,0,3,6
SetMirrorSize mirror,2,2
SetMirrorResolution mirror,256,256

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
