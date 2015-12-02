#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_DATA 512
#define MAX_ROWS 100

struct Address {
  int id;
  int set;
  char *name;
  char *email;
};

struct Database{
  int max_data;
  int max_rows;
  struct Address **rows;
};

struct Connection {
  FILE *file;
  struct Database *db;
};

void
die(const char *message)
{
  if(errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }

  exit(1);
}

void
Address_print(struct Address *addr)
{
  printf("%d %s %s\n", addr->id, addr->name, addr->email);
}

void 
Database_load(struct Connection *conn)
{
  int rc = fread(conn->db, sizeof(struct Database), 1, conn->file);
  if(rc != 1) die("Failed to load database.");
}

struct Connection *
Database_open(const char *filename, char mode)
{
  struct Connection *conn = malloc(sizeof(struct Connection));
  if(!conn) die("Memory error");

  conn->db = malloc(sizeof(struct Database));
  if(!conn->db) die("Memory error");

  if(mode=='c') {
    conn->file = fopen(filename, "w");
  } else {
    conn->file = fopen(filename, "r+");
    
    if(conn->file) {
      Database_load(conn);
    }
  }

  if(!conn->file) die("Failed to open the file");

  return conn;
}

void
Database_close(struct Connection *conn)
{
  if(conn){
    if(conn->file) fclose(conn->file);
    if(conn->db) free(conn->db);
    free(conn);
  }
}

void
Database_write(struct Connection *conn)
{
  rewind(conn->file);

  int rc = fwrite(conn->db, sizeof(struct Database), 1, conn->file);
  if(rc!=1) die("Failed to write database.");

  rc = fflush(conn->file);
  if(rc==-1) die("Cannot flush database.");
}

void
Database_create(struct Connection *conn)
{
  int i = 0;

  for(i=0;i<conn->db->max_rows;i++) {
    // make a prototype to initialize it
    // need to dinamically allocate this structure, otherwise
    // it will die after the function returns.
    struct Address addr = {.id = i, .set = 0};
    // the just assign it
    conn->db->rows[i] = &addr;
  }
}

void 
Database_set(struct Connection *conn, int id, const char *name,
                  const char *email)
{
  // We had to drop de & here, it already contains an address :)
  struct Address *addr = conn->db->rows[id];
  if(addr->set) die("Already set, delete it first");

  addr->set = 1;
  // WARNING: bug, read the "How to Break It" and fix this
  char *res = strncpy(addr->name, name, MAX_DATA);
  // demostrate the strncpy bug
  if(!res) die("Name copy failed");

  res = strncpy(addr->email, email, MAX_DATA);
  if(!res) die("Email copy failed");
}

void
Database_get(struct Connection *conn, int id)
{
  struct Address *addr = conn->db->rows[id];

  if(addr->set){
    Address_print(addr);
  }else{
    die("ID is not set");
  }
}

void 
Database_delete(struct Connection *conn, int id)
{
  struct Address addr = {.id = id, .set = 0};
  conn->db->rows[id] = &addr;
}

void
Database_list(struct Connection *conn)
{
  int i = 0;
  struct Database *db = conn->db;

  for(i=0;i<MAX_ROWS;i++){
    struct Address *cur = db->rows[i];

    if(cur->set){
      Address_print(cur);
    }
  }
}

int main(int argc, char *argv[])
{
    if(argc < 5) die("USAGE: ex17 <dbfile> <max_rows> <max_data> <action> [action params]");

    char *filename = argv[1];
    char action = argv[4][0];
    struct Connection *conn = Database_open(filename, action);
    int id = 0;
    int max_rows = atoi(argv[2]);
    int max_data = atoi(argv[3]);

    if(argc > 5) id = atoi(argv[5]);
    if(id >= MAX_ROWS) die("There's not that many records.");

    switch(action) {
        case 'c':
            Database_create(conn);
            Database_write(conn);
            break;

        case 'g':
            if(argc != 6) die("Need an id to get");

            Database_get(conn, id);
            break;

        case 's':
            if(argc != 8) die("Need id, name, email to set");

            Database_set(conn, id, argv[6], argv[7]);
            Database_write(conn);
            break;

        case 'd':
            if(argc != 6) die("Need id to delete");

            Database_delete(conn, id);
            Database_write(conn);
            break;

        case 'l':
            Database_list(conn);
            break;
        default:
            die("Invalid action, only: c=create, g=get, s=set, d=del, l=list");
    }

    Database_close(conn);

    return 0;
}
