# CDD FFI Intermediate Representation (IR) DSL

The CDD FFI IR Domain Specific Language (DSL) is a human-readable, highly structured interface definition language used to map C and C++ native code into an intermediate format.

This DSL allows developers to explicitly define APIs, data models, namespaces, and macros. It has been meticulously designed to prioritize **lossless roundtripping** (formatting and comments survive translation exactly), machine parsability, and deterministic semantics.

## Core Design Principles

1. **Left-to-Right Typing:** Unlike C's notoriously ambiguous "inside-out" declaration syntax, the DSL uses a strict left-to-right type composition scheme (inspired by Rust and Zig).
2. **Explicit Semantic Anchoring:** Macros and memory intents (`[in]`, `[out]`) are first-class structural citizens, avoiding the pitfalls of blind C preprocessor text replacement.
3. **Lossless Trivia:** Comments and whitespace are gathered and mapped specifically to the syntax nodes they precede, allowing code generators to emit bindings that preserve your exact original documentation.
4. **Raw Unambiguous Blocks:** C code blocks are fenced with `%{ ... }%`, making lexing deterministic even if the inner code contains unbalanced strings or quotes.

---

## EBNF Grammar Overview

The parser implements the following exact specification:

```ebnf
<Program> ::= <Declaration>*

<Declaration> ::= <NamespaceDecl>
                | <StructDecl>
                | <EnumDecl>
                | <FuncDecl>
                | <MacroDecl>

<NamespaceDecl> ::= "namespace" <Identifier> "{" <Declaration>* "}"

<StructDecl> ::= "struct" <Identifier> "{" <FieldDecl>* "}"
<FieldDecl> ::= <AttributeList>? <Type> <Identifier> ";"

<EnumDecl> ::= "enum" <Identifier> "{" <EnumVariantList>? "}"
<EnumVariantList> ::= <EnumVariant> ( "," <EnumVariant> )* ","?
<EnumVariant> ::= <Identifier> ( "=" <NumberLiteral> )?

<FuncDecl> ::= <FuncModifier>* "func" <Identifier> "(" <FuncParams>? ")" <PureSpecifier>? ( "{" <FuncBody> "}" | ";" )
<FuncModifier> ::= "virtual" | "pure"
<PureSpecifier> ::= "=" "0"
<FuncParams> ::= <FieldDecl> ( "," <FieldDecl> )*
<FuncBody> ::= ( <RawBody> | <MacroInvocation> ";" )*

<MacroDecl> ::= "macro_const" <Identifier> "=" <NumberLiteral> ";"
              | "macro_expr" <Identifier> "(" <IdentifierList>? ")" "=" <RawBody> ";"
              | "macro_stmt" <Identifier> "(" <IdentifierList>? ")" "=" <RawBody> ";"

<RawBody> ::= "body" "%{" <RawText> "}%"
<MacroInvocation> ::= "@expr" "(" <RawText> ")" | "@stmt" "(" <RawText> ")"

<AttributeList> ::= "[" <Attribute> ( "," <Attribute> )* "]"
<Attribute> ::= "in" | "out" | "inout" | "len" "=" <Identifier>

<Type> ::= <TypeModifier>* <BaseType>
<TypeModifier> ::= "*mut" | "*const" | "[" <NumberLiteral> "]" | "[" <Identifier> "]"
<BaseType> ::= "void" | "i8" | "u8" | "i32" | "u32" | "i64" | "f32" | "bool" | "size_t" | "char"

<TriviaEscape> ::= "#!ws" "(" <StringLiteral> ")" | "#!comment" "(" <StringLiteral> ")"
```

---

## Feature Deep-Dive & Examples

### 1. Namespaces and Names
A file can be wrapped in a namespace which dictates the target library or module bindings.

```c
/* This comment becomes the leading trivia for the namespace */
namespace MyAudioEngine {
    ...
}
```

### 2. Type Syntax & Intents
Pointers are specified explicitly as `*mut` (mutable) or `*const` (immutable), stacking from left to right. Arrays are brackets on the left. Memory data-flow intents (`in`, `out`) are explicitly bracketed prior to the type.

```c
/* A function taking an array of mutable bytes, and writing to an output size */
func write_buffer([in] *const u8 buf, [inout, len=buf] *mut size_t buf_len);

/* A complex pointer: A mutable pointer to a constant pointer of i32 */
func multi_depth([in] *mut *const i32 matrix);
```

### 3. Fenced Raw C Bodies
When defining custom C-level logic inside the DSL, we escape the parsing tree using `%{ ... }%`. This prevents lexer fragmentation when parsing complicated native logic.

```c
func hash_password([in] *const char input) {
    body %{
        if (!input) return 0;
        uint32_t hash = 5381;
        int c;
        while ((c = *input++)) hash = ((hash << 5) + hash) + c;
        return hash;
    }%
}
```

### 4. Semantic Macros
Instead of traditional `#define` textual replacement, the IR requires identifying the structural intent of the macro.
When invoking a macro inside a function, explicit annotations tell the parser what tree structure to expect.

```c
// Macro definitions
macro_const MAX_RETRIES = 5;
macro_expr LOG_ERROR(msg) = body %{ fprintf(stderr, "%s", msg) }%;

func attempt_connection() {
    // Macro Invocation
    @stmt( LOG_ERROR("Failed to connect!") );
}
```

### 5. Enums and C23 Numeric Literals
Numbers inside the DSL directly support natively parsed prefixes (`0x`, `0b`) and C23 digit separators (`'`).

```c
enum ConnectionStatus {
    DISCONNECTED = 0,
    CONNECTED = 0x01,
    TIMEOUT = 1'000'000  // Supported natively!
}
```

### 6. Escape Hatches
In automated toolchains, there may be a requirement to inject trivia without relying on the parser's implicit heuristic anchoring. You can force injection using `#!ws` and `#!comment`.

```c
#!ws("\n\n")
#!comment("/* AUTO-GENERATED - DO NOT EDIT */")
struct Config {
    [in] i32 flag;
}
```

## Integrating into cdd-c

The DSL parser is fully integrated into the existing `cdd_api.h` pipeline. You can convert any DSL string into a native `cdd_ffi_ir_t` AST and subsequently emit language bindings directly.

```c
cdd_generate_bindings_config_t config = {0};
config.output_dir = "out/";
config.target_langs = "rust,python,dsl";

// Generates bindings AND losslessly reconstructs the DSL format!
cdd_c_error_t rc = cdd_generate_bindings_from_dsl(dsl_source_string, &config);
```
