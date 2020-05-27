CC = cl.exe
LINK = link.exe
CFLAGS = /nologo /W3 /MD /D_CRT_SECURE_NO_DEPRECATE

all: so-cpp.exe

build: so-cpp.exe

so-cpp.exe: hashtable.obj main.obj
	$(LINK) /OUT:so-cpp.exe hashtable.obj main.obj

main.obj: hashtable.h main.c
	$(CC) $(CFLAGS) /c main.c /Fomain.obj

hashtable.obj: hashtable.c
	$(CC) $(CFLAGS) /c hashtable.c /Fohashtable.obj

clean: exe_clean obj_clean

obj_clean:
	del *.obj

exe_clean:
	del so-cpp.exe

