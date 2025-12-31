# TP SecOS/NG – Cartographie mémoire

## Objectif
Cette cartographie mémoire a été conçue pour permettre l’exécution de deux tâches utilisateur
préemptives en ring 3, partageant une page mémoire commune, tout en conservant un noyau simple
(identity mapping) adapté au cadre du TP.

Le choix principal est d’avoir :
- un noyau identique pour toutes les tâches
- un espace virtuel distinct par tâche
- une page physique partagée mappée à des adresses virtuelles différentes

---

## Principes retenus

### 1. Identity mapping du noyau
Le noyau est mappé en **identity mapping** (adresse virtuelle = adresse physique) sur les premiers
mégaoctets de mémoire.

Avantages :
- simplicité d’implémentation
- facilité de débogage
- cohérent avec les TPs précédents

Ce choix est volontairement peu sécurisé mais parfaitement acceptable dans le cadre de l’examen.

---

### 2. Espaces virtuels distincts par tâche
Chaque tâche dispose :
- de son propre **Page Directory (PGD)**
- de sa propre pile utilisateur
- de sa propre pile noyau

Lors d’un changement de tâche :
- le registre **CR3** est mis à jour
- l’espace virtuel actif change entièrement

Cela garantit une isolation logique entre les tâches.

---

### 3. Mémoire partagée
Une **page physique unique** est utilisée comme mémoire partagée (compteur).

Cette page est :
- mappée dans l’espace virtuel de chaque tâche
- accessible en lecture/écriture
- accessible depuis le mode utilisateur (bit US activé)

Les adresses virtuelles sont différentes pour chaque tâche afin de bien illustrer
le mécanisme de traduction d’adresses par pagination.

---

## Organisation mémoire (vue physique)

| Adresse physique | Rôle |
|------------------|------|
| 0x300000         | Code et données noyau |
| 0x400000         | PGD tâche 1 |
| 0x401000         | PTB tâche 1 |
| 0x402000         | Pile noyau tâche 1 |
| 0x403000         | Pile utilisateur tâche 1 |
| 0x500000         | PGD tâche 2 |
| 0x501000         | PTB tâche 2 |
| 0x502000         | Pile noyau tâche 2 |
| 0x503000         | Pile utilisateur tâche 2 |
| 0x700000         | Page mémoire partagée |

---

## Organisation mémoire (vue virtuelle)

### Tâche 1
- Code utilisateur
- Pile utilisateur
- Pile noyau
- Mémoire partagée à une adresse virtuelle dédiée

### Tâche 2
- Même code physique
- Piles distinctes
- Mémoire partagée mappée à une autre adresse virtuelle

---

## Justification des choix
- séparation claire entre noyau et utilisateurs
- mise en évidence du rôle de CR3
- démonstration explicite du partage mémoire
- implémentation simple et robuste pour un TP d’examen

---

## Limites connues
- absence de protection stricte du noyau
- pas de validation poussée des pointeurs utilisateurs
- une seule page partagée

Ces limitations sont assumées et cohérentes avec les objectifs pédagogiques du TP.
