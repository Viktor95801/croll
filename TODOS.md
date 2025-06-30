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
- [ ] Optimize croll_readEntireFile
  - [ ] Use fread + buffered chunks instead of fgetc (improves speed drastically)
- [ ] Improve safety of croll__htSetEntry
  - [ ] Replace strcpy with strncpy (check buffer size vs key_max_len)
- [ ] Make croll_textSubString safer
  - [ ] Document clearly that buffer is rotating and non-thread-safe
  - [ ] Consider providing copy variant to avoid buffer invalidation
- [ ] Improve croll_htGet() performance which needs tuse of the croll_texSubString function
- [ ] Improve croll_checkNullPtr macro
  - [ ] Avoid `return` inside macro, may cause undefined behavior
- [ ] Replace __STATIC_FUNCTION with CROLL_STATIC for convention and safety
- [ ] Add validation to croll_daLast to avoid undefined behavior if len == 0
- [ ] Add optional hash caching in croll_HtEntry to avoid repeated strcmp calls

### Ideas

- [ ] Add croll_daMax(da) and croll_daMin(da) macros
- [ ] Stack allocator (some sort of `static char memory[1024];` to hold temporary strings, probably will use heap instead)
- [ ] Add croll_daClear(da) to reset len without freeing
- [ ] Add croll_sbClear(sb) as alias to daClear
- [ ] Benchmark malloc vs pool allocator
- [ ] Consider thread-safe variant of croll_textFmt using TLS or stack buffer
