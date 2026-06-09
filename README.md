# chess-game-mds

Un joc de șah implementat în C++ cu SFML, cu suport complet pentru regulile jocului.

## Descriere

chess-game-mds este un joc de șah pentru doi jucători pe același calculator. Jocul respectă regulile complete ale șahului, inclusiv rocada, en passant și promovarea pionului. Interfața grafică este construită cu SFML și afișează piese de șah reale, evidențierea mutărilor legale și detecția șahului.

### Funcționalități

- Tablă de șah 8×8 cu piese grafice
- Mutări valide per piesă (pion, cal, nebun, turn, regină, rege)
- Validare mutări — nu poți lăsa regele în șah
- Rocadă mică și mare
- En passant
- Promovare pion cu UI de selecție
- Evidențiere mutări legale la selectarea unei piese
- Highlight rege în șah
- Turn-based (alb → negru → alb)

## Cerințe

- `g++` cu suport C++17
- SFML 2.5+
- Google Test (pentru teste)

```bash
sudo apt install libsfml-dev libgtest-dev g++
```

## Compilare

```bash
# Jocul
make app

# Testele
make run_tests

# Rulare teste
make test
```

## Rulare

```bash
./app
```

Asigură-te că folderul `sprites/` se află în același director cu executabilul.

## Structura proiectului

```
chess-game-mds/
├── main.cpp
├── Board.h / Board.cpp
├── MoveGen.h / MoveGen.cpp
├── GameLogic.h / GameLogic.cpp
├── Move.h
├── makefile
├── sprites/
│   └── PNGs/
│       └── No shadow/
│           └── 1x/
├── tests/
│   ├── test_board.cpp
│   ├── test_movegen.cpp
│   └── test_gamelogic.cpp
└── docs/
    └── diagrams/
        ├── class_diagram.puml
        ├── sequence_diagram.puml
        └── state_diagram.puml
```

## Diagrame UML

### Class Diagram

![Class Diagram](docs/diagrams/class_diagram.png)

### Sequence Diagram

![Sequence Diagram](docs/diagrams/sequence_diagram.png)

### State Diagram

![State Diagram](docs/diagrams/state_diagram.png)

## CI/CD

Proiectul folosește GitHub Actions pentru CI/CD:

- La fiecare push pe `main` — rulează testele automat
- Dacă testele trec — compilează și publică un release pe GitHub cu executabilul și sprite-urile incluse într-un zip

## Cum se joacă

1. Click pe o piesă proprie pentru a o selecta
2. Pătratele verzi arată mutările legale
3. Click pe un pătrat verde pentru a muta
4. Dacă regele e în șah, pătratul lui devine roșu
5. La promovarea unui pion apare un meniu de selecție