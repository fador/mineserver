#ifndef _PERMISSIONS_H
#define _PERMISSIONS_H


// Bitmask definitions
#define PERM_CUSTOM 1
#define PERM_GUEST  (1 << 1)
#define PERM_MEMBER (1 << 2)
#define PERM_OP     (1 << 3)
#define PERM_ADMIN  (1 << 4)

// Permission check macros
#define IS_CUSTOM(p) (p & PERM_GUEST)
#define IS_GUEST(p)  (p & PERM_GUEST)
#define IS_MEMBER(p) (p & PERM_MEMBER)
#define IS_OP(p)     (p & PERM_OP)
#define IS_ADMIN(p)  (p & PERM_ADMIN)

// Setter macros for built-in permissions
#define SET_GUEST(p)                            \
  p = (IS_CUSTOM(p) | PERM_MEMBER)

#define SET_MEMBER(p)                           \
  p = (IS_CUSTOM(p) | PERM_GUEST | PERM_MEMBER)

#define SET_OP(p)                                         \
  p = (IS_CUSTOM(p) | PERM_GUEST | PERM_MEMBER | PERM_OP)

#define SET_ADMIN(p)                                                    \
  p = (IS_CUSTOM(p) | PERM_GUEST | PERM_MEMBER | PERM_OP | PERM_ADMIN)


#endif /* _PERMISSIONS_H */
