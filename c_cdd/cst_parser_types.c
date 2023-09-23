#include "cst_parser_types.h"

const char *CstNodeKind_to_str(const enum CstNodeKind kind) {
  switch (kind) {
  case BlockEnd:
    return "BlockEnd";
  case BlockStart:
    return "BlockStart";
  case Break:
    return "Break";
  case Case:
    return "Case";
  case Continue:
    return "Continue";
  case Declaration:
    return "Declaration";
  case Definition:
    return "Definition";
  case Do:
    return "Do";
  case Else:
    return "Else";
  case ElseIf:
    return "ElseIf";
  case Enum:
    return "Enum";
  case For:
    return "For";
  case Function:
    return "Function";
  case FunctionPrototype:
    return "FunctionPrototype";
  case GoTo:
    return "GoTo";
  case If:
    return "If";
  case Label:
    return "Label";
  case MacroDefine:
    return "MacroDefine";
  case MacroElif:
    return "MacroElif";
  case MacroElse:
    return "MacroElse";
  case MacroIf:
    return "MacroIf";
  case MacroIfDef:
    return "MacroIfDef";
  case MacroInclude:
    return "MacroInclude";
  case MacroPragma:
    return "MacroPragma";
  case Return:
    return "Return";
  case Struct:
    return "Struct";
  case Switch:
    return "Switch";
  case Union:
    return "Union";
  case While:
    return "While";
  case Expression:
  default:
    return "Expression";
  }
}

enum CstNodeKind str_to_CstNodeKind(const char *const s) {
  if (strcmp(s, "BlockEnd") == 0)
    return BlockEnd;
  else if (strcmp(s, "BlockStart") == 0)
    return BlockStart;
  else if (strcmp(s, "Break") == 0)
    return Break;
  else if (strcmp(s, "Case") == 0)
    return Case;
  else if (strcmp(s, "Continue") == 0)
    return Continue;
  else if (strcmp(s, "Declaration") == 0)
    return Declaration;
  else if (strcmp(s, "Definition") == 0)
    return Definition;
  else if (strcmp(s, "Do") == 0)
    return Do;
  else if (strcmp(s, "Else") == 0)
    return Else;
  else if (strcmp(s, "ElseIf") == 0)
    return ElseIf;
  else if (strcmp(s, "Enum") == 0)
    return Enum;
  else if (strcmp(s, "For") == 0)
    return For;
  else if (strcmp(s, "Function") == 0)
    return Function;
  else if (strcmp(s, "FunctionPrototype") == 0)
    return FunctionPrototype;
  else if (strcmp(s, "GoTo") == 0)
    return GoTo;
  else if (strcmp(s, "If") == 0)
    return If;
  else if (strcmp(s, "Label") == 0)
    return Label;
  else if (strcmp(s, "MacroDefine") == 0)
    return MacroDefine;
  else if (strcmp(s, "MacroElif") == 0)
    return MacroElif;
  else if (strcmp(s, "MacroElse") == 0)
    return MacroElse;
  else if (strcmp(s, "MacroIf") == 0)
    return MacroIf;
  else if (strcmp(s, "MacroIfDef") == 0)
    return MacroIfDef;
  else if (strcmp(s, "MacroInclude") == 0)
    return MacroInclude;
  else if (strcmp(s, "MacroPragma") == 0)
    return MacroPragma;
  else if (strcmp(s, "Return") == 0)
    return Return;
  else if (strcmp(s, "Struct") == 0)
    return Struct;
  else if (strcmp(s, "Switch") == 0)
    return Switch;
  else if (strcmp(s, "Union") == 0)
    return Union;
  else if (strcmp(s, "While") == 0)
    return While;
  else /* if(strcmp(s, "Expression") == 0) */
    return Expression;
}

void cst_node_arr_cleanup(struct cst_node_arr *cst_arr) {
  struct CstNode *cur, *next;
  cur = cst_arr == NULL ? NULL : cst_arr->elem;
  while (cur != NULL) {
    next = cur + 1;
    free(cur);
    cur = next;
  }
}
