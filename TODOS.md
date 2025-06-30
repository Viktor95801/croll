# TODOS

## croll v0.0.1

### Todos

- [x] End implementation of hash tables (following [the tutorial](https://benhoyt.com/writings/hash-table-in-c/) by Ben Hoyt)
  - [x] Hash table test
  - [ ] Fix excessive complexity allegations (PoolAlloc instead of strdup, etc)
- [x] Bump allocator
  - [x] Bump allocator test
  - [x] Bump allocator linked list expansion
- [x] Pool allocator [(with this amazing source)](https://8dcc.github.io/programming/pool-allocator.html)
  - [x] Variable chunk size
  - [ ] Add checking on poolFree() function to check if certain pointer is really from the pool allocator or if it is just a random pointer
  - [x] Pool allocator test
  - [x] Add expand functionality to pool allocator (maybe make the expansion mode onto a separate function? (instead of normal and expand on croll_poolAlloc, make a croll_poolAllocOrExpand))
- [ ] Update croll_strip_prefix before croll v0.0.2
- [ ] Optimize croll_readEntireFile.
- [ ] Add croll_readDir (returning a list of paths on it). Maybe add a croll_readDirR? (R = recursive)
- [ ] More utility functions for files and directories (maybe even for running CMD processes).

### Ideas

- [ ] Add croll_daMax(da) and croll_daMin(da) macros
- [ ] Stack allocator (some sort of `static char memory[1024];` to hold temporary strings, probably will use heap instead)
