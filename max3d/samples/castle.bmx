		
Strict

Import Bmx3d.Max3d

Max3dGraphics 1024,768,0,60,1

SetClearColor .75,.5,1

SetAmbientColor .25,.25,.25

'skybox
Local skymat=CreateMaterial()
SetMaterialFloat skymat,"FogStart",56
SetmaterialFloat skymat,"FogEnd",64
SetMaterialTexture skymat,"BackgroundTexture",LoadTexture( "dayfair.jpg" )

Local skybox=CreateSphere( skymat,64,0,0 )
Local surf=GetModelSurface( skybox,0 )
SetSurfaceShader surf,BackgroundShader()
FlipModel skybox
ScaleModelTexCoords skybox,.5,2

'Set up fog
Rem
Local fogShader=CreateShader( LoadString( "linearfog.glsl" ) )
Local fogMaterial=CreateMaterial()
SetMaterialFloat fogMaterial,"FogStart",48
SetMaterialFloat fogMaterial,"FogEnd",64
SetMaterialColor fogMaterial,"FogColor",.25,.5,1	'same as clear color...
AddRenderPass fogShader,fogMaterial
End Rem

'Rem
Local godShader=CreateShader( LoadString( "godrays.glsl" ) )
Local godMaterial=CreateMaterial()
SetMaterialColor godMaterial,"GodRaysColor",1,1,0
AddRenderPass godShader,godMaterial
'End Rem

Rem
Local blurShader=CreateShader( LoadString( "blur.glsl" ) )
Local blurMaterial=CreateMaterial()
SetMaterialFloat blurMaterial,"BlurStrength",0
'AddRenderPass blurShader,blurMaterial

'Local lineShader=CreateShader( LoadString( "outline.glsl" ) )
'Local lineMaterial=CreateMaterial()
'AddRenderPass lineShader,lineMaterial
End Rem
Local toneShader=LoadShader( "tonemap.glsl" )
Local toneMaterial=CreateMaterial()
AddRenderPass toneShader,toneMaterial

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

Rem
Local camera2=CreateCamera()
SetCameraViewport camera2,512-64,768-128,128,128
SetEntityParent camera2,player
MoveEntity camera2,0,20,0
TurnEntity camera2,0,90,0
End Rem

Local bluspark=CreateMaterial()
SetMaterialColor bluspark,"SpriteColor",1,1,1
SetMaterialTexture bluspark,"SpriteTexture",LoadTexture( "bluspark.bmp" )
Local sprite=CreateSprite( bluspark )

Rem
Local mirror=CreateMirror()
MoveEntity mirror,0,3.75,8.5
'SetMirrorSize mirror,8,4'4,2
'SetMirrorResolution mirror,512,512
End Rem

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
	
	SetEntityTranslation skybox,cx,cy,cz
	SetEntityRotation skybox,60,0,0
	
	SetEntityTranslation sprite,cx,cy,cz
	SetEntityRotation sprite,lightYaw,lightPitch,lightRoll
	MoveEntity sprite,0,0,-20
	
	Local sx#=EntityMatrixElement( sprite,3,0 )
	Local sy#=EntityMatrixElement( sprite,3,1 )
	Local sz#=EntityMatrixElement( sprite,3,2 )

'Rem	
	If CameraProject( camera,sx,sy,sz )
		Local x#=ProjectedX(),y#=ProjectedY()
		Local dx#=x-512,dy#=y-384
		Local d#=Sqr( dx*dx+dy*dy )/384.0
		If d>1 d=1
		SetMaterialFloat godMaterial,"GodRaysExposure",(1-d)*.5
		SetMaterialFloat godMaterial,"GodRaysLightX",ProjectedX()
		SetMaterialFloat godMaterial,"GodRaysLightY",ProjectedY()
	Else
		SetMaterialFloat godMaterial,"GodRaysExposure",0
		SetMaterialFloat godMaterial,"GodRaysLightX",0
		SetMaterialFloat godMaterial,"GodRaysLightY",0
	EndIf
'End Rem
	
	RenderWorld
	
	Flip
	
Wend
