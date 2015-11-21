# Exercise 17: Heap And Stack Memory Allocation

## Extra Credit
### The die function need to be augmented to let you pass the conn variable so it can close it and clean up.
[[Solution Code]](ex17_e1.c)

This one was very easy. Just add a `struct Connection *conn` parameter to the `die()` function [[1]](ex17_e1.c#L37). Then make sure you send that pointer everytime you call `die()`

Whenever you have to call `die()`, there is always a "struct Connection" pointer available. The only time there isn't, is at the beginning of `main()`, that's why you send a NULL pointer to the `die()` function [[2]](ex17_e1.c#L163).
