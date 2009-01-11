		
Strict

Import Bmx3d.Max3d

Max3dGraphics 1024,768

'DisableShadows

SetAmbientColor .1,.1,.1

'collision types:
'1=player
'2=dynamic scenery
'3=lift
'4=static scenery

EnableCollisions 1,2,0,.75,1

'Rem
Local bluspark=CreateMaterial()
SetMaterialColor bluspark,"SpriteColor",1,1,1
SetMaterialTexture bluspark,"SpriteTexture",LoadTexture( "bluspark.bmp" )

Local sprite=CreateSprite( bluspark )
CreateSphereBody sprite,.01,1,1

'Rem
For Local i=0 Until 100
	Local copy=CopyEntity( sprite )
	TurnEntity copy,Rnd(360),0,0
	MoveEntity copy,0,Rnd(15,20),Rnd(5,10)
Next
DestroyEntity sprite
'End Rem

Local mossy=CreateMaterial()
SetMaterialTexture mossy,"DiffuseMap",LoadTexture( "eurofan_d.png" )
SetMaterialTexture mossy,"NormalMap",LoadTexture( "eurofan_local.png" )
SetMaterialTexture mossy,"SpecularMap",LoadTexture( "eurofan_s.png" )

Local ground=CreateBox( mossy,25,1,25,2,0 )
ScaleModelTexCoords ground,20,20

Local light=CreateSpotLight()
MoveEntity light,0,25,0
TurnEntity light,0,90,0
SetLightRange light,50
setlightshadowsize light,0

Local camera=CreateCamera()

Local yaw#,pitch#=30,zoom#=-5

While Not KeyHit( KEY_ESCAPE )

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
