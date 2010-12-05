#ifndef _PERMISSIONS_H
#define _PERMISSIONS_H


// Bitmask definitions
#define PERM_CUSTOM 1
#define PERM_GUEST  (1 << 2)
#define PERM_MEMBER (1 << 3)
#define PERM_OP     (1 << 4)
#define PERM_ADMIN  (1 << 5)

// Permission check macros
#define IS_CUSTOM(u) (u->permissions & PERM_GUEST)
#define IS_GUEST(u)  (u->permissions & PERM_GUEST)
#define IS_MEMBER(u) (u->permissions & PERM_MEMBER)
#define IS_OP(u)     (u->permissions & PERM_OP)
#define IS_ADMIN(u)  (u->permissions & PERM_ADMIN)


// Setter macros for built-in permissions
#define SET_GUEST(u)                            \
  u->permissions = (IS_CUSTOM(u) | PERM_MEMBER)

#define SET_MEMBER(u)                           \
  u->permissions = (IS_CUSTOM(u) | PERM_MEMBER)

#define SET_OP(u)                                         \
  u->permissions = (IS_CUSTOM(u) | PERM_MEMBER | PERM_OP)

#define SET_ADMIN(u)                                                    \
  u->permissions = (IS_CUSTOM(u) | PERM_MEMBER | PERM_OP | PERM_ADMIN)


#endif /* _PERMISSIONS_H */
