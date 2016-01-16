// Kernel of Game Of Life

__kernel void gameoflife(const int N, __global int *grid, __global int *newGrid)
{
    // Get global id
    int id = get_global_id(0);

    int nbNeighbors;
    // Check if we are on the grid
    if ((id / N) < N && (id % N) < N) {
        // Get the number of neighbors for a cell
        nbNeighbors = grid[id + N] + grid[id - N]               // up and down
                     + grid[id + 1] + grid[id - 1]              // right and left
                     + grid[id + N + 1] + grid[id + N - 1]      // diagonals
                     + grid[id - N + 1] + grid[id - N - 1];

        // If cell is on a border, check the other side
        // First column
        if (id % N == 0) {
            nbNeighbors += grid[id + (N - 1)];
        }
        // Last column
        if (id % N == (N - 1)) {
            nbNeighbors += grid[(id + 1) % N];
        }
        // First line
        if (id >=0 && id < N) {
            nbNeighbors += grid[id + (N * (N - 1))];
        }
        if (id < (N * N) && id >= (N * (N-1))) {
            nbNeighbors += grid[id - (N * (N - 1))];
        }

        int cell = grid[id];
        // Game rules
        if (cell == 1 && nbNeighbors < 2)
            newGrid[id] = 0;
        else if (cell == 1 && (nbNeighbors == 2 || nbNeighbors == 3))
            newGrid[id] = 1;
        else if (cell == 1 && nbNeighbors > 3)
            newGrid[id] = 0;
        else if (cell == 0 && nbNeighbors == 3)
            newGrid[id] = 1;
        else
            newGrid[id] = cell;
    }
}
