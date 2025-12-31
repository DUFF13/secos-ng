# TP SecOS/NG – Description des fichiers

Ce document décrit le rôle de chaque fichier utilisé ou ajouté dans le cadre du TP SecOS/NG.

---

## Fichier principal

### `tp.c`
Fichier central du TP.

Contient :
- la fonction d’entrée `tp()`
- l’initialisation de la GDT et du TSS
- la mise en place de la pagination
- l’initialisation des tâches
- le démarrage de l’ordonnanceur
- les handlers principaux (IRQ0, syscall)

C’est ce fichier qui orchestre l’ensemble du comportement du noyau pour l’examen.

---

## Gestion des tâches

### `task.h`
Définit :
- la structure représentant une tâche
- les champs nécessaires au changement de contexte
- les prototypes liés à l’ordonnancement

Chaque tâche possède :
- un identifiant
- une pile noyau
- une pile utilisateur
- un pointeur vers son contexte sauvegardé

---

## Gestion mémoire

### `config.h`
Centralise **toutes les constantes mémoire** :
- adresses physiques
- adresses virtuelles
- tailles des piles
- adresses de la mémoire partagée

Ce fichier permet de modifier la cartographie mémoire sans toucher au reste du code.

---

### `memory.h`
Contient des fonctions utilitaires pour la pagination :
- calcul des indices de PGD / PTB
- création des entrées de page
- abstraction de la manipulation des structures de pagination

Il évite la duplication de code bas niveau.

---

## Interruptions et exceptions

### `handlers.h`
Déclare les prototypes des handlers :
- interruption timer (IRQ0)
- appel système (int 0x80)
- fautes (page fault, general protection fault)

Les implémentations correspondantes sont dans `tp.c`.

---

## Documentation

### `MEMORY_MAP.md`
Explique :
- la cartographie mémoire choisie
- les raisons techniques de ces choix
- la distinction entre mémoire physique et virtuelle
- le fonctionnement de la mémoire partagée

---

### `FILES.md`
Document actuel.
Présente le rôle de chaque fichier pour faciliter la compréhension globale du projet.

---

## Fichiers existants non modifiés

Les fichiers du noyau fournis (`kernel/`, `utils/`, scripts de build) n’ont pas été modifiés,
sauf mention explicite ci-dessous.

---

## Modification liée à un problème de compilation

Lors de la compilation, un problème est apparu lié à une **incompatibilité avec la version du compilateur**.

Une **modification mineure du noyau** a été effectuée afin de :
- restaurer la compatibilité avec la version utilisée
- permettre la compilation sans avertissements bloquants

Cette modification n’impacte pas la logique du TP et ne change pas le comportement fonctionnel
du noyau, mais uniquement sa compatibilité avec l’environnement de compilation.

---

## Conclusion
L’organisation des fichiers vise à :
- séparer clairement les responsabilités
- rendre le TP lisible et maintenable
- faciliter la compréhension pour la correction
