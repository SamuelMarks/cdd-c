#include "cdd.h"

namespace cdd {
namespace emit {

std::unique_ptr<clang::RecordDecl> struct_from_json_schema(/*json_schema*/) {
  auto record_decl = std::unique_ptr<clang::RecordDecl>();
  /*clang::ASTContext C;
  clang::DeclContext
  clang::RecordDecl record = clang::RecordDecl::Create(const ASTContext &C,
  TagKind TK, DeclContext *DC, SourceLocation StartLoc, SourceLocation IdLoc,
                     IdentifierInfo *Id, RecordDecl* PrevDecl = nullptr)



  record_decl->setFreeStanding(true);
  record_decl->setAnonymousStructOrUnion(true);

  // clang::RecordDecl::Create();

  clang::IdentifierInfo *Name = nullptr;

  std::string label = "your_label";  // function scope.
      Name = &(*Context).Idents.get(label); // Use the context to get the
  identifier. clang::LabelDecl *labelDecl = clang::LabelDecl::Create((*Context),
  Context->getTranslationUnitDecl(),stmt->getBeginLoc(), Name);


  // Debug field content
  for (clang::RecordDecl::field_iterator jt = record_decl->field_begin();
       jt != record_decl->field_end(); ++jt) {
    llvm::outs() << jt->getType().getAsString() << " " << jt->getNameAsString()
                 << '\n';
  }*/
  return record_decl;
}

}; // namespace emit
}; // namespace cdd
