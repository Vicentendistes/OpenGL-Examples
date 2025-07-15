__kernel void life(
    __global const uchar* current,
    __global uchar* next,
    const int width,
    const int height,
    const int cells_per_thread
) {
    const int total = width * height;
    const int tid = get_global_id(0);
    const int start = tid * cells_per_thread;
    
    for (int i = 0; i < cells_per_thread; i++) {
        const int idx = start + i;
        if (idx >= total) return;
        
        const int x = idx % width;
        const int y = idx / width;
        
        uchar alive = 0;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                
                const int nx = (x + dx + width) % width;
                const int ny = (y + dy + height) % height;
                alive += current[ny * width + nx];
            }
        }
        
        const uchar cell = current[idx];
        next[idx] = (alive == 3) || (cell && alive == 2) ? 1 : 0;
    }
}