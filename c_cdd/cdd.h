#ifndef CDD_H
#define CDD_H

#include <clang/AST/Decl.h>
#include <clang/AST/DeclarationName.h>

#include "c_cdd_export.h"

namespace cdd {
namespace emit {

std::unique_ptr<clang::RecordDecl> struct_from_json_schema(/*json_schema*/);

}; // namespace emit
}; // namespace cdd

#endif /* !CDD_H */
