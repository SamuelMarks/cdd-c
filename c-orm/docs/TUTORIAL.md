# End-to-End Tutorial: From SQL to C Application

This tutorial demonstrates the workflow of defining a database schema, generating C models with `cdd-c`, and performing database operations via `c-orm`.

## 1. Defining the Schema

First, define your schema in a standard `schema.sql` file:

```sql
CREATE TABLE users (
    id INT PRIMARY KEY,
    username VARCHAR(255) NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    age INT,
    score FLOAT,
    is_active BOOLEAN,
    created_at TIMESTAMP
);
```

Notice that constraints like `NOT NULL` are used to dictate whether generated C fields become raw primitives or pointers to primitives.

## 2. Generating the C Models

Use the `cdd-c` CLI tool to generate the boilerplate `Models.c` and `Models.h` files:

```bash
cdd-c sql2c schema.sql ./out_dir
```

This will produce:
- `Models.h`: Struct definitions (`struct Users`, `struct Users_Array`) and cleanup helpers.
- `Models.c`: Metadata definitions mapping `c-orm` metadata into these C structs.

Include `Models.h` and link against `Models.c` when building your application.

## 3. Writing the C Application

Your code will need the `c_orm_sqlite.h` header for the SQLite driver initialization, and `c_orm_api.h` for core functionality:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_orm_sqlite.h"
#include "c_orm_api.h"
#include "out_dir/Models.h" /* Generated */

int main(void) {
    c_orm_db_t *db = NULL;
    c_orm_error_t err;
    struct Users user;
    int32_t age = 30;
    float score = 9.5f;
    bool is_active = true;
    
    /* 1. Connect */
    err = c_orm_sqlite_connect("app.db", &db);
    if (err != C_ORM_OK) return 1;

    /* 2. Setup (If table does not exist) */
    c_orm_execute_raw(db, "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY,"
        "username VARCHAR(255) NOT NULL,"
        "email VARCHAR(255) UNIQUE NOT NULL,"
        "age INTEGER,"
        "score FLOAT,"
        "is_active BOOLEAN,"
        "created_at TIMESTAMP"
        ");");

    /* 3. Insert */
    memset(&user, 0, sizeof(user));
    user.id = 1;
    user.username = "smarks";
    user.email = "samuel@example.com";
    user.age = &age;
    user.score = &score;
    user.is_active = &is_active;
    user.created_at = "2026-03-14 12:00:00";

    c_orm_insert(db, &Users_meta, &user);

    /* 4. Query */
    struct Users fetched_user;
    memset(&fetched_user, 0, sizeof(fetched_user));
    
    err = c_orm_find_by_id_int32(db, &Users_meta, 1, &fetched_user);
    if (err == C_ORM_OK) {
        printf("Hello %s!\n", fetched_user.username);
    }

    /* 5. Cleanup */
    Users_free(&fetched_user); /* Required: Free retrieved strings/pointers */
    /* Note: Don't call Users_free(&user) unless it was dynamically allocated! */
    return 0;
}
```

By leveraging `cdd-c` alongside `c-orm`, database interaction in C becomes completely type-safe and vastly more efficient than manually writing `sqlite3_bind` routines.