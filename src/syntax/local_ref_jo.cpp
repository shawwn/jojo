#include "local_ref_jo.hpp"

local_ref_jo_t::
local_ref_jo_t (size_t level, size_t index)
{
    this->level = level;
    this->index = index;
}

jo_t *
local_ref_jo_t::copy ()
{
    return new local_ref_jo_t (this->level, this->index);
}

void
local_ref_jo_t::exe (env_t &env, local_scope_t &local_scope)
{
    // this is the only place where
    //   the local_scope in the arg of exe is uesd.
    auto bind_vector =
        vector_rev_ref (local_scope, this->level);
    auto bind =
        vector_rev_ref (bind_vector, this->index);
    auto obj = bind.second;
    env.obj_stack.push (obj);
}

string
local_ref_jo_t::repr (env_t &env)
{
    return "local." +
        to_string (this->level) + "." +
        to_string (this->index);
}
