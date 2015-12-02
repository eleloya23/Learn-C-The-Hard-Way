#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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
  int rc = fread(&conn->db->max_rows, sizeof(int), 1, conn->file);
  if(rc != 1) die("Failed to load MAX_ROWS.");

  rc = fread(&conn->db->max_data, sizeof(int), 1, conn->file);
  if(rc != 1) die("Failed to load MAX_DATA.");

  conn->db->rows = malloc(conn->db->max_rows * sizeof(struct Address*));

  for(int i=0;i<conn->db->max_rows;i++) {
    struct Address *addr = malloc(sizeof(struct Address));
    rc = fread(&(addr->id), sizeof(int),1,conn->file);
    if(rc != 1) die("Failed to load address->id");

    rc = fread(&(addr->set), sizeof(int),1,conn->file);
    if(rc != 1) die("Failed to load address->set");
    
    addr->name = malloc(conn->db->max_data * sizeof(char));
    addr->email= malloc(conn->db->max_data * sizeof(char));

    rc = fread(addr->name,sizeof(char),conn->db->max_data,conn->file);
    if(rc != conn->db->max_data) die("Failed to load address->name");

    rc = fread(addr->email,sizeof(char),conn->db->max_data,conn->file);
    if(rc != conn->db->max_data) die("Failed to load address->email");

    conn->db->rows[i] = addr;
  }

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
    
    for(int i=0;i<conn->db->max_rows;i++){
      if(conn->db->rows[i]){
        free(conn->db->rows[i]->name);
        free(conn->db->rows[i]->email);
        free(conn->db->rows[i]);
      }
      }

    if(conn->db) free(conn->db);
    free(conn);
  }
}

void
Database_write(struct Connection *conn)
{
  rewind(conn->file);
  /*
   * max_rows,max_data,id,set,NAAAME,EEMAIL,id,set,NAAAME,EEMAIL,....
   */
  
  int rc;
  //Write the max_rows columns
  rc = fwrite(&conn->db->max_rows, sizeof(conn->db->max_rows), 1, conn->file);
  if(rc!=1) die("Failed to write MAX_ROWS.");

  rc = fwrite(&conn->db->max_data, sizeof(conn->db->max_data), 1, conn->file);
  if(rc!=1) die("Failed to write MAX_DATA.");

  for(int i=0; i<conn->db->max_rows; i++){
    struct Address *address = conn->db->rows[i];

    rc = fwrite(&address->id, sizeof(address->id), 1, conn->file);
    if(rc!=1) die("Failed to write address->id");

    rc = fwrite(&address->set, sizeof(address->set), 1, conn->file);
    if(rc!=1) die("Failed to write address->set");

    rc = fwrite(address->name, sizeof(char), conn->db->max_data,  conn->file);
    if(rc!=conn->db->max_data) die("Failed to write address->name");

    rc = fwrite(address->email, sizeof(char), conn->db->max_data, conn->file);
    if(rc!=conn->db->max_data) die("Failed to write address->email");
  }

  rc = fflush(conn->file);
  if(rc==-1) die("Cannot flush database.");
}

void
Database_create(struct Connection *conn, int max_rows, int max_data)
{
  int i = 0;

  conn->db->max_rows = max_rows;
  conn->db->max_data = max_data;
  conn->db->rows = malloc(max_rows * sizeof(struct Address*));

  for(i=0;i<conn->db->max_rows;i++) {
    // make a prototype to initialize it
    // need to dinamically allocate this structure, otherwise
    // it will die after the function returns.
    struct Address *addr = malloc(sizeof(struct Address));
    addr->id =i;
    addr->set=0;
    addr->name = malloc(max_data * sizeof(char));
    addr->email= malloc(max_data * sizeof(char));

    memset(addr->name, 0, max_data * sizeof(char) );
    memset(addr->email, 0, max_data * sizeof(char) );
    memset(addr->name, '-', max_data * sizeof(char) -1);
    memset(addr->email, '_', max_data * sizeof(char) -1);
    // the just assign it
    conn->db->rows[i] = addr;
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
  char *res = strncpy(addr->name, name, conn->db->max_data);
  // demostrate the strncpy bug
  if(!res) die("Name copy failed");

  res = strncpy(addr->email, email, conn->db->max_data);
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
  // We don't really delete the entry, because that's expensive, we just write the "set" flag as 0
  struct Address *addr = conn->db->rows[id];
  addr->set=0;
}

void
Database_list(struct Connection *conn)
{
  int i = 0;
  struct Database *db = conn->db;

  for(i=0;i<db->max_rows;i++){
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
    if(id >= max_rows) die("There's not that many records.");

    switch(action) {
        case 'c':
            Database_create(conn,max_rows,max_data);
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
