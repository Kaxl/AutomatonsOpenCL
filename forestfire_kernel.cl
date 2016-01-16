// Kernel of Forest Fire

int isNeighborsOnFire(const int N, __global int *grid, int id);
int isCaseOnFire(int i, __global int *grid); 

__kernel void forestfire(const int N, __global uint *grid, __global uint *newGrid,
                         const double p, const double f, __global double *randoms)
{
    // Get global id
    int id = get_global_id(0);
    double r = randoms[id];

    // Check if we are on the grid
    if ((id / N) < N && (id % N) < N) {
        // If tree is burning, set to nothing
        if (grid[id] == 2) {
            newGrid[id] = 0;
        }
        else if (grid[id] == 3) { // Healthy tree
            if (isNeighborsOnFire(N, grid, id)) {
                // If one of the neighbors is burning, set the tree on fire
                newGrid[id] = 2;
            }
            else {
                // If no neighbor on fire, check probability for spontaneous combustion
                if (r < f)
                    newGrid[id] = 2;
                else
                    newGrid[id] = grid[id];
            }
        }
        else if (grid[id] == 0) { // No tree
            // Check probability for growth
            if (r < p)
                newGrid[id] = 3;
            else
                newGrid[id] = grid[id];
        }
    }
}

// Check if one of the neighbors is on fire
// Return 0 if not, else return value > 0
int isNeighborsOnFire(const int N, __global int *grid, int id) {
    int res = 0;
    res += isCaseOnFire(id + N, grid) + isCaseOnFire(id - N, grid)          // up and down
         + isCaseOnFire(id + 1, grid) + isCaseOnFire(id - 1, grid)          // right and left
         + isCaseOnFire(id + N + 1, grid) + isCaseOnFire(id + N - 1, grid)  // diagonals
         + isCaseOnFire(id - N + 1, grid) + isCaseOnFire(id - N - 1, grid);

    // If cell is on a border, check the other side
    // First column
    //if (id % N == 0) {
    //    res += isCaseOnFire(id + (N - 1), grid);
    //}
    //// Last column
    //if (id % N == (N - 1)) {
    //    res += isCaseOnFire((id + 1) % N, grid);
    //}
    //// First line
    //if (id >=0 && id < N) {
    //    res += isCaseOnFire(id + (N * (N - 1)), grid);
    //}
    //if (id < (N * N) && id >= (N * (N-1))) {
    //    res += isCaseOnFire(id - (N * (N - 1)), grid);
    //}
    return res;
}

// Check if a case is on fire
int isCaseOnFire(int i, __global int *grid) {
    return (grid[i] == 2) ? 1 : 0;
}
