# Exercise 17: Heap And Stack Memory Allocation

## Extra Credit
### The die function need to be augmented to let you pass the conn variable so it can close it and clean up.
[Solution Code](ex17_e1.c)

This one was very straight forward. Just needed to add a "struct Connection" parameter to the die function. Whenever you had to call die, there was always a "struct Connection" pointer available. The only time you didn't was at the beginning of the main function, that's why we send a NULL pointer to the die function.
