
Strict

'works for marky!
Global bmx_path$=RequestDir( "BlitzMax path..." )

If FileType( bmx_path+"/mod" )<>FILETYPE_DIR
	Notify "Can't find BMX path"
EndIf

Const TYPE_VOID=0
Const TYPE_INT=1
Const TYPE_FLOAT=2
Const TYPE_OBJECT=3
Const TYPE_CSTRING=4
Const TYPE_VOIDPOINTER=5
Const TYPE_CONSTVOIDPOINTER=6

Global bmx_decls$
Global bmx_inits$

Function Bmx_Func( ty,id$,argtys[],argids$[] )

	Global ty_names$[]=["","%","#","%","$z",":Byte Ptr",":Byte Ptr"]
	
	Local tid$=id
	Select id
	Case "m3dInit"
	Default
		If tid.StartsWith( "m3d" ) tid=tid[3..]
	End Select
	
	Local decl$="Global "+tid+ty_names[ty]+"("
	For Local i=0 Until argids.length
		If i decl:+","
		decl:+argids[i]+ty_names[argtys[i]]
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
	
	Select Parse( t )
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
			Return TYPE_OBJECT
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

Local devPath$=RealPath( CurrentDir()+"/.." )
'Print devPath

Local max3d$=LoadString( "max3d_template.bmx" )
max3d=max3d.Replace( "{INITS}",bmx_inits )
max3d=max3d.Replace( "{DECLS}",bmx_decls )
max3d=max3d.Replace( "{DEVPATH}",devPath )

'Print max3d

CreateDir bmx_path+"/mod/max3d.mod"
CreateDir bmx_path+"/mod/max3d.mod/max3d.mod"
SaveString max3d,bmx_path+"/mod/max3d.mod/max3d.mod/max3d.bmx"

Print "Created module max3d.max3d"
