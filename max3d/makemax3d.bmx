
Strict

Global bmx_path$=RequestDir( "BlitzMax path...","C:\Program Files\BlitzMax" )

If FileType( bmx_path+"/mod" )<>FILETYPE_DIR
	Notify "Can't find BMX path"
EndIf

Local mod_time=FileTime( bmx_path+"/mod/bmx3d.mod/max3d.mod/max3d.bmx" )
If mod_time
	If mod_time>FileTime( "max3d/api.cpp" )
		If mod_time>FileTime( "makemax3d.bmx" )
			If mod_time>FileTime( "max3d_template.bmx" )
				If Not Confirm( "Max3d mod appears to be up to date - make anyway?" )
					End
				EndIf
			EndIf
		EndIf
	EndIf
EndIf

Global bmx_decls$
Global bmx_inits$

Global bmx_oodecls$
Global bmx_ooinits$

Const TYPE_VOID=0
Const TYPE_INT=1
Const TYPE_FLOAT=2
Const TYPE_CSTRING=3
Const TYPE_VOIDPOINTER=4
Const TYPE_CONSTVOIDPOINTER=5
Const TYPE_OBJECT=6

Global ObjectTypes$[]=[..
"Object",..
" Resource",..
"  Shader",..
"  Texture",..
"  Material",..
"  Surface",..
"   ModelSurface",..
"   SpriteSurface",..
"   MirrorSurface",..
"   TerrainSurface",..
" Entity",..
"  Camera",..
"  Light",..
"  Model",..
"  Sprite",..
"  Mirror",..
"  Terrain"]

Function Bmx_TypeName$( ty )
	Global ty_names$[]=["","%","#","$z",":Byte Ptr",":Byte Ptr"]
	If ty<TYPE_OBJECT Return ty_names[ty]
	Return "%"
	Return ":T"+ObjectTypes[ty-TYPE_OBJECT].Trim()
End Function

Function Bmx_Func( ty,id$,argtys[],argids$[] )
	Local tid$=id

	If tid.StartsWith( "m3d" ) tid=tid[3..]
	
	Local decl$="Global "+tid+Bmx_TypeName(ty)+"("
	For Local i=0 Until argids.length
		If i decl:+","
		decl:+argids[i]+Bmx_TypeName(argtys[i])
	Next
	decl:+")~n"
	decl="Rem~nbbdoc: "+tid+"~nEnd Rem~n"+decl
	
	Local init$="~t"+tid+"=m3dProc(~q"+id+"~q)~n"
	
	bmx_decls:+decl
	bmx_inits:+init

End Function

Function Parse$( t$ Var )
	Local i
	While i<t.length And t[i]<=32
		i:+1
	Wend
	Local i0=i
	While i<t.length
		Local c=t[i]
		i:+1
		If c=Asc("_") Continue
		If c>=Asc("0") And c<=Asc("9") Continue
		If c>=Asc("A") And c<=Asc("Z") Continue
		If c>=Asc("a") And c<=Asc("z") Continue
		If i>i0+1 i:-1
		Exit
	Wend
	Local q$=t[i0..i]
	t=t[i..]
	Return q
End Function

Function ParseType( t$ Var )
	Local tl$=t
	Local ty$="bad!"
	
	Local p$=Parse( t )
	Select p
	Case "void"
		Local q$=t
		Select Parse( t )
		Case "*"
			Return TYPE_VOIDPOINTER
		End Select
		t=q
		Return TYPE_VOID
	Case "int"
		Return TYPE_INT
	Case "float"
		Return TYPE_FLOAT
	Case "const"
		Select Parse( t )
		Case "char"
			Select Parse( t )
			Case "*"
				Return TYPE_CSTRING
			End Select
		Case "void"
			Select Parse( t )
			Case "*"
				Return TYPE_CONSTVOIDPOINTER
			End Select
		End Select
	Default
		Select Parse( t )
		Case "*"
			For Local i=0 Until ObjectTypes.length
				If p="C"+ObjectTypes[i].Trim() Return TYPE_OBJECT+i
			Next
		End Select
	End Select
	Print "ERROR:"+tl
	End

End Function

Local api$=LoadString( "max3d/api.cpp" )

For Local tline$=EachIn api.Split( "~n" )
	If Not tline.StartsWith( "API " ) Continue
	
	Local line$=tline[4..]
	
	Local ty=ParseType( line )
	Local id$=Parse( line )
	
	Local argids$[],argtys[]
	
	If Parse( line )="("
		Local q$=line
		If Parse( line )<>")"
			line=q
			Repeat
				Local ty=ParseType( line )
				Local id$=Parse( line )
				argids:+[id]
				argtys:+[ty]
				Select Parse( line )
				Case ")"
					Exit
				Case ","
				Default
					Print "ERROR:"+tline
				End Select
			Forever
		EndIf
	Else
		Print "ERROR:"+tline
	EndIf
	
	Bmx_Func( ty,id,argtys,argids )	

Next

CreateDir bmx_path+"/mod/bmx3d.mod"
CreateDir bmx_path+"/mod/bmx3d.mod/max3d.mod"

Local incbins$
Rem
For Local f$=EachIn LoadDir( "max3d" )
	If f.EndsWith( ".glsl" ) And FileType( "max3d/"+f )=FILETYPE_FILE
		CopyFile "max3d/"+f,bmx_path+"/mod/bmx3d.mod/max3d.mod/"+f
		incbins:+"Incbin ~q"+f+"~q~n"
	EndIf
Next
End Rem

Local devDir$=RealPath( CurrentDir()+"/.." )

Local max3d$=LoadString( "max3d_template.bmx" )

max3d=max3d.Replace( "{INCBINS}",incbins )
max3d=max3d.Replace( "{INITS}",bmx_inits )
max3d=max3d.Replace( "{DECLS}",bmx_decls )
max3d=max3d.Replace( "{DEVDIR}",devDir )

'Print max3d

Extern 
Function system( cmd$z )
End Extern

SaveString max3d,bmx_path+"/mod/bmx3d.mod/max3d.mod/max3d.bmx"

system "~q"+bmx_path+"/Bin/bmk~q makemods -a bmx3d"

Print "Created module bmx3d"
