# CDD-C Usage & c-orm Integration

`cdd-c` provides a complete database migration management system out of the box to securely prepare target development schemas prior to `c-orm` code generation.

The code generator strictly introspects database schema artifacts locally to ensure C type safety, which mandates a live running database equipped with identical schema profiles.

## Configuration

To establish the environment variable configuration globally across CLI tools:

```bash
export DATABASE_URL="postgresql://user:password@localhost:5432/my_dev_db"
```

## Creating & Running Migrations

```bash
# Generate a new boilerplate .sql file stamped by timestamp inside ./migrations/
cdd-c migrate create init_users

# Once written, apply all pending forward chronological UP migrations
cdd-c migrate up

# Rollback the last applied migration natively
cdd-c migrate down
```

## CI/CD Pipeline Operations

For headless systems such as GitHub Actions, `cdd-c` integrates seamlessly to scaffold dynamic integration testing environments. 

Point `DATABASE_URL` momentarily to the administrative `postgres` DB to establish the target.

```bash
# Administrative context
export DATABASE_URL="postgresql://postgres:pass@localhost:5432/postgres"

# Establish target database (if it doesn't already exist)
cdd-c setup_test_db my_ci_test_db

# Pivot context towards the created database
export DATABASE_URL="postgresql://postgres:pass@localhost:5432/my_ci_test_db"

# Force clear all schemas & execute all UP migrations entirely
cdd-c db reset

# Execute a direct initialization payload of default parameters/entities
cdd-c seed seeds.sql
```

## Exposing the Schema to c-orm

To perform programmatic extraction, run the schema dumper. This guarantees your `c-orm` generators operate off the pristine schema state.

```bash
# Extract the active schema structure utilizing pg_dump wrapper
cdd-c schema dump schema.sql
```

Now, `c-orm` can aggressively compile structural mapping layers across all verified SQLite/PostgreSQL/MySQL models.
