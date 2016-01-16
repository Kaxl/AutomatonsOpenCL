Game Of Life
------------

Mise à jour de l'affichage très lent.
Probablement à cause du réseau.
L'affichage est mis à jour toute les milisecondes + le temps de calcul de l'automate
+ le temps de modification dans le tableau d'affichage
Donc le réseau n'arrive probablement pas à suivre (latence)

Forest Fire
-----------

On ne peut pas generer des nombres aleatoires dans le kernel.
J'ai donc du creer un tableau de N * N valeurs aleatoires que je passe
au kernel.
Ce n'est pas forcement la meilleure solution, mais bon, ca marche
