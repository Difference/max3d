
Strict

Import Max3d.Max3d

Max3dGraphics 800,600

SetClearColor 0,0,0

EnableCollisions 1,2,0,0,1

Local yellow=CreateMaterial()
SetMaterialColor yellow,"DiffuseColor",1,1,0

Local room_mat=CreateMaterial()
SetMaterialTexture room_mat,"DiffuseMap",LoadTexture( "stonefloor_d.jpg" )
SetMaterialTexture room_mat,"NormalMap",LoadTexture( "stonefloor_local.jpg" )
SetMaterialTexture room_mat,"SpecularMap",LoadTexture( "stonefloor_s.jpg" )
Local room=CreateBox( room_mat,10,10,10,0,0 )
FlipModel room

Local box_mat=CreateMaterial()
SetMaterialTexture box_mat,"DiffuseMap",LoadTexture( "eurofan_d.png" )
SetMaterialTexture box_mat,"NormalMap",LoadTexture( "eurofan_local.png" )
SetMaterialTexture box_mat,"SpecularMap",LoadTexture( "eurofan_s.png" )

Local pbody
For Local y=5 To 1 Step -1
	Local body=CreateCapsule( yellow,.1,.8,3,1 )
	MoveEntity body,0,-.4,0
	ResetModelTransform body
	MoveEntity body,0,y,0
	CreateBallJoint body,body,pbody
	pbody=body
Next
Local box=CreateBox( box_mat,1,1,1,2,1 )
MoveEntity box,0,-.5,0
ResetModelTransform box
CreateBallJoint box,box,pbody

Local light=CreateSpotLight()
MoveEntity light,0,0,-4
SetLightRange light,20
SetLightColor light,2,2,2

Local camera=CreateCamera()
CreateSphereBody camera,.5,1,-1

Local yaw#,pitch#,zoom#=-5

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
