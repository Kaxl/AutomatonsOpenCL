// Kernel majorité

__kernel void majority(const int N, __global int *grid, __global int *newGrid)
{
    // id global
    int id = get_global_id(0);

    int nbVoteUn;
    // Test si bien dand la grille
    if ((id / N) < N && (id % N) < N) {
    
        // pour savoir de quelle couleurs est la majorité, additionne
        // toutes les cellules voisinnes pour avoir le nombre en position 1
        // et après reste plus qu'a comparer avec le nombre de cellule total
        // pour savoir si qui des 1 ou 0 est la majorité
        nbVoteUn = grid[id + N] + grid[id - N]               	// haut et bas
                     + grid[id + 1] + grid[id - 1]              // droite et gauche
                     + grid[id + N + 1] + grid[id + N - 1]      // diagonales
                     + grid[id - N + 1] + grid[id - N - 1];

        // Si la cellule est sur le bord, test la cellule à sont opposé sur la grille
        // Première colonne
        if (id % N == 0) {
            nbVoteUn += grid[id + (N - 1)];
        }
        // Dernière colonne
        if (id % N == (N - 1)) {
            nbVoteUn += grid[(id + 1) % N];
        }
        // Première ligne
        if (id >=0 && id < N) {
            nbVoteUn += grid[id + (N * (N - 1))];
        }
        // Dernière ligne
        if (id < (N * N) && id >= (N * (N-1))) {
            nbVoteUn += grid[id - (N * (N - 1))];
        }
        
		// Cellule courante
        int cell = grid[id];
        
        // Règles
        // 5 = chiffre à partir du quel on à la majorité
        // test si les 1 on la majorité, sinon c'est les 0
        if (cell + nbVoteUn >= 5){
        	newGrid[id] = 1;
        }else{
        	newGrid[id] = 0;
        }
    }
}
