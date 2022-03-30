#include "cdd.h"

namespace cdd {
namespace emit {

std::unique_ptr<clang::RecordDecl> struct_from_json_schema(/*json_schema*/) {
  auto record_decl = std::unique_ptr<clang::RecordDecl>();
  record_decl->setFreeStanding(true);
  record_decl->setAnonymousStructOrUnion(true);

  // Debug field content
  for (clang::RecordDecl::field_iterator jt = record_decl->field_begin();
       jt != record_decl->field_end(); ++jt) {
    llvm::outs() << jt->getType().getAsString() << " " << jt->getNameAsString()
                 << '\n';
  }
  return record_decl;
}

}; // namespace emit
}; // namespace cdd
