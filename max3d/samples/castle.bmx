		
Strict

Import Bmx3d.Max3d

Max3dGraphics 1024,768

?MacOS
DisableShadows
?

SetClearColor .25,.5,1

SetAmbientColor .25,.25,.25

'Set up fog
Local fogShader=CreateShader( LoadString( "linearfog.glsl" ) )
Local fogMaterial=CreateMaterial()
SetMaterialFloat fogMaterial,"FogStart",25
SetMaterialFloat fogMaterial,"FogEnd",50
SetMaterialColor fogMaterial,"FogColor",.25,.5,1	'same as clear color...
AddRenderPass fogShader,fogMaterial

Local godShader=CreateShader( LoadString( "godrays.glsl" ) )
Local godMaterial=CreateMaterial()
SetMaterialColor godMaterial,"Color",.2,.2,0
?Not MacOS
AddRenderPass godShader,godMaterial	'A bit much for poor Mac...
?

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
Local lightYaw#=45,lightPitch#=45,lightRoll#
TurnEntity light,lightYaw,lightPitch,lightRoll

Local splits#[]=[.1,4.0,16.0,64.0,256.0]
SetLightShadowSplitsTable light,5,splits

Local castle=LoadModel( "CASTLE1.X",4,0 )
MoveEntity castle,0,.5,0

Local bluspark=CreateMaterial()
SetMaterialColor bluspark,"SpriteColor",1,1,1
SetMaterialTexture bluspark,"SpriteTexture",LoadTexture( "bluspark.bmp" )
Local sprite=CreateSprite( bluspark )

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
	
	Local sx#=EntityMatrixElement( camera,3,0 )+10
	Local sy#=EntityMatrixElement( camera,3,1 )+10
	Local sz#=EntityMatrixElement( camera,3,2 )-10
	SetEntityTranslation sprite,sx,sy,sz
	SetEntityRotation sprite,lightYaw,lightPitch,lightRoll
	MoveEntity sprite,0,0,-20
	
	If CameraProject( camera,sx,sy,sz )
		Local x#=ProjectedX(),y#=ProjectedY()
		Local dx#=x-512,dy#=y-384
		Local d#=Sqr( dx*dx+dy*dy )/384.0
		If d>1 d=1
		SetMaterialFloat godMaterial,"Exposure",(1-d)*.0034
		SetMaterialFloat godMaterial,"LightPosX",ProjectedX()
		SetMaterialFloat godMaterial,"LightPosY",ProjectedY()
	Else
		SetMaterialFloat godMaterial,"Exposure",0
		SetMaterialFloat godMaterial,"LightPosX",0
		SetMaterialFloat godMaterial,"LightPosX",0
	EndIf
	
	RenderWorld
	Flip
	
'	CameraProject camera,EntityX( mirror ),EntityY( mirror ),EntityZ( mirror )
'	Print ProjectedX()+","+ProjectedY()+","+ProjectedZ()
	
Wend
