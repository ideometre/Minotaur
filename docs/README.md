# "Revivez le mythe sur Arduboy"

## Reprise du dev sur VS Code

Guide d'installation et workflow Linux/Arduboy: [DEV_SETUP.md](DEV_SETUP.md)

## Les élements du jeu

### Implémenté

* Un Minotaure <img src="./img/min_front.png"> <img src="./img/min_right.png"> <img src="./img/min_back.png"> <img src="./img/min_left.png">
* Un Thésée (armé ou non) <img src="./img/the_front.png"> <img src="./img/the_right.png"> <img src="./img/the_back.png"> <img src="./img/the_left.png">
* Une épée (ramassable) <img src="./img/sword.png">
* Une pelote (ramassable) <img src="./img/string.png">
* Une clé (ramassable) <img src="./img/key.png">
* Des points de départ <img src="./img/input.png">
* Une porte de sortie <img src="./img/output.png">
* Menu principal (Play / Help / Credits)
* Écran d’aide avec les contrôles
* Écran crédits (galerie de sprites)
* 3 niveaux avec collisions et positions de spawn distinctes
* Transformation Thésée ↔ Minotaure (touche B)
* Équipement de l’épée au ramassage (touche A pour dégainer)
* Retour au menu depuis n’importe quel écran (A+B)
* Choix d’un thème de terrain <img src="./img/wall_cross.png"> <img src="./img/wall_straight.png"> <img src="./img/wall_t.png"> <img src="./img/wall_dot.png"> <img src="./img/wall_angle.png"> <img src="./img/wall_end.png"> <img src="./img/wall_empty.png">

### Roadmap

* Génération aléatoire du labyrinthe (miroir + puzzle)
* Déplacement aléatoire du Minotaure
* Résumé du mythe au démarrage (peut être passé)
* Choix de la taille du labyrinthe (petit 2×2 / normal 3×3 / grand 4×4)
* Son
* LED radar (bleu épée, vert ariane, rouge clef)
* Image de fin (Thésée tuant le Minotaure)
* Téléportation aux bords de la carte
* Enregistrement des records de temps par catégorie


<img src="./img/arduboyplay.png">


<a href="https://maxime.hanicotte.net"><img src="./img/mx-logo.png" width="36" alt="MX" align="right"></a>

--
