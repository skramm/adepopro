
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

### Configuration

Le programme est adaptable via un fichier texte de type ".ini": adepopro.ini
Un exemple d'un tel fichier est fourni.

On peut y spécifier:
* les indices des colonnes dans le fichier d'entrée,
* les positions des clés à utiliser pour le regroupement dans le rapport par module d'enseignement
* l'intitulé de regroupement de 1er et 2ème niveau ("semestre", "Unité d'enseignement", "formation", ...), voire ci-dessous.

### Regroupement de modules ###

Très souvent, le code module d'un enseignement "encode" de façon alphanumérique des informations comme par exemple le semestre, l'unité d'enseignement ou la formation concernée.
Dans le fichier-rapport "ME", il est possible de mettre en oeuvre un regroupement avec calcul de sous total sur la base de ce code, et ce à deux niveaux.
Attention cependant, ce tri n'est possible que via l'utilisation d'un seul caractère du code-module.

Par exemple, si le code module est de la forme ```ABC143``` et que le 1er chiffre encode le semestre et le deuxième encode l'unité d'enseignement,
alors on pourra utiliser ces caractères pour grouper les modules.
Ceci se paramètre dans le fichier de configuration, via les clés de la section ```[grouping]```:
Pour activer le regroupement de 1er niveau, il faut mettre la clé ```groupKey1``` à 1.
De façon similaire (et uniquement si la clé précédente est activée), on pourra activer le  regroupement de 2ème niveau avec ```groupKey2=1```.

Les caractères à considérer dans le code module pour effectuer ce regroupement sont donnés par leur position dans le code-module,
et les clés dans le fichier de configuration sont:
```groupKey1_pos``` et ```groupKey2_pos``` (indice commencant à 0 pour le 1er caractère de la chaîne).

L'intitulé de ce qu'on veut regrouper est à indiquer dans les clés
```groupKey1_name``` et ```groupKey1_name```.

Par exemple, si le code module est de la forme ABC1XYZ pour un module du 1er semestre et ABC2QSD pour un module du second semestre
et qu'on souhaite activer un regroupement par semestre, alors il faudra spécifier:
```
groupKey1=1
groupKey1_name=Semestre
groupKey1_pos=3
```

Le fichier de sortie "ME" sera alors découpé en autant de sections qu'il y a de valeurs pour le 4ème caractère du code-module, et présentera un sous-total par semestre:
```
*** Semestre: 1 ***
...
...
...
* Total Semestre 1: CM: 124.5 h. - TD: ...
```

Il est aussi possible d'indiquer un intitulé pour chaque regroupement.
Par exemple, supposons que les codes modules soient de la forme ```ABC1XXX```, ```ABC2XXX```,
et que le chiffre en 4ème position encode la formation, appelée TATATA (pour "1") ou TITITI (pour "2").
Alors il sera possible d'avoir un regroupement tel que cela apparaisse comme ceci dans le fichier de sortie:

```
*** Formation: TATATA ***
...
...
...
* Total Formation: TATATA: CM: 123 h. - TD: ...
```
et
```
*** Formation: TITITI ***
...
...
...
* Total Formation: TITITI: CM: 44 h. - TD:  ...
```

Ceci sera paramétré dans le fichier de configuration par les clés ```groupKey1_pairs```
(ou ```groupKey2_pairs``` pour le regroupement de 2ème niveau).
Il faudra y spécifier la liste des associations à faire entre le caractère du code module et la chaine de caractère correspondante à associer.

Pour l'exemple ci-dessus, on mettra:
```
groupKey1_pairs=1-TATATA;2-TITITI
```

### Options

Le programme supporte les deux options suivantes:
* "-s" : le rapport texte par module d'enseignement sera regroupé par sections, voir "Configuration".
* "-p" : affiche les paramètres de fonctionnement.


S. Kramm - 2018
