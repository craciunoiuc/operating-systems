
build: so_stdio.obj
	link /nologo /dll /out:so_stdio.dll /implib:so_stdio.lib so_stdio.obj

so_stdio.obj:
	cl.exe /DDLL_EXPORTS /W3 /nologo /c so_stdio.c
	
clean:
	del *.obj *.lib *.dll *.exp