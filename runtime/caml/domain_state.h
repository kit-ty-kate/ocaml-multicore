/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*      KC Sivaramakrishnan, Indian Institute of Technology, Madras       */
/*                Stephen Dolan, University of Cambridge                  */
/*                                                                        */
/*   Copyright 2019 Indian Institute of Technology, Madras                */
/*   Copyright 2019 University of Cambridge                               */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#ifndef CAML_STATE_H
#define CAML_STATE_H

#ifdef __APPLE__
#include <pthread.h>
#endif
#include <stddef.h>
#include <stdio.h>

#include "misc.h"

/* This structure sits in the TLS area and is also accessed efficiently
 * via native code, which is why the indices are important */
typedef struct {
#define DOMAIN_STATE(type, name) CAMLalign(8) type name;
#include "domain_state.tbl"
#ifndef NATIVE_CODE
  /* Bytecode TLS vars, not used for native code */
  #define BYTE_DOMAIN_STATE(type, name) type name;
  #include "byte_domain_state.tbl"
  #undef BYTE_DOMAIN_STATE
#endif
#undef DOMAIN_STATE
  CAMLalign(8) char end_of_domain_state;
} caml_domain_state;

enum {
  Domain_state_num_fields =
#define DOMAIN_STATE(type, name) + 1
#include "domain_state.tbl"
#undef DOMAIN_STATE

#ifndef NATIVE_CODE
  ,
  Byte_domain_state_num_fields =
#define BYTE_DOMAIN_STATE(type, name) + 1
#include "byte_domain_state.tbl"
#undef BYTE_DOMAIN_STATE
#endif
};

/* Check that the structure was laid out without padding,
   since the runtime assumes this in computing offsets */
#ifdef NATIVE_CODE
CAML_STATIC_ASSERT(
    offsetof(caml_domain_state, end_of_domain_state) ==
    Domain_state_num_fields * 8);
#else
CAML_STATIC_ASSERT(
    offsetof(caml_domain_state, end_of_domain_state) ==
    (Domain_state_num_fields + Byte_domain_state_num_fields) * 8);
#endif

#ifdef __APPLE__
  CAMLextern pthread_key_t caml_domain_state_key;
  CAMLextern void caml_init_domain_state_key(void);
  #define CAML_INIT_DOMAIN_STATE caml_init_domain_state_key()
  #define Caml_state \
      ((caml_domain_state*) pthread_getspecific(caml_domain_state_key))
  #define SET_Caml_state(x) \
      (pthread_setspecific(caml_domain_state_key, x))
#else
  CAMLextern __thread caml_domain_state* Caml_state;
  #define CAML_INIT_DOMAIN_STATE
  #define SET_Caml_state(x) (Caml_state = (x))
#endif

#define Caml_state_field(field) (Caml_state->field)

#endif /* CAML_STATE_H */
