
## ADEPoPro: Manuel français

Voir https://github.com/skramm/adepopro

(version du 2018-06-23)

### Introduction

Ce programme est dédié à la génération de rapports de synthèse à partir d'un export de données depuis le client lourd Windows de ADE Campus.
En effet, il semble que ni le client lourd, ni le client Web ne permettent d'obtenir des informations synthétiques telles que:

* quels sont les modules d'enseignement sur lesquels un enseignant donné intervient ?
* quels sont les enseignants qui interviennent sur un module donné ?
* sur combien de jours, de semaines s'étale un module ?
* combien de jours, de semaines d'intervention pour un enseignant ?

À partir d'un unique fichier d'entrée, généré à partir de ADE Campus, ce programme génère quatre fichiers de sortie regroupant toutes ces informations (voir ci-dessous).

### Procédure

Le fichier d'entrée au format csv est à générer depuis le client lourd Windows de ADE Campus.
La manip ci-dessous a été testée avec la version 6.5.3

1. Ouvrir ADE Campus et sélectionner "Activités" dans la barre d'outils:
![im1](ade_1b.png)
1. Dans le volet de gauche, sélectionner les formations désirées, puis cliquer sur la première ligne de la liste.<br>
Vérifier que les champs apparaissent bien dans cet ordre.
Au besoin, ajouter les champs nécessaires en cliquant sur le bouton entouré en rouge ci-dessous.
![im2](ade_2b.png)
1. Sélectionner l'ensemble des évènements avec CTRL-A:
![im3](ade_3b.png)
1. Copier (CTRL-C), puis ouvrir un tableur (Excel ou LibreOffice-Calc), sélectionner la cellule en haut à gauche, et coller (CTRL-V).
1. Sauvegarder au format CSV, en vérifiant les options: séparateur de champ: ";", et pas de guillements autour des chaînes de caractères.

Vous avez maintenant un fichier CSV, qui pourra être utilisée comme entrée de AdePoPro, comme ceci:
```
adepopro monfichier.csv
```

### Fichiers de sortie

En cas de succès, ceci va générer 4 fichiers dans le dossier courant:
* 2 fichier csv de 9 champs, qui peuvent être ouverts dans n'importe quel tableur, permettant un reporting ou d'autres traitements:
  * ```adepopro_E_monfichier.csv``` :<br>
 Contient la liste des enseignants avec pour chacun d'eux, le nombre de jours et de semaines d'activité, le volume total d'enseignement en CM,TD, TP, ainsi que le total présentiel (4 valeurs: CM, TD, TP, plus le total), et le nombre de modules où l'enseignant intervient.
  * ```adepopro_M_monfichier.csv```<br>
 Contient la liste des modules avec pour chacun d'eux, le nombre de jours et de semaines d'activité, le volume CM,TD, TP, ainsi que le total et le nombre d'enseignants intervenant sur ce module.


* 2 fichiers "texte", contenant un rapport synthétique:
  * ```adepopro_EM_monfichier.txt```
  * ```adepopro_ME_monfichier.txt```

Ces fichiers donnent le détail des infos.
* Le fichier "EM" (*Enseignant/Module*) donne pour chaque enseignant le détails des modules dans lequel il intervient, avec le volume correspondant, exprimé en CM/TD/TP.
* Le fichier "ME" (*Module/Enseignant*) donne pour chaque module la liste des enseignants qui interviennent, avec le volume correspondant.

Les volumes sont exprimés en heures. Dans les fichiers .csv, ils apparaissent sous la forme:<br>
```vol_CM;vol_TD;vol_TP;total```

Dans les fichiers texte (rapports), les volumes sont également donnés en "heures équivalent TD".
Attention cependant, ceci utilise le calcul "classique"
(CM: coeff. 3/2, TD: coeff. 1, TP: coeff. 2/3),
et l'application ou non de ce calcul pour la rémunération depend du statut de l'enseignant.

Dans le fichier-rapport "ME", il est possible de mettre en oeuvre un regroupement avec calcul de sous total, à deux niveaux.
Ceci est possible uniquement si le code-module encode les informations telle que le semestre, l'unité d'enseignement ou la formation concernée.
Attention cependant, ce tri n'est possible que via l'utilisation d'un seul caractère du code-module.

Par exemple, si le code module est de la forme **ABC143** et que le 1er chiffre encode le semestre et le deuxième encode l'unité d'enseignement, alors on pourra utiliser ces caractères pour grouper les modules.

### Options

Le programme supporte les deux options suivantes:
* "-s" : le rapport texte par module d'enseignement sera regroupé par sections, voir "Configuration".
* "-p" : affiche les paramètres de fonctionnement.

### Configuration

Le programme est adaptable via un fichier texte de type ".ini": adepopro.ini
Un exemple d'un tel fichier est fourni.

On peut y spécifier:
* les indices des colonnes dans le fichier d'entrée,
* les positions des clés à utiliser pour le regroupement dans le rapport par module d'enseignement
* l'intitulé de regroupement de 1er et 2ème niveau ("semestre", "Unité d'enseignement", "formation", ...)


S. Kramm - 2018
