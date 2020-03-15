Nume: Craciunoiu Cezar
Grupă: 334CA

# Tema 2 - Stdio

Organizare
-
* Enuntul temei a fost destul de strict, trebuind doar sa se implementeze
* functiile din header-ul dat. Totusi, a ramas la propria implementare formatul
* structurii. S-a urmat modelul oficial de implementare a structurii FILE si
* s-au implementat pe rand functionalitatile din tema.

***Obligatoriu:*** 
* Mai intai s-a creat o structura de baza FILE, la care s-au adaugat, pe rand,
* campuri, in functie de nevoie. Pentru functiile de citire: s-a citit in
* buffer daca a fost nevoie, altfel s-a returnat informatia mutandu-se
* pointerul indicator in functie de cat s-a dorit citit. In partea de scriere
* s-a procedat asemanator, dar in sens invers. Datele erau scrise in buffer
* inainte de iesire (nu venire).
* Consider ca tema nu a ajutat studentii la a invata nimic nou, doar la a
* repeta exercitiile de la laborator. Acesta nu este un lucru rau neaparat.
* Implementarea este destul de completa pe toate testele ce sunt trecute,
* nelasand loc pentru interpretare.

***Opțional:***
* In enunt nu se specifica si nu se verifica cazurile de "eroare umana" in care
* inputul de la tastatura este eronat. Acest lucru a fost implementat in tema.


Implementare
-

* Pe partea de Linux, s-a implementat totul, dar, din pacate, functia so_popen
* nu functioneaza corespunzator. Pe partea de Windows s-au implementat
* functiile pentru testele de citire (so_fgetc/so_fread/so_fopen).
* Functionalitate lipsa: so_popen -> testele reflecta acest lucru
* S-au intampinat dificultati in a se gasi sfarsitul unui fisier de pipe.
* Mi s-a parut interesanta metoda prin care se verifica cate apeluri
* de read/write au fost facute la sistem.

Cum se compilează și cum se rulează?
-
* Build-ul este acoperit de makefile. Acesta creeaza o biblioteca partajata
* care este folosita de checker pentru a rula diferite surse.
* Astfel ca, nu se creeaza un executabil propriu-zis ce poate fi rulat.

Bibliografie
-

* https://stackoverflow.com/questions/5672746/what-exactly-is-the-file-keyword-in-c
* Laboratoarele 1-2-3 (de pe ocw)

Git
-
1. https://gitlab.cs.pub.ro/cezar.craciunoiu/l3-so-assignments/tree/master/2-stdio
2. https://gitlab.cs.pub.ro/cezar.craciunoiu/l3-so-assignments.git

