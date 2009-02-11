		
Strict

Import Bmx3d.Max3d

Max3dGraphics 1024,768

SetClearColor .25,.5,1
SetAmbientColor .25,.25,.25

'fog
Local fogMat=CreateMaterial()
SetMaterialFloat fogMat,"FogStart",48
SetMaterialFloat fogMat,"FogEnd",64
SetMaterialColor fogMat,"FogColor",.25,.5,1	'same as clear color...
AddRenderPass LoadShader( "linearfog.glsl" ),fogMat

'god rays
Local godMat=CreateMaterial()
SetMaterialColor godMat,"GodRaysColor",1,1,0
AddRenderPass LoadShader( "godrays.glsl" ),godMat

'blur
Local blurMat=CreateMaterial()
SetMaterialFloat blurMat,"BlurStrength",1
AddRenderPass LoadShader( "blur.glsl" ),blurMat

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
Local ground=CreateBox( grass,200,1,200,4,0 )
ScaleModelTexCoords ground,50,50
MoveEntity ground,0,-.5,0

Local light=CreateDistantLight()
Local lightYaw#=45+90,lightPitch#=45,lightRoll#
TurnEntity light,lightYaw,lightPitch,lightRoll

Local splits#[]=[0.0,4.0,16.0,64.0]
SetLightShadowSplitsTable light,4,splits

Local castle=LoadModel( "CASTLE1.X",0,0 )
'Local castle=LoadModel( "elvenhouse.b3d",0,0 )
SetEntityScale castle,.035,.035,.035
ResetModelTransform castle
CreateModelBody castle,castle,4,0

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

Local player=CreateCapsule( blue,.5,2,1,-1 )
MoveEntity player,0,5,-10

Local camera=CreateCamera()
SetCameraPerspective camera,90,1.333,.1,256
SetCameraViewport camera,0,0,1024,768
SetEntityParent camera,player
MoveEntity camera,0,1,0

Local bluspark=CreateMaterial()
SetMaterialColor bluspark,"SpriteColor",1,1,1
SetMaterialTexture bluspark,"SpriteTexture",LoadTexture( "bluspark.bmp" )
Local sprite=CreateSprite( bluspark )

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
	
	Local cx#=EntityMatrixElement( camera,3,0 )
	Local cy#=EntityMatrixElement( camera,3,1 )
	Local cz#=EntityMatrixElement( camera,3,2 )
	
	SetEntityTranslation sprite,cx,cy,cz
	SetEntityRotation sprite,lightYaw,lightPitch,lightRoll
	MoveEntity sprite,0,0,-20
	
	Local sx#=EntityMatrixElement( sprite,3,0 )
	Local sy#=EntityMatrixElement( sprite,3,1 )
	Local sz#=EntityMatrixElement( sprite,3,2 )

	If CameraProject( camera,sx,sy,sz )
		Local x#=ProjectedX(),y#=ProjectedY()
		Local dx#=x-512,dy#=y-384
		Local d#=Sqr( dx*dx+dy*dy )/384.0
		If d>1 d=1
		SetMaterialFloat godMat,"GodRaysExposure",(1-d)*.5
		SetMaterialFloat godMat,"GodRaysLightX",ProjectedX()
		SetMaterialFloat godMat,"GodRaysLightY",ProjectedY()
	Else
		SetMaterialFloat godMat,"GodRaysExposure",0
		SetMaterialFloat godMat,"GodRaysLightX",0
		SetMaterialFloat godMat,"GodRaysLightY",0
	EndIf
	
	RenderWorld
	
	Flip
	
Wend
