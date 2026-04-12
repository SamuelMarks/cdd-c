# cdd-c AST Transformers

This directory contains standalone, AST-aware transformers that modify C codebases directly while perfectly preserving all formatting, macros, and comments.

## Creating a new Transformer

1. Create a new directory for your transformer (e.g., `src/transformers/my_transformer`).
2. Create a `.c` file inside it (e.g., `my_transformer.c`).
3. Include the standard AST and core headers you need (e.g., `classes/parse/cdd_cst_query.h`, `classes/parse/cdd_cst_mutate.h`).
4. Define a transformation function conforming to the following signature (defined in `include/cdd_cst_transform.h`):

```c
int cdd_transform_my_tool(cdd_cst_tree_t *tree, const cdd_transform_config_t *config);
```

5. Declare your function prototype in `include/cdd_cst_transform.h`.
6. Add the source file to `src/CMakeLists.txt`.
7. Map your tool to the CLI interface in `src/routes/parse/cli_cst.c` so users can run it via `cdd-c transformer my_tool`.
8. Use `cdd_cst_find_nodes_by_type` to locate the target `CDD_CST_NODE` items, manipulate them using `cdd_cst_node_replace` or `cdd_cst_node_insert_after`, and return `0` on success.
9. Write comprehensive unit tests in `src/tests/transformers/my_transformer/test_my_tool.h` to ensure 100% correctness and 0 memory leaks.
