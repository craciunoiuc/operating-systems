Nume: Craciunoiu Cezar
Grupa: 334CA

# Tema 3 - Loader

Organizare
-
* Scheletul temei a fost destul de strict, trebuind doar sa se implementeze
* in fisierul loader.c dat. Totusi, a ramas la propria implementare continutul
* functiei de gestionare al segmentation fault-ului.
* S-a utilizat fisierul de parse-are al executabilului de tip ELF si
* s-a construit un mod de citire a informatiei per segment si scriere
* in fiecare pagina necesara.

***Obligatoriu:*** 
* Structura generala a programului este destul de simpla. Mai intai se
* realizeaza parse-area, folosind functia oferita. Functia de seteaza
* intreruperi marcheaza intreruperea SIGSEGV ca cea care se va gestiona.
* Restul implementarii se afla in functia de gestionare al acestui semnal, in
* care se foloseste si o functie aditionala de citire din fisier.
* Consider ca tema, desi a avut un enunt destul de vag la prima vedere, este
* una care face legatura intr-un mod elegant intre curs si laborator. Cu alte
* cuvinte, ajuta studentii sa inteleaga cum sa gestioneze memoria, dar si
* sa inteleaga formatul ELF.
* Implementarea este destul de simpla. Pentru fiecare acces la memorie verifica
* daca accesta este ilegal sau trebuie sa se mapeze acea zona de memorie.
* Din pacate, implementarea duce la esuarea ultimului test, deoarece se ajunge,
* intr-un mod nedeterminat, cu mult, in afara zonei programului.

Implementare
-

* S-a incercat o implementare completa, dar, din pacate, la un numar foarte
* mare de page-fault-uri, se ajunge la un comportament nedeterminat, care duce
* programul intr-un Segmentation Fault normal.
* Functionalitate lipsa: loading cu multe page fault-uri -> testele reflecta
* Mi s-a parut interesant faptul ca nu exista magie "in spate" (si programul
* care porneste programe e un program ca toate programele)

Cum se compilează și cum se rulează?
-
* Build-ul este acoperit de makefile (care a fost si oferit de echipa).
* Acesta creeaza o biblioteca partajata care este folosita de checker pentru
* a incarca si rula diferite surse.
* Pentru a se verifica trebuie ca biblioteca partajata sa se afle in folder-ul
* numit "checker-lin" ce a fost oferit.
Bibliografie
-

* https://linux.die.net/
* Laboratoarele 4-5-6 (de pe ocw)

Git
-
1. https://gitlab.cs.pub.ro/cezar.craciunoiu/l3-so-assignments/tree/master/3-loader
2. https://gitlab.cs.pub.ro/cezar.craciunoiu/l3-so-assignments.git

Mentiuni
* Au fost folosite bullet-uri la fiecare linie pentru a se garanta pe gitlab
* ca se pastreaza liniile de 80 de caractere din README intr-un mod
* relativ elegant.
