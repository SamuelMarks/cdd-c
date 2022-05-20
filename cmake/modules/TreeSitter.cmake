# - Try to find tree-sitter
# Once done, this will define
#
#  TreeSitter_FOUND        - system has tree-sitter
#  TreeSitter_INCLUDE_DIRS - the tree-sitter include directories
#  TreeSitter_LIBRARIES    - link these to use tree-sitter

set(TreeSitter_INCLUDE_DIRS "${CMAKE_HOME_DIRECTORY}/prepackaged/tree_sitter")
set(TreeSitter_LIBRARIES "${CMAKE_HOME_DIRECTORY}/prepackaged/libtree-sitter-c.a")
if (EXISTS "${TreeSitter_INCLUDE_DIRS}" AND EXISTS "${TreeSitter_LIBRARIES}")
    set(TreeSitter_FOUND 1)
endif (EXISTS "${TreeSitter_INCLUDE_DIRS}" AND EXISTS "${TreeSitter_LIBRARIES}")
