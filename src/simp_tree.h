#ifndef SIMP_TREE_H
#define SIMP_TREE_H

#define DEFINE_SIMP_TREE(type, name) \
    typedef struct \
    { \
        type data; \
        struct name* parent; \
        struct name* child; \
    } name;

#endif // SIMP_TREE_H

