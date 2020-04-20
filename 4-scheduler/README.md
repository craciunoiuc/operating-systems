Nume: Craciunoiu Cezar
Grupa: 334CA

# Tema 4 - Scheduler

Organizare
-
* Spre deosebire de celelalte teme, aceasta a avut un schelet foarte permisiv,
* doar header-ul fiind oferit si checker-ul. Pentru implementarea priority
* queue-ului s-a folosit o sursa de pe github (mentionata in Bibliografie),
* care ofera suport pentru tipuri de date generice.

***Obligatoriu:*** 
* Structura generala a programului este simplificata, deoarece s-a dorit
* obtinerea a doar jumatate din punctaj. Doar functiile de baza (fork/end/init)
* sunt implementate ca atare, restul verificand doar cazurile de input invalid.
* Implementarea din coada cu prioritati este de baza, fiind oferite functii
* pentru adaugarea si eliminearea de elemente.
* Consider ca tema cere, de fapt, implementarea unui automat cu stari, si nu
* mai mult de atat. Ideea de planificator de thread-uri este folosita doar
* pentru a testa cunostiintele de la laborator.
* Implementarea este simpla. Pentru fiecare fork cerut, se creeaza un thread
* nou cu datele sale si este adaugat in coada. Thread-urile ruleaza unul cate
* unul, deoarece modul de rulare presupune un singur core.

Implementare
-

* S-a incercat sa se maximizeze punctajul pentru o implementare simplificata.
* Thread-urile se executa cu succes, unul dupa altul, dar nu in ordinea din
* coada folosita.
* Functionalitate lipsa: preemtarea si ordonarea thread-urilor.
* Interesant: prima tema de la toate materiile din facultate care are checker-ul
* scris in C++.

Cum se compilează și cum se rulează?
-
* Build-ul este acoperit de makefile (un simplu make este de ajuns).
* Acesta creeaza o biblioteca partajata care este folosita de checker pentru
* a incarca si rula diferite surse.
* Pentru a se verifica trebuie ca biblioteca partajata sa se afle in folder-ul
* numit "checker-lin" ce a fost oferit (se poate folosi 'make move');
Bibliografie
-
* https://github.com/nomemory/c-generic-pqueue
* https://linux.die.net/
* Laboratoarul 8 (de pe ocw)

Git
-
1. https://gitlab.cs.pub.ro/cezar.craciunoiu/l3-so-assignments/tree/master/4-scheduler
2. https://gitlab.cs.pub.ro/cezar.craciunoiu/l3-so-assignments.git

Mentiuni
* Au fost folosite bullet-uri la fiecare linie pentru a se garanta pe gitlab
* ca se pastreaza liniile de 80 de caractere din README intr-un mod
* relativ elegant.
