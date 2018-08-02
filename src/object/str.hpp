#pragma once

#include "../libjojo.hpp"

struct str_o: obj_t
{
    string str;
    str_o (env_t &env, string str);
    bool equal (env_t &env, shared_ptr <obj_t> obj);
    string repr (env_t &env);
};

str_o::str_o (env_t &env, string str);

string
str_o::repr (env_t &env);

bool
str_o::equal (env_t &env, shared_ptr <obj_t> obj);

bool
str_p (env_t &env, shared_ptr <obj_t> a);

void
test_str ();
