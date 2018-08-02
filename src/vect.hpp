#pragma once

#include "jojo.hpp"

struct vect_o: obj_t
{
    obj_vector_t obj_vector;
    vect_o (env_t &env, obj_vector_t obj_vector);
    bool equal (env_t &env, shared_ptr <obj_t> obj);
    string repr (env_t &env);
};

struct collect_vect_jo_t: jo_t
{
    size_t counter;
    collect_vect_jo_t (size_t counter);
    jo_t * copy ();
    void exe (env_t &env, local_scope_t &local_scope);
    string repr (env_t &env);
};

bool
vect_equal (env_t &env,
            obj_vector_t &lhs,
            obj_vector_t &rhs);

bool
vect_p (env_t &env, shared_ptr <obj_t> a);

shared_ptr <vect_o>
list_to_vect (env_t &env, shared_ptr <obj_t> l);

shared_ptr <obj_t>
vect_to_list (env_t &env, shared_ptr <vect_o> vect);

void import_vect (env_t &env);
void test_vect ();
