/* clang-format off */
#include "cdd_ffi_emit_java.h"
#include <stdio.h>
#include "c_cdd/format_specifiers.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static const char *get_java_primitive(cdd_ffi_type_t type) {
  if (type.pointer_depth > 0) {
    if (type.kind == CDD_FFI_KIND_INT8 || type.kind == CDD_FFI_KIND_UINT8) {
      return "String"; /* Typically string for char* or Pointer */
    }
    return "Pointer"; /* JNA Pointer */
  }

  switch (type.kind) {
  case CDD_FFI_KIND_VOID:
    return "void";
  case CDD_FFI_KIND_BOOL:
    return "boolean";
  case CDD_FFI_KIND_INT8:
    return "byte";
  case CDD_FFI_KIND_UINT8:
    return "byte";
  case CDD_FFI_KIND_INT16:
    return "short";
  case CDD_FFI_KIND_UINT16:
    return "short";
  case CDD_FFI_KIND_INT32:
    return "int";
  case CDD_FFI_KIND_UINT32:
    return "int";
  case CDD_FFI_KIND_INT64:
    return "long";
  case CDD_FFI_KIND_UINT64:
    return "long";
  case CDD_FFI_KIND_FLOAT32:
    return "float";
  case CDD_FFI_KIND_FLOAT64:
    return "double";
  case CDD_FFI_KIND_FUNCTION_PTR:
    return "Callback";
  case CDD_FFI_KIND_STRUCT_REF:
    return type.ref_name ? type.ref_name : "Pointer";
  case CDD_FFI_KIND_ENUM_REF:
    return "int"; /* Map enum to int in JNA by default */
  case CDD_FFI_KIND_TYPEDEF_REF:
    return type.ref_name ? type.ref_name : "Pointer";
  case CDD_FFI_KIND_OPAQUE_PTR:
    return "Pointer";
  default:
    return "Pointer";
  }
}

static void to_camel_case(const char *snake, char *out, size_t out_size) {
  size_t i, j = 0;
  int up = 1;
  for (i = 0; i < strlen(snake) && j < out_size - 1; i++) {
    if (snake[i] == '_') {
      up = 1;
    } else {
      if (up) {
        out[j++] = (char)toupper(snake[i]);
        up = 0;
      } else {
        out[j++] = snake[i];
      }
    }
  }
  out[j] = '\0';
}

static enum cdd_c_error
emit_java_file(cdd_ffi_ir_t *ir, const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;
  size_t i, j;
  const char *lib_name = config->library_name ? config->library_name : "MyLib";
  char class_name[256];

  to_camel_case(lib_name, class_name, sizeof(class_name));

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\%s.java", config->output_dir,
               class_name);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/%s.java", config->output_dir,
               class_name);
  f = fopen(filepath, "w");
  /* LCOV_EXCL_START */
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  /* LCOV_EXCL_STOP */
#endif

  fprintf(f, "// Auto-generated JNA bindings for %s\n\n", lib_name);
  fprintf(f, "import com.sun.jna.Library;\n");
  fprintf(f, "import com.sun.jna.Native;\n");
  fprintf(f, "import com.sun.jna.Pointer;\n");
  fprintf(f, "import com.sun.jna.Structure;\n");
  fprintf(f, "import com.sun.jna.Callback;\n");
  fprintf(f, "import java.util.Arrays;\n");
  fprintf(f, "import java.util.List;\n\n");

  fprintf(f, "public class %s {\n", class_name);

  fprintf(
      f,
      "    public static class NativeException extends RuntimeException {\n");
  fprintf(
      f,
      "        public NativeException(String message) { super(message); }\n");
  fprintf(f, "    }\n\n");

  fprintf(f, "    public static class CddFfiError extends Structure {\n");
  fprintf(f, "        public int code;\n");
  fprintf(f, "        public String message;\n");
  fprintf(f, "        public CddFfiError() { super(); }\n");
  fprintf(f, "        protected List<String> getFieldOrder() {\n");
  fprintf(f, "            return Arrays.asList(\"code\", \"message\");\n");
  fprintf(f, "        }\n");
  fprintf(f, "        public static class ByReference extends CddFfiError "
             "implements Structure.ByReference {}\n");
  fprintf(f, "    }\n\n");

  fprintf(f, "    private interface %sLib extends Library {\n", class_name);
  fprintf(
      f,
      "        %sLib INSTANCE = (%sLib) Native.load(\"%s\", %sLib.class);\n\n",
      class_name, class_name, lib_name, class_name);

  /* Emit Enums as static final ints */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];

    if (node->kind == CDD_FFI_NODE_MACRO) {
      if (node->evaluated_value) {
        /* LCOV_EXCL_START */
        if (node->inferred_type == CDD_FFI_MACRO_TYPE_STRING) {
          fprintf(f, "    public static final String %s = %s;\n\n", node->name,
                  node->evaluated_value);
        } else if (node->inferred_type == CDD_FFI_MACRO_TYPE_FLOAT) {
          fprintf(f, "    public static final double %s = %s;\n\n", node->name,
                  node->evaluated_value);
        } else {
          fprintf(f, "    public static final long %s = %s;\n\n", node->name,
                  node->evaluated_value);
        }
        /* LCOV_EXCL_STOP */
      }
    } else if (node->kind == CDD_FFI_NODE_ENUM) {
      if (node->doc)
        fprintf(f, "        /**\n         * %s\n         */\n", node->doc);
      fprintf(f, "        public interface %s {\n", node->name);
      for (j = 0; j < node->variants_count; j++) {
        cdd_ffi_enum_variant_t *var = &node->variants[j];
        if (var->doc)
          fprintf(f, "            /** %s */\n", var->doc);
        if (var->value) {
          fprintf(f, "            public static final int %s = %s;\n",
                  var->name, var->value);
        } else {
          fprintf(f,
                  "            public static final int %s = %" CDD_PRIz ";\n",
                  var->name, j);
        }
      }
      fprintf(f, "        }\n\n");
    }
  }

  /* Emit Interfaces for base classes */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      int has_derived = 0;
      size_t k, b;
      for (k = 0; k < ir->nodes_count; k++) {
        if (ir->nodes[k].kind == CDD_FFI_NODE_STRUCT) {
          for (b = 0; b < ir->nodes[k].base_classes_count; b++) {
            if (strcmp(ir->nodes[k].base_classes[b].name, node->name) == 0) {
              has_derived = 1;
              break;
            }
          }
        }
        if (has_derived)
          break;
      }
      if (has_derived || node->base_classes_count > 0) {
        fprintf(f, "        public interface I%s {\n", node->name);
        fprintf(f, "            Pointer getPointer();\n");
        fprintf(f, "        }\n\n");
      }
    }
  }

  /* Emit Structs */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_STRUCT) {
      if (node->doc)
        fprintf(f, "        /**\n         * %s\n         */\n", node->doc);

      fprintf(f, "        public static class %s extends Structure ",
              node->name);
      if (node->base_classes_count > 0) {
        int first_iface = 1;
        for (j = 0; j < node->base_classes_count; j++) {
          /* LCOV_EXCL_START */
          if (node->base_classes[j].is_virtual)
            continue; /* Diamond base instances are shared, omit duplicate
                         implements if language forces it */
          /* LCOV_EXCL_STOP */
          if (first_iface) {
            fprintf(f, "implements ");
            first_iface = 0;
          } else {
            fprintf(f, ", ");
          }
          fprintf(f, "I%s", node->base_classes[j].name);
        }
        fprintf(f, " {\n");
      } else {
        fprintf(f, "{\n");
      }

      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        const char *ftype = get_java_primitive(node->fields[j].type);
        fprintf(f, "            public %s %s;\n", ftype, fname);
      }

      fprintf(f, "\n            public %s() { super(); }\n", node->name);
      fprintf(f, "            public %s(Pointer peer) { super(peer); }\n",
              node->name);

      fprintf(f, "\n            protected List<String> getFieldOrder() {\n");
      fprintf(f, "                return Arrays.asList(");
      for (j = 0; j < node->fields_count; j++) {
        const char *fname =
            node->fields[j].name ? node->fields[j].name : "field";
        fprintf(f, "\"%s\"", fname);
        if (j < node->fields_count - 1)
          fprintf(f, ", ");
      }
      fprintf(f, ");\n            }\n");
      fprintf(
          f,
          "            public static class ByReference extends %s implements "
          "Structure.ByReference {}\n",
          node->name);
      fprintf(f,
              "            public static class ByValue extends %s implements "
              "Structure.ByValue {}\n",
              node->name);

      fprintf(f, "        }\n\n");
    }
  }

  /* Emit Functions */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      fprintf(f, "        %s %s(",
              get_java_primitive(node->return_or_base_type), node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        /* LCOV_EXCL_START */
        if (strcmp(arg_name, "interface") == 0)
          arg_name = "iface";
        /* LCOV_EXCL_STOP */

        fprintf(f, "%s %s", get_java_primitive(node->fields[j].type), arg_name);
        if (j < node->fields_count - 1 || node->is_variadic)
          fprintf(f, ", ");
      }
      if (node->is_variadic) {
        fprintf(f, "Object... args");
      } else {
        if (node->fields_count > 0)
          fprintf(f, ", ");
        fprintf(f, "CddFfiError.ByReference err");
      }
      fprintf(f, ");\n");
    }
  }
  fprintf(f, "    }\n\n");

  /* Emit Public Wrappers */
  for (i = 0; i < ir->nodes_count; i++) {
    cdd_ffi_ir_node_t *node = &ir->nodes[i];
    if (node->kind == CDD_FFI_NODE_FUNCTION) {
      if (node->doc)
        fprintf(f, "    /**\n     * %s\n     */\n", node->doc);
      fprintf(f, "    public static %s %s(",
              get_java_primitive(node->return_or_base_type), node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        /* LCOV_EXCL_START */
        if (strcmp(arg_name, "interface") == 0)
          arg_name = "iface";
        /* LCOV_EXCL_STOP */

        if (node->fields[j].intent == CDD_FFI_INTENT_OUT ||
            node->fields[j].intent == CDD_FFI_INTENT_INOUT) {
          /* Use JNA ByReference types for OUT parameters */
          cdd_ffi_type_t base_t = node->fields[j].type;
          if (base_t.pointer_depth > 0)
            base_t.pointer_depth--;
          if (base_t.kind == CDD_FFI_KIND_INT32)
            fprintf(f, "com.sun.jna.ptr.IntByReference %s", arg_name);
          /* LCOV_EXCL_START */
          else if (base_t.kind == CDD_FFI_KIND_INT64)
            fprintf(f, "com.sun.jna.ptr.LongByReference %s", arg_name);
          else if (base_t.kind == CDD_FFI_KIND_FLOAT32)
            fprintf(f, "com.sun.jna.ptr.FloatByReference %s", arg_name);
          else if (base_t.kind == CDD_FFI_KIND_FLOAT64)
            fprintf(f, "com.sun.jna.ptr.DoubleByReference %s", arg_name);
          /* LCOV_EXCL_STOP */
          else
            fprintf(f, "com.sun.jna.ptr.PointerByReference %s", arg_name);
        } else {
          fprintf(f, "%s %s", get_java_primitive(node->fields[j].type),
                  arg_name);
        }
        if (j < node->fields_count - 1 || node->is_variadic)
          fprintf(f, ", ");
      }
      if (node->is_variadic) {
        fprintf(f, "Object... args");
      }
      fprintf(f, ") {\n");
      if (!node->is_variadic) {
        fprintf(f, "        CddFfiError.ByReference err = new "
                   "CddFfiError.ByReference();\n");
      }

      fprintf(f, "        ");
      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "%s ret = ", get_java_primitive(node->return_or_base_type));
      }
      fprintf(f, "%sLib.INSTANCE.%s(", class_name, node->name);
      for (j = 0; j < node->fields_count; j++) {
        const char *arg_name =
            node->fields[j].name ? node->fields[j].name : "arg";
        if (strcmp(arg_name, "class") == 0)
          arg_name = "clazz";
        /* LCOV_EXCL_START */
        if (strcmp(arg_name, "interface") == 0)
          arg_name = "iface";
        /* LCOV_EXCL_STOP */
        fprintf(f, "%s", arg_name);
        if (j < node->fields_count - 1 || node->is_variadic)
          fprintf(f, ", ");
      }
      if (node->is_variadic) {
        fprintf(f, "args");
      } else {
        if (node->fields_count > 0)
          fprintf(f, ", ");
        fprintf(f, "err");
      }
      fprintf(f, ");\n");

      if (!node->is_variadic) {
        fprintf(f, "        if (err.code != 0) {\n");
        fprintf(f,
                "            throw new NativeException(err.message != null ? "
                "err.message : \"Unknown native error\");\n");
        fprintf(f, "        }\n");
      }

      if (node->return_or_base_type.kind != CDD_FFI_KIND_VOID) {
        fprintf(f, "        return ret;\n");
      }
      fprintf(f, "    }\n\n");
    }
  }

  fprintf(f, "}\n");
  fclose(f);
  return CDD_C_SUCCESS;
}

static enum cdd_c_error
emit_pom_xml(const cdd_generate_bindings_config_t *config) {
  char filepath[1024];
  FILE *f = NULL;

#if defined(_MSC_VER)
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s\\pom.xml", config->output_dir);
  if (fopen_s(&f, filepath, "w") != 0) {
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  CDD_SNPRINTF(filepath, sizeof(filepath), "%s/pom.xml", config->output_dir);
  f = fopen(filepath, "w");
  /* LCOV_EXCL_START */
  if (!f) {
    return CDD_C_ERROR_UNKNOWN;
  }
  /* LCOV_EXCL_STOP */
#endif

  fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(f, "<project xmlns=\"http://maven.apache.org/POM/4.0.0\"\n");
  fprintf(f,
          "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
  fprintf(f, "         xsi:schemaLocation=\"http://maven.apache.org/POM/4.0.0 "
             "http://maven.apache.org/xsd/maven-4.0.0.xsd\">\n");
  fprintf(f, "    <modelVersion>4.0.0</modelVersion>\n");
  fprintf(f, "    <groupId>com.cdd</groupId>\n");
  fprintf(f, "    <artifactId>%s</artifactId>\n",
          config->library_name ? config->library_name : "mylib");
  fprintf(f, "    <version>1.0-SNAPSHOT</version>\n");
  fprintf(f, "    <dependencies>\n");
  fprintf(f, "        <dependency>\n");
  fprintf(f, "            <groupId>net.java.dev.jna</groupId>\n");
  fprintf(f, "            <artifactId>jna</artifactId>\n");
  fprintf(f, "            <version>5.13.0</version>\n");
  fprintf(f, "        </dependency>\n");
  fprintf(f, "        <dependency>\n");
  fprintf(f, "            <groupId>junit</groupId>\n");
  fprintf(f, "            <artifactId>junit</artifactId>\n");
  fprintf(f, "            <version>4.13.2</version>\n");
  fprintf(f, "            <scope>test</scope>\n");
  fprintf(f, "        </dependency>\n");
  fprintf(f, "    </dependencies>\n");
  fprintf(f, "    <build>\n");
  fprintf(f, "        <plugins>\n");
  fprintf(f, "            <plugin>\n");
  fprintf(f, "                <groupId>org.apache.maven.plugins</groupId>\n");
  fprintf(f,
          "                <artifactId>maven-compiler-plugin</artifactId>\n");
  fprintf(f, "                <version>3.8.1</version>\n");
  fprintf(f, "                <configuration>\n");
  fprintf(f, "                    <source>8</source>\n");
  fprintf(f, "                    <target>8</target>\n");
  fprintf(f, "                </configuration>\n");
  fprintf(f, "            </plugin>\n");
  fprintf(f, "        </plugins>\n");
  fprintf(f, "    </build>\n");
  fprintf(f, "</project>\n");

  fclose(f);
  return CDD_C_SUCCESS;
}

enum cdd_c_error
cdd_ffi_emit_java(cdd_ffi_ir_t *ir,
                  const cdd_generate_bindings_config_t *config) {
  int rc;
  if (!ir || !config || !config->output_dir) {
    return CDD_C_ERROR_UNKNOWN;
  }

  rc = emit_java_file(ir, config);
  if (rc != 0)
    return rc;

  return emit_pom_xml(config);
}
