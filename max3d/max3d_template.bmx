
Strict

Rem
bbdoc: Max3D
End Rem
Module Max3D.Max3D

Import BRL.Pixmap
Import BRL.FileSystem
Import BRL.StandardIO
Import BRL.GLGraphics

Const FORMAT_A8=1
Const FORMAT_I8=2
Const FORMAT_L8=3
Const FORMAT_LA8=4
Const FORMAT_RGB8=5
Const FORMAT_RGBA8=6
Const FORMAT_RGB10A2=7
Const FORMAT_RGBA16F=16
Const FORMAT_DEPTH=32

Const TEXTURE_FILTER=1
Const TEXTURE_MIPMAP=2
Const TEXTURE_CLAMPS=4
Const TEXTURE_CLAMPT=8
Const TEXTURE_RENDER=16
Const TEXTURE_STATIC=32
Const TEXTURE_CLAMPST=TEXTURE_CLAMPS|TEXTURE_CLAMPT

{DECLS}

Rem
bbdoc: Max3dGraphics
End Rem
Function Max3dGraphics( w,h,d=0,r=60 )
	GLGraphics w,h,d,r,GRAPHICS_BACKBUFFER
	OpenMax3d
End Function

Rem
bbdoc: LoadTexture
End Rem
Function LoadTexture( path$ )
	Local flags=TEXTURE_FILTER|TEXTURE_MIPMAP|TEXTURE_STATIC
	Print "Loading texture:"+path
	Local t:TPixmap=LoadPixmap( path )
	If Not t t=LoadPixmap( "../samples/"+StripDir(path) )
	If Not t t=LoadPixmap( "../media/"+StripDir(path) )
	If Not t Return 0
	Local fmt=m3dPixelFormat( t )
	Local tex=CreateTexture( t.width,t.height,fmt,flags )
	If Not tex Return 0
	SetTexturePath tex,path
	SetTextureData tex,t.pixels
	Return tex
End Function

Rem
bbdoc: LoadCubeTexture
End Rem
Function LoadCubeTexture( path$ )
	Local flags=TEXTURE_FILTER|TEXTURE_MIPMAP|TEXTURE_STATIC
	Print "Loading cube texture:"+path
	Local t:TPixmap=LoadPixmap( path )
	If Not t t=LoadPixmap( "../samples/"+StripDir(path) )
	If Not t t=LoadPixmap( "../media/"+StripDir(path) )
	If Not t Return 0
	Local size=t.width
	If size=t.height
		Local p:TPixmap=TPixmap.Create( size,size*6,t.format )
		For Local y=0 Until 6
			p.Paste t,0,y*size
		Next
		t=p
	Else If t.height<>size*6
		Return
	EndIf
	Local fmt=m3dPixelFormat( t )
	Local tex=CreateCubeTexture( size,fmt,flags )
	If Not tex Return 0
	SetTexturePath tex,path
	SetCubeTextureData tex,t.pixels
	Return tex
End Function

Rem
bbdoc: LoadTerrain
End Rem
Function LoadTerrain( path$,material,width#,height#,depth#,collType,mass# )
	Local hmap:TPixmap=LoadPixmap( "terrain_256.png" )
	If Not hmap Return
	Local xsize=hmap.width
	Local zsize=hmap.height
	Local terrain=CreateTerrain( material,xsize,zsize,width,height,depth )
	For Local z=0 Until zsize
		For Local x=0 Until xsize
			SetTerrainHeight terrain,(ReadPixel(hmap,x,z) & $ff)/255.0,x,z
		Next
	Next
	If collType Or mass CreateTerrainBody terrain,terrain,collType,mass
	Return terrain
End Function

Rem
bbdoc: LoadMaterial
End Rem
Function LoadMaterial( path$ )
	Local material=CreateMaterial()
	Local diffuse=LoadTexture( path )
	If diffuse SetMaterialTexture material,"DiffuseMap",diffuse
	Return material
End Function

Private

Function m3dLoadTexture( pathz:Byte Ptr )
	Return LoadTexture( String.FromCString( pathz ) )
End Function

Function m3dPixelFormat( t:TPixmap Var )
	Select t.format
	Case PF_ALPHA 
		Return FORMAT_I8
	Case PF_RGB
		Return FORMAT_RGB8
	Case PF_RGBA
		Return FORMAT_RGBA8
	Case PF_BGR
'		t=t.Convert( PF_RGB )
		Return FORMAT_RGB8
	End Select
	Print "Unknown pixel format"
	End
End Function

?Win32
Extern "win32"
Function LoadLibraryA( dll$z )
Function GetProcAddress:Byte Ptr( libhandle,func$z )
End Extern
?Not Win32
Extern
Function dlopen( path$z,mode )
Function dlsym:Byte Ptr( dl,sym$z )
Function dlclose( dl )
End Extern
?

Global m3dLib

Function m3dProc:Byte Ptr( t$ )
	Local p:Byte Ptr
?Win32
	p=GetProcAddress( m3dLib,t )
?Not Win32
	p=dlsym( m3dLib,t )
?
	If Not p Throw "Can't find Max3d symbol:"+t
	Return p
End Function

Function OpenMax3d()
	If m3dLib Return
	
	Local ext$
?Win32
	ext=".dll"
?Macos
	ext=".dylib"
?Linux
	ext=".so"
?
	Local cd$=CurrentDir()
	ChangeDir "{DEVPATH}"
	Local dbg$="Debug/max3d"+ext,rel$="Release/max3d"+ext,ver$
?Debug
	ver=dbg
?Not Debug
	ver=rel
?
	Print "Opening Max3d lib:"+ver
?Win32
	m3dLib=LoadLibraryA( ver$ )
?Not Win32
	m3dLib=dlopen( ver$,1 )
?
	ChangeDir cd
	If Not m3dLib Throw "Can't open Max3d lib:"+ver
	
{INITS}

	ChangeDir "{DEVPATH}/max3d/max3d"
	m3dInit "SHADOWS=TRUE"
	ChangeDir cd

	SetTextureLoader m3dLoadTexture
End Function
