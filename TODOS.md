# TODOS

## croll v0.0.1

- [ ] End implementation of hash tables (following [the tutorial](https://benhoyt.com/writings/hash-table-in-c/) by Ben Hoyt)
- [x] Bump allocator
  - [x] Bump allocator test
  - [x] Bump allocator linked list expansion
- [x] Pool allocator [(with this amazing source)](https://8dcc.github.io/programming/pool-allocator.html)
  - [x] Variable chunk size
  - [ ] Add checking on poolFree() function to check if certain pointer is really from the pool allocator or if it is just a random pointer
  - [x] Pool allocator test
- [ ] Stack allocator (some sort of `static char memory[1024];` to hold temporary strings, probably will use heap instead)
- [ ] Update croll_strip_prefix
