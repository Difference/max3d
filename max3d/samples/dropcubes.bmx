
Strict

Import bmx3d.max3d

Max3dGraphics 1024,768

DisableShadows

EnableCollisions 1,1,1000,0,0
EnableCollisions 1,10,1000,0,0

'SetAmbientColor 1,1,1

Local light=CreateDistantLight()
TurnEntity light,0,60,0

Local white=CreateMaterial()
SetMaterialColor white,"DiffuseColor",1,1,1

Local green=CreateMaterial()
SetMaterialColor green,"DiffuseColor",0,1,0

Local yellow=CreateMaterial()
SetMaterialColor yellow,"DiffuseColor",1,1,0

Local shape=CreateBox( green,1,1,1,1,1 )
HideEntity shape

Local ground=CreateBox( yellow,50,1,50,10,0 )

Local camera=CreateCamera()

Local yaw#,pitch#=45,zoom#=-50,rot#=0

While Not KeyHit( KEY_ESCAPE )

	If KeyDown( KEY_SPACE )
		For Local i=0 Until 3
			rot:+25
			Local copy=CopyEntity( shape )
			TurnEntity copy,rot,0,0
			MoveEntity copy,0,Cos( MilliSecs()/3 )*3+15,Sin( MilliSecs() )*6+12;
			Global n
			n:+1
'			Print n
		Next
	EndIf

	If KeyDown( KEY_LEFT )
		yaw:-1
	Else If KeyDown( KEY_RIGHT )
		yaw:+1
	EndIf
	
	If KeyDown( KEY_UP )
		pitch:+1
	Else If KeyDown( KEY_DOWN )
		pitch:-1
	EndIf

	If KeyDown( KEY_A )
		zoom:+.1
	Else If KeyDown( KEY_Z )
		zoom:-.1
	EndIf
	
	SetEntityTranslation camera,0,0,0
	SetEntityRotation camera,yaw,pitch,0
	MoveEntity camera,0,0,zoom
	
	UpdateWorld
	RenderWorld
	
	Flip 
	
Wend

