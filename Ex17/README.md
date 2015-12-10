# Exercise 17: Heap And Stack Memory Allocation

## Extra Credit
### The die function need to be augmented to let you pass the conn variable so it can close it and clean up.
[[Solution Code]](ex17_e1.c)

This one was very easy. Just add a `struct Connection *conn` parameter to the `die()` function [[1]](ex17_e1.c#L37). Then make sure you send that pointer everytime you call `die()`

Whenever you have to call `die()`, there is always a "struct Connection" pointer available. The only time there isn't, is at the beginning of `main()`, that's why you send a NULL pointer to the `die()` function [[2]](ex17_e1.c#L163).

### Change the code to accept parameters for MAX_DATA and MAX_ROWS, store them in the Database struct, and write that to the file, thus creating a database that can be arbitrarily sized.
[[Solution Code]](ex17_e2.c)

This seemingly easy credit turns out to be quite hard for a beginner. Arbitrarily sized is the keyword here, you'll have to change your whole code to add some pointer magic. So what's the magic sauce here? I broke the problem in five manageable parts.

**First Step**. You need to remove the `MAX_ROWS` and `MAX_DATA` constants. Modify the `structs` to have variable sized strings and a variable sized array of `struct Address` [[1]](ex17_e2.c#L7-L18). 

**Second Step**. You need to parse two new command line arguments, maxrows and maxdata, in the `main()` function [[2]](https://www.diffchecker.com/v8fp7fwj).

After doing the first two steps, the drill is to basically modify every `Database_something()`.

**Third Step**. This is probably the hardest step.  `database_create()` needs to take max_rows and max_data as parameters [[3]](ex17_e2.c#L155-L181). Specify a format for your database file in the `database_write()` function [[4]](ex17_e2.c#L119-L153).

**Fourth Step**. Very easy rewrites here. Replace  the MAX_DATA constant in `Database_set()` with the new max_data variable [[5]](https://github.com/eleloya/Learn-C-The-Hard-Way/commit/2c07813f833ff5520591a1b5e222f3cf40b18a82?diff=unified). Likewise you'll have to replace MAX_ROWS with max_rows in `Database_list()` [[6]](https://github.com/eleloya/Learn-C-The-Hard-Way/commit/331e4cb6bc047507c6658914c51bc55361d7ee10). Also change the code in `Database_delete()` to just rewrite the Set variable.  [[7]](https://github.com/eleloya/Learn-C-The-Hard-Way/commit/0f1e09fac57412be73d211a3a34a4894bbd2fc7e).

**Fifth Step**. The last step consists in reading and closing the database. To read the database, you'll need to mix code from the write and create function [[7]](https://github.com/eleloya/Learn-C-The-Hard-Way/commit/d1b6454935da2ab3b50e3bf9f1dd7cee148abd53). There's a lot of dynamic memory being used, so it's important to make sure to free those resources at the end [[8]](https://github.com/eleloya/Learn-C-The-Hard-Way/commit/9d7ee17a84d3c85a89abd3ed2598bc31516d0f48).

### Add more operations you can do on the database, like find.
[[Solution Code]](ex17_e3.c)

Here is my crude but easy solution:
```c
void
Database_find(struct Connection *conn, const char *query)
{
  //Super slow implementation :p
  int not_found = 1;
  for(int i=0;i<MAX_ROWS;i++){
    struct Address *addr = &conn->db->rows[i];

    if( 
        ((strcmp(query,addr->name)==0) || (strcmp(query,addr->email)==0)) && 
            addr->set ){
      Address_print(addr);
      not_found = 0;
    }

  }
  if(not_found) die("Unable to found a record with that name/email");
}
```
Dont forget to add the new action in `main`
```c
switch(action) {
    //...
    case 'f':
        if(argc != 4) die("Need a name/email to find");

        Database_find(conn,argv[3]);
        break;
    //...
}
```

Find in action:
```sh
$ ./ex17 db.dat c
$ ./ex17 db.dat s 1 zed zed@zedshaw.com
$ ./ex17 db.dat s 2 zad zad@zadshaw.com
$ ./ex17 db.dat f zed
1 zed zed@zedshaw.com
$ ./ex17 db.dat f zed@zedshaw.com
1 zed zed@zedshaw.com
```

### Read about how C does it's struct packing, and then try to see why your file is the size it is. See if you can calculate a new size after adding more fields.

You just have to sum the byte size of each member. 

```c
struct Address {
  int id;  //4
  int set; //4
  char name[MAX_DATA]; //512
  char email[MAX_DATA]; //512
};

struct Database{
  struct Address rows[MAX_ROWS]; //1032 * 100 == 103,200
};
```

```sh
$ ./ex17 db.dat c
$ ls -la db.dat
-rw-r--r--  1 ele  staff  103200  3 Dec 02:00 db.dat
```

When your members are not byte-aligned by 4 or 8, C will add some padding bytes. 
It's better if you see the example below.

```c
struct Address {
  int id;  //4
  char initial; //1
  int set; //4
  char name[MAX_DATA]; //512
  char email[MAX_DATA]; //512
  // 1033
  // But C internaly adds 3 more bytes for padding. 
  // char padding[3];
  // See reference: http://www.catb.org/esr/structure-packing/
  // 1036
  // Now, it is divisible by 4
  // 
};

struct Database{
  struct Address rows[MAX_ROWS]; //1036 * 100 == 103,600
};
```

```sh
$ ./ex17 db.dat c
$ ls -la db.dat
-rw-r--r--  1 ele  staff  103600  3 Dec 02:04 db.dat
```

### Add some more fields to the Address and make them searchable.
[[Solution Code]](ex17_e5.c)

Just add a lastname field to the data structure.

```c
struct Address {
 int id;
 int set;
 char name[MAX_DATA];
 char lastname[MAX_DATA];
 char email[MAX_DATA];
};
```

After that you need to incorporate the new field in `main()`[[1]](ex17_e5.c#L210-L215)[[2]](ex17_e5.c#L229) `Address_print()`[[3]](ex17_e5.c#L42), `Database_set()`[[4]](ex17_e5.c#L127-L128), `Database_find()`[[5]](ex17_e5.c#L142). Everything else stays the same.

### Write a shell script that will do your testing automatically for you by running commands in the right order. Hint: Use set -e at the top of a bash to make it abort the whole script if any command has an error.
[[Solution Code]](test.sh)

The Makefile has to look like this
```Makefile
CFLAGS=-Wall -g

test:
	./test.sh

clean:
	rm -rf a.out ex17 db.dat *.dSYM
```

The script `test.sh`
```bash
#!/bin/bash
set -e

echo "Create database db.dat"
./ex17 db.dat c

echo "Set 1 foo foo@bar.com"
./ex17 db.dat s 1 foo foo@bar.com

echo "Set 2 baz baz@qux.com"
./ex17 db.dat s 2 baz baz@qux.com

echo "Printing all records"
./ex17 db.dat l

echo "Deleting record 1"
./ex17 db.dat d 1

echo "Getting record 2"
./ex17 db.dat g 2
```

To run this script just type `make test`


### Try reworking the program to use a single global for the database connection. How does this new version of the program compare to the other one?
[[Solution code]](ex17_e7.c)

Pros:
* Less code
* Fewer arguments

Cons:
* Not reusable functions
* Global state is risky


### Go research "stack data structure" and write one in your favorite language, then try to do it in C.
[[Solution Code]](ex17_e8.c)

My favorite language is Ruby, so:

```ruby
class Stack
  def initialize
    @elements = []
  end

  def push(arg)
    @elements << arg
  end

  def pop()
    @elements.pop
  end

end
```

For C, to represent the Stack Structure, I ended up using a `Struct` with  `int` and an `int []`. I wanted to use function pointers, but, thats for the next chapter of this book.

```C
#include <stdio.h>

#define MAX_SIZE 100

struct Stack{
  int top;
  int elements[MAX_SIZE];
};

struct Stack stack;

void
push(int n)
{
  stack.elements[stack.top] = n;
  stack.top++;
}

int
pop()
{
 stack.top--;
 int n = stack.elements[stack.top];
 return n;
}

int
main(int argc, char **argv){
  stack.top = 0;
  push(1);
  push(2);
  push(5);

  int a = pop();
  int b = pop();

  printf("a = %d\nb = %d\n", a, b);

  return 0;
}
```
