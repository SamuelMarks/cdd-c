#!/usr/bin/env python3
"""
AST-Based C Return Type Refactoring Tool
Author: Gemini CLI / Samuel Marks
Date: 2026-03-04

This script utilizes `tree-sitter` to parse a C codebase, identify functions 
returning non-standard types (e.g., structs, char*, enums), and enforces a 
strict `int` return type for error code bubbling. Original return values are 
converted into pointer-based out-parameters. Call sites are rewritten using 
comma operators and block-hoisted variables to maintain ISO C90 compliance.
"""

import tree_sitter_c as tsc
from tree_sitter import Language, Parser
import glob
import re
import os

LANGUAGE = Language(tsc.language())
parser = Parser(LANGUAGE)

def mask_msc_ver(src: bytes) -> bytes:
    """
    Masks preprocessor macros like `#if defined(_MSC_VER)` with spaces.
    This prevents tree-sitter from getting confused by un-evaluated branches 
    that break standard AST parsing, while preserving byte offsets.
    """
    text = src.decode('utf-8')
    out = []
    lines = text.split('
')
    in_msc = False
    in_else = False
    for line in lines:
        if re.match(r'^\s*#\s*if(def)?\s+(defined\(_MSC_VER\)|_WIN32|__WIN32__|__WINDOWS__)', line):
            in_msc = True
            in_else = False
            out.append(" " * len(line))
        elif in_msc and re.match(r'^\s*#\s*elif', line):
            in_else = True
            out.append(" " * len(line))
        elif in_msc and re.match(r'^\s*#\s*else\b', line):
            in_else = True
            out.append(" " * len(line))
        elif in_msc and re.match(r'^\s*#\s*endif\b', line):
            in_msc = False
            in_else = False
            out.append(" " * len(line))
        elif in_msc and not in_else:
            out.append(" " * len(line))
        else:
            out.append(line)
    return '
'.join(out).encode('utf-8')


def get_functions_to_refactor(base_dir: str) -> dict:
    """
    Pass 1: Collects all functions that do not return 'int' or 'void'.
    Returns a dict mapping func_name -> clean_return_type_string.
    """
    funcs_to_refactor = {}

    for filepath in glob.glob(f"{base_dir}/**/*.c", recursive=True) + glob.glob(f"{base_dir}/**/*.h", recursive=True):
        if "tests/" in filepath or "mocks/" in filepath or "cdd_test_helpers/" in filepath:
            continue
        with open(filepath, "rb") as f:
            src = f.read()
        
        src_cleaned = mask_msc_ver(src).replace(b"C_CDD_EXPORT", b"            ")
        tree = parser.parse(src_cleaned)
        
        def walk(n):
            if n.type == 'function_definition':
                decl = n.child_by_field_name('declarator')
                type_node = n.child_by_field_name('type')
                if decl and type_node:
                    curr = decl
                    while curr.type in ('pointer_declarator', 'function_declarator', 'parenthesized_declarator'):
                        if curr.type == 'function_declarator':
                            curr = curr.child_by_field_name('declarator')
                        elif curr.type == 'pointer_declarator':
                            curr = curr.child_by_field_name('declarator')
                        elif curr.type == 'parenthesized_declarator':
                            curr = curr.child(1)
                        else:
                            break
                    func_name = curr.text.decode('utf-8') if curr else ""
                    
                    ptr_str = ""
                    curr2 = decl
                    while curr2.type == 'pointer_declarator':
                        ptr_str += "*"
                        curr2 = curr2.child_by_field_name('declarator')
                    
                    if func_name:
                        base_rt = type_node.text.decode('utf-8')
                        full_rt = base_rt + (" " + ptr_str if ptr_str else "")
                        clean_rt = full_rt.replace('static', '').replace('inline', '').replace('extern', '').replace('C_CDD_EXPORT', '').strip()
                        clean_rt = re.sub(r'\s+', ' ', clean_rt)
                        
                        if clean_rt not in ('int', 'void', 'long', 'double', 'float', 'unsigned int', 'uint64_t', 'uint32_t', 'int32_t', 'int64_t', 'size_t', 'bool', 'char'):
                            funcs_to_refactor[func_name] = clean_rt
            for c in n.children:
                walk(c)
        walk(tree.root_node)

    return funcs_to_refactor


def process_file(filepath: str, funcs_to_refactor: dict):
    """
    Pass 2 & 3: Rewrite function definitions and update call sites.
    """
    with open(filepath, "rb") as f:
        src = bytearray(f.read())
    original_src = bytes(src)
    
    edits = []
    
    src_cleaned = mask_msc_ver(bytes(src)).replace(b"C_CDD_EXPORT", b"            ")
    tree = parser.parse(src_cleaned)
    
    def walk_defs(n):
        if n.type in ('function_definition', 'declaration'):
            decl = n.child_by_field_name('declarator')
            type_node = n.child_by_field_name('type')
            
            is_func = False
            if n.type == 'function_definition':
                is_func = True
            elif decl:
                curr = decl
                while curr and curr.type == 'pointer_declarator':
                    curr = curr.child_by_field_name('declarator')
                if curr and curr.type == 'function_declarator':
                    is_func = True
                    
            if is_func and type_node:
                curr = decl
                param_list = None
                while curr and curr.type in ('pointer_declarator', 'function_declarator', 'parenthesized_declarator'):
                    if curr.type == 'function_declarator':
                        param_list = curr.child_by_field_name('parameters')
                        curr = curr.child_by_field_name('declarator')
                    elif curr.type == 'pointer_declarator':
                        curr = curr.child_by_field_name('declarator')
                    elif curr.type == 'parenthesized_declarator':
                        curr = curr.child(1)
                    else:
                        break
                
                id_node = curr
                func_name = id_node.text.decode('utf-8') if id_node else ""
                
                if func_name in funcs_to_refactor:
                    prefix = bytes(src)[n.start_byte:id_node.start_byte].decode('utf-8')
                    keep = []
                    for w in ['static', 'inline', 'extern', 'C_CDD_EXPORT']:
                        if re.search(r'\b' + w + r'\b', prefix):
                            keep.append(w)
                    new_prefix = " ".join(keep + ["int "])
                    
                    edits.append((n.start_byte, id_node.start_byte, new_prefix.encode('utf-8')))
                    
                    if param_list:
                        out_type = funcs_to_refactor[func_name]
                        out_param = f"{out_type} *_out_val"
                        
                        params_text = bytes(src)[param_list.start_byte:param_list.end_byte].decode('utf-8')
                        inner_params = params_text[1:-1].strip()
                        if inner_params == '' or inner_params == 'void':
                            new_params = f"({out_param})"
                        else:
                            new_params = f"({inner_params}, {out_param})"
                            
                        edits.append((param_list.start_byte, param_list.end_byte, new_params.encode('utf-8')))
                        
                    if n.type == 'function_definition':
                        body = n.child_by_field_name('body')
                        if body:
                            ret_nodes = []
                            def find_returns(bn):
                                if bn.type == 'return_statement':
                                    ret_nodes.append(bn)
                                for bc in bn.children:
                                    find_returns(bc)
                            find_returns(body)
                            
                            for rnode in ret_nodes:
                                if rnode.child_count > 1:
                                    ret_expr_node = None
                                    for rc in rnode.children:
                                        if rc.type not in ('return', ';'):
                                            ret_expr_node = rc
                                            break
                                    if ret_expr_node:
                                        expr_text = bytes(src)[ret_expr_node.start_byte:ret_expr_node.end_byte].decode('utf-8')
                                        new_ret = f"{{ *_out_val = {expr_text}; return 0; }}"
                                        edits.append((rnode.start_byte, rnode.end_byte, new_ret.encode('utf-8')))
                                    else:
                                        edits.append((rnode.start_byte, rnode.end_byte, b"return 0;"))
                                        
        for c in n.children:
            walk_defs(c)
            
    walk_defs(tree.root_node)
    
    # Sort and apply signature and return edits in reverse order
    edits.sort(key=lambda x: x[0], reverse=True)
    for start, end, repl in edits:
        src[start:end] = repl
        
    def walk_and_find_calls(node, calls_list):
        if node.type == 'call_expression':
            func_node = node.child_by_field_name('function')
            if func_node and func_node.type == 'identifier':
                if func_node.text.decode('utf-8') in funcs_to_refactor:
                    p = node.parent
                    if p and p.type == 'comma_expression':
                        pass # Already transformed
                    else:
                        calls_list.append(node)
        for c in node.children:
            walk_and_find_calls(c, calls_list)

    var_dict = {}
    var_idx = 0
    
    # Iteratively replace innermost calls (to handle byte offsets shifting dynamically)
    while True:
        src_cleaned = mask_msc_ver(bytes(src)).replace(b"C_CDD_EXPORT", b"            ")
        tree = parser.parse(src_cleaned)
        calls = []
        walk_and_find_calls(tree.root_node, calls)
        if not calls:
            break
            
        call = calls[0]
        
        curr = call
        while curr and curr.type != 'function_definition':
            curr = curr.parent
            
        if not curr:
            # Reached global scope (e.g. macro or struct init)
            func_name = call.child_by_field_name('function').text.decode('utf-8')
            args_node = call.child_by_field_name('arguments')
            inner_args = bytes(src)[args_node.start_byte+1:args_node.end_byte-1].decode('utf-8').strip()
            
            var_name = "NULL" # Fallback
            if inner_args:
                new_call = f"({func_name}({inner_args}, &{var_name}), {var_name})"
            else:
                new_call = f"({func_name}(&{var_name}), {var_name})"
                
            src[call.start_byte:call.end_byte] = new_call.encode('utf-8')
            continue
            
        decl_node = curr.child_by_field_name('declarator')
        parent_func_name = decl_node.text.decode('utf-8') if decl_node else "unknown"
        if parent_func_name not in var_dict:
            var_dict[parent_func_name] = []
            
        fn_text = call.child_by_field_name('function').text.decode('utf-8')
        ret_type = funcs_to_refactor[fn_text]
        
        var_name = f"_ast_{fn_text}_{var_idx}".replace("c_cdd_str_", "").replace("c_cdd_", "")
        var_idx += 1
        
        var_dict[parent_func_name].append(f"{ret_type} {var_name};")
        
        args_node = call.child_by_field_name('arguments')
        inner_args = bytes(src)[args_node.start_byte+1:args_node.end_byte-1].decode('utf-8').strip()
        
        if inner_args:
            new_call = f"({fn_text}({inner_args}, &{var_name}), {var_name})"
        else:
            new_call = f"({fn_text}(&{var_name}), {var_name})"
            
        src[call.start_byte:call.end_byte] = new_call.encode('utf-8')
        
    # Final pass: Hoist out-parameter variable declarations to the top of blocks.
    src_cleaned = mask_msc_ver(bytes(src)).replace(b"C_CDD_EXPORT", b"            ")
    tree = parser.parse(src_cleaned)
    inserts = []
    
    def find_inserts(n):
        if n.type == 'function_definition':
            decl = n.child_by_field_name('declarator')
            fn = decl.text.decode('utf-8') if decl else "unknown"
            if fn in var_dict and var_dict[fn]:
                body = n.child_by_field_name('body')
                if body and body.type == 'compound_statement':
                    brace_byte = body.start_byte + 1
                    # Hoist immediately after opening brace
                    inserts.append((brace_byte, " " + " ".join(var_dict[fn]) + " "))
        for c in n.children:
            find_inserts(c)
            
    find_inserts(tree.root_node)
    inserts.sort(key=lambda x: x[0], reverse=True)
    
    for byte_idx, text in inserts:
        src[byte_idx:byte_idx] = text.encode('utf-8')
        
    if bytes(src) != original_src:
        with open(filepath, "wb") as f:
            f.write(src)


if __name__ == "__main__":
    print("Collecting functions to refactor...")
    funcs = get_functions_to_refactor("src")
    print(f"Identified {len(funcs)} target functions.")
    
    print("Refactoring codebase AST...")
    for filepath in glob.glob("src/**/*.c", recursive=True) + glob.glob("src/**/*.h", recursive=True):
        if "mocks" in filepath or "simple_json" in filepath:
            continue
        try:
            process_file(filepath, funcs)
        except Exception as e:
            print(f"Warning: Failed to process {filepath}: {e}")
            
    print("Done! Codebase successfully refactored.")
