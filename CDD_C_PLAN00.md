# CDD-C Plan: Database Schema & Migration Management (Steps 1-15)

To support live database introspection in `c-orm`, `cdd-c` must provide a reliable way to define, migrate, and spin up a development database schema. The codegen tool needs a living database with the correct schema to interrogate.

### Phase 1: Migration Foundations
- [x] **Step 1:** Define a standard migration directory structure (e.g., `migrations/`).
- [x] **Step 2:** Implement a parser to read `.sql` migration files, separating `UP` (apply) and `DOWN` (rollback) statements.
- [x] **Step 3:** Implement database connection logic for the migration runner using a `DATABASE_URL` environment variable.
- [x] **Step 4:** Automatically create a `schema_migrations` tracking table on the first run to track applied migrations.
- [x] **Step 5:** Implement the `apply` logic to execute pending `UP` migrations in chronological order.
- [x] **Step 6:** Implement the `rollback` logic to execute `DOWN` migrations in reverse chronological order.

### Phase 2: CLI Commands & Tooling
- [x] **Step 7:** Add CLI command: `cdd-c migrate up` to apply pending migrations.
- [x] **Step 8:** Add CLI command: `cdd-c migrate down` to rollback the last applied migration.
- [x] **Step 9:** Add CLI command: `cdd-c migrate create <name>` to generate boilerplate `timestamp_name.sql` files.
- [x] **Step 10:** Add CLI command: `cdd-c db reset` to drop the database, recreate it, and run all `UP` migrations (crucial for clean codegen runs).
- [x] **Step 11:** Implement a `schema.sql` dumper that snapshots the current DB state after migrations run.

### Phase 3: Integration Readiness
- [x] **Step 12:** Implement a CI/CD "headless" mode for automatically creating a test database without user prompts.
- [x] **Step 13:** Add support for seeding the database with test data (e.g., `seeds.sql`).
- [x] **Step 14:** Write unit and integration tests for the migration runner to ensure schema integrity.
- [x] **Step 15:** Document `cdd-c` usage specifically tailored to how it prepares the database for `c-orm` type introspection.
