/*
   Copyright (c) 2011, The Mineserver Project
   All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the The Mineserver Project nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
