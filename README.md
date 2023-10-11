**Nume: Alexandru Alin-Ioan**

**Grupă: 312CA**

## Tema 2 - Load Balancer

### Descriere:

* Implementarile functiilor din server.c sunt foarte scurte, este necesara doar apelarea functiilor corespunztoare pentru hashtable-ul serverului.
* Pentru a gasi un server in hash-ring am folosit doua metode:
  * cautarea binara in momentul cand se cauta serverul pentru un obiect.
  * cautarea secventiala, cand se adauga sau se sterge un server.
* Se putea folosi cautarea binara in ambele cazuri, dar nu am mai schimbat implementarea pe care am scris-o initial pentru adaugare si sterger.
* Pentru adaugare / stergere s-au realizat 3 functii. Un care sa gaseasc pozitia serverului ce trebuie adugat / sters, una care sa adauge / sa stearga un server in hash-ring si una care sea redistribuie obiectele.
* Pentru redistribuirea obiectelor la adugare:
  * urmatorul server nu este primul in hash-ring si serverul nou nu este ultimul: hashul obiectului trebuie sa fie mai mic decat hashul serverului nou.
  * serverul nou este ultimul in hash-ring: hashul obiectului trebuie sa fie mai mic decat hashul serverului nou si mai mare decat hashul primului server (serverul urmator).
  * serverul nou este primul in hashring: hashul obiectului trebuie sa fie mai mare decat hashul serverului urmator.
* Pentru redistribuirea obiectelor la stergere toate obiectel din serverul current se adauga in serverul urmator.
* Store si retrieve ambele cauta serverul cu cautarea binara.

### Resurse / Bibliografie:

1. https://ocw.cs.pub.ro/courses/sd-ca/laboratoare/lab-04
