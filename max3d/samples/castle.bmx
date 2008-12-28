		
Strict

Import Max3d.Max3d

Max3dGraphics 1280,1024,0,60'32,60

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

Local light1=CreateDistantLight()
TurnEntity light1,0,45,0
'SetLightColor light1,.25,.25,.5

Rem
Local light2=CreateSpotLight()
MoveEntity light2,0,10,10
TurnEntity light2,0,90,0
SetLightRange light2,20
SetLightColor light2,3,3,3
End Rem

Rem
Local joint=CreatePivot()
Local box=CreateBox( green,3,.75,2,2,1 )
SetEntityParent box,joint
MoveEntity box,0,1,0
Local caps=CreateCapsule( yellow,.5,3,2,1 )
SetEntityParent caps,joint
MoveEntity caps,0,-2,0
CreateBallJoint joint,box,caps
HideEntity joint
For Local x=-25 To 25 Step 5
	For Local z=-25 To 25 Step 5
		Local copy=CopyEntity( joint )
		MoveEntity copy,x,40,z
	Next
Next
DestroyEntity joint
End Rem

Rem
Local tex=LoadTexture( "bluspark.bmp" )
Local sprite_mat=CreateMaterial()
SetMaterialColor sprite_mat,"SpriteColor",1,1,1
SetMaterialTexture sprite_mat,"SpriteTexture",tex

For Local i=0 Until 1
	Local sprite=CreateSprite( sprite_mat )
	MoveEntity sprite,Rnd(-50,50),Rnd(10,20),Rnd(-50,50)
	CreateSphereBody sprite,.5,3,1
	Local light=CreatePointLight()
	SetEntityParent light,sprite
	SetLightRange light,5
	SetLightShadowMask light,0
Next
End Rem

Local castle=LoadModel( "CASTLE1.X",4,0 )
MoveEntity castle,0,.5,0

Local house=LoadModel( "elvenhouse.b3d",4,0 )
MoveEntity house,0,2,0
TurnEntity house,180,0,0
For Local x=-90 To 90 Step 45
	For Local z=-90 To 90 Step 45
		If Not x And Not z Continue
		Local copy=CopyEntity( house )
		MoveEntity copy,x,0,z
	Next
Next
DestroyEntity house

'Rem
'End Rem

Rem
For Local x=-80 To 80 Step 20
	For Local z=-80 To 80 Step 20
		Local rgb![]=[Rnd(),Rnd(),Rnd()]
		rgb[Rand(0,2)]=1
		Local light=CreateLight()
		SetLightColor light,rgb[0],rgb[1],rgb[2]
		SetLightRange light,20
		MoveEntity light,x,5,z
		SetLightShadowMask light,0
	Next
Next
'End Rem

'Rem
For Local x=0 To 0'-40 To 40 Step 40
	For Local z=0 To 0 '-40 To 40 Step 40
		Local rgb![]=[Rnd(),Rnd(),Rnd()]
		rgb[Rand(0,2)]=1
		Local light=CreateLight()
		SetLightColor light,rgb[0],rgb[1],rgb[2]
		SetLightRange light,20
		MoveEntity light,x,5,z
		SetLightShadowMask light,0
	Next
Next
End Rem

Local player=CreateCapsule( blue,.5,2,1,-1 )
MoveEntity player,0,5,-10

Local camera=CreateCamera()
SetCameraViewport camera,(1280-1024)/2,(1024-768)/2,1024,768
SetEntityParent camera,player
MoveEntity camera,0,1,0

Local yvel#

Local update_ms,total_ms

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

	yvel:-9.81/60/20
	
	Local y#=EntityY( player )
	
	MoveEntity player,0,yvel,0
	
	update_ms=MilliSecs()
	UpdateWorld
	Local update_ms=MilliSecs()-update_ms
	
	yvel=EntityY( player )-y
		
	If KeyHit( KEY_SPACE )
		yvel=.2
	EndIf
	
	RenderWorld
	Flip

Wend
