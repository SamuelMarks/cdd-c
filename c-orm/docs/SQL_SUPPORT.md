# cdd-c SQL DDL Support

The `cdd-c` parser strictly supports a specific subset of SQL DDL for generating C structs and metadata. This ensures predictable memory layouts and consistent code generation across supported databases.

## Supported Data Types

The following SQL data types are fully parsed and mapped to `c-orm` C types:

* **Integers**: `INT`, `INTEGER` -> `int32_t` (`C_ORM_TYPE_INT32`)
* **Big Integers**: `BIGINT` -> `int64_t` (`C_ORM_TYPE_INT64`)
* **Floating Point**: `FLOAT` -> `float` (`C_ORM_TYPE_FLOAT`)
* **Double Precision**: `DOUBLE`, `DECIMAL` -> `double` (`C_ORM_TYPE_DOUBLE`)
* **Booleans**: `BOOL`, `BOOLEAN` -> `bool` (`C_ORM_TYPE_BOOL`)
* **Strings**: `VARCHAR(n)`, `TEXT`, `CHAR` -> `char *` (`C_ORM_TYPE_STRING`)
* **Dates/Timestamps**: `DATE` -> `char *` (`C_ORM_TYPE_DATE`), `TIMESTAMP` -> `char *` (`C_ORM_TYPE_TIMESTAMP`)

## Supported Constraints

The parser extracts constraints to provide accurate metadata for primary keys, foreign keys, and nullability:

* `PRIMARY KEY`: Marks the column as the primary key. If composite, handled correctly across multiple metadata rows.
* `FOREIGN KEY`: Captures the `REFERENCES table(column)` constraint. Currently exposed in metadata array as the target table name string.
* `NOT NULL`: Marks the column as non-nullable. If a column is omitted from this constraint, it is assumed `NULL` by default and is represented as a pointer in the generated C structs (e.g., `int32_t *`).
* `UNIQUE`: Parsed but currently does not affect struct generation; useful for database integrity checks only.
* `DEFAULT`: Values (numbers, strings, `CURRENT_TIMESTAMP`) are parsed but are handled by the database engine upon insertion rather than the C structs themselves.

## Example

```sql
CREATE TABLE users (
    id INT PRIMARY KEY,
    username VARCHAR(255) NOT NULL,
    score FLOAT, /* nullable float */
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```
