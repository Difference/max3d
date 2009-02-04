
Strict

Rem
bbdoc: Max3D
End Rem
Module Bmx3D.Max3D

Import BRL.Pixmap
Import BRL.FileSystem
Import BRL.StandardIO
Import BRL.GLGraphics

{INCBINS}

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

Global Max3dImportDirs$[]=["{DEVDIR}/max3d/max3d"]

Rem
bbdoc: Max3dGraphics
End Rem
Function Max3dGraphics( w,h,d=0,r=60,flags=0 )
	GLGraphics w,h,d,r,GRAPHICS_BACKBUFFER
	OpenMax3d flags
End Function

Rem
bbdoc: LoadShader
End Rem
Function LoadShader( path$ )
	Local source$=LoadString( path )
	Local shader=CreateShader( source )
	Return shader
End Function

Rem
bbdoc: LoadTexture
End Rem
Function LoadTexture( path$ )
	Print "Loading texture:"+path
	Local flags=TEXTURE_FILTER|TEXTURE_MIPMAP|TEXTURE_STATIC'TEXTURE_FILTER|TEXTURE_MIPMAP|TEXTURE_STATIC
	Local t:TPixmap=LoadPixmap( path )
	If Not t Return
	Local fmt=m3dPixelFormat( t )
	Local tex=CreateTexture( t.width,t.height,fmt,flags )
	If Not tex Return 0
	SetObjectImportPath tex,path
	SetTextureData tex,t.pixels
	Return tex
End Function

Rem
bbdoc: LoadCubeTexture
End Rem
Function LoadCubeTexture( path$ )
	Local flags=TEXTURE_FILTER|TEXTURE_MIPMAP|TEXTURE_STATIC
	Local t:TPixmap=LoadPixmap( path )
	If Not t Return
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
	SetObjectImportPath tex,path
	SetCubeTextureData tex,t.pixels
	Return tex
End Function

Rem
bbdoc: LoadMaterial
End Rem
Function LoadMaterial( path$ )

	Local file$=StripExt( path )
	Local extn$=ExtractExt( path )
	
	Local mat=CreateMaterial()
	SetObjectImportPath mat,path
	
	Local exts$[]=[..
	"DiffuseTexture,,_d",..
	"SpecularTexture,Spec,_s",..
	"NormalTexture,Normal,_n,_local"]
	
	If file.EndsWith( "_d" ) file=file[..file.length-2]
	
	For Local i=0 Until exts.length
		Local bits$[]=exts[i].Split(",")
		For Local j=1 Until bits.length
			Local p$=file+bits[j]+"."+extn
			If FileType( p )<>FILETYPE_FILE Continue
			Local tex=LoadTexture( p )
			If Not tex Continue
			SetMaterialTexture mat,bits[0],tex
			Exit
		Next
	Next
	
	Return mat
End Function

Rem
bbdoc: LoadTerrain
End Rem
Function LoadTerrain( path$,material,width#,height#,depth#,collType,mass# )
	Local hmap:TPixmap=LoadPixmap( path )
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

Private

Function m3dImporter( classz:Byte Ptr,pathz:Byte Ptr )
	Local class$=String.FromCString( classz )
	Local path$=String.FromCString( pathz )
	
	If FileType( path )=FILETYPE_NONE
		If FileType( path.ToLower() )<>FILETYPE_NONE
			path=path.ToLower()
		Else
			Local file$=StripDir( path ),tpath$
			For Local dir$=EachIn Max3dImportDirs
				tpath=dir+"/"+file
				If FileType( tpath )=FILETYPE_FILE Exit
				tpath=""
			Next
			If Not tpath
				Print "Max3d Error: Unable to locate object of type '"+class+"' at:"+path
				Return
			EndIf
			path=tpath
		EndIf
	EndIf
	
	Select class
	Case "CShader"
		Return LoadShader( path )
	Case "CTexture"
		Return LoadTexture( path )
	Case "CMaterial"
		Return LoadMaterial( path )
	End Select

	Print "Max3d Error: Don't know how to import object of type '"+class+"'"
	End
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
	Print "Max3d Error: Unknown pixel format"
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
	If p Return p
	Print "Max3d Error: Can't find Max3d symbol:"+t
	End
End Function

Function OpenMax3d( flags )
	If m3dLib Return
	
	Local lib$="{DEVDIR}/"
?Debug
	lib:+"Debug/"
?Not Debug
	lib:+"Release/"
?Win32
	lib:+"max3d.dll"
?Macos
	lib:+"max3d.dylib"
?Linux
	lib:+"max3d.so"
?Win32
	m3dLib=LoadLibraryA( lib )
?Not Win32
	m3dLib=dlopen( lib,1 )
?
	If Not m3dLib
		Print "Max3d Error: Can't open Max3d lib:"+lib
		End
	EndIf
	
	{INITS}
	
	InitMax3d m3dImporter,flags
End Function
