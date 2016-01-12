// Kernels of Game Of Life
// Ghosts kernel are use to keep the game going when reaching the end of the grid

__kernel void ghostRows(const int N, __global int *grid)
{
    int id = get_global_id(0);

    if (id < N)
    {
        grid[(N + 2) * ( N + 1) + id] = grid[N + 2 + id];   // Copy first real row to bottom ghost row
        grid[id] = grid[N + 2 * N + id];                    // Copy last real row to top ghost row
    }
}

__kernel void ghostCols(const int N, __global int *grid)
{
    int id = get_global_id(0);

    if (id <= N)
    {
        grid[id * N + 2 + N] = grid[id * N + 2]; // Copy first real column to right most ghost column
        grid[id * N + 2] = grid[id * N + 2 + N]; // Copy last real column to left most ghost column
    }
}

__kernel void gol(const int N, __global int *grid, __global int *newGrid)
{
    // Get global id 
    int id = get_global_id(0);

    int nbNeighbors;
    // Check if we are on the grid
    if ((id / N) < N && (id % N) < N) {
        // Get the number of neighbors for a cell
        nbNeighbors = grid[id + N] + grid[id - N]               // up and down
                     + grid[id + 1] + grid[id - 1]              // right and left
                     + grid[id + N + 1] + grid[id - N + 1]      // diagonals
                     + grid[id - N + 1] + grid[id + N + 1];

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
