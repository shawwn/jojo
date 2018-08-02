#include "libjojo.hpp"

string
jojo_repr (env_t &env, shared_ptr <jojo_t> jojo)
{
    assert (jojo->jo_vector.size () != 0);
    string repr = "";
    for (auto &jo: jojo->jo_vector) {
        repr += jo->repr (env);
        repr += " ";
    }
    repr.pop_back ();
    return repr;
}

string
name_vector_repr (name_vector_t &name_vector)
{
    if (name_vector.size () == 0) {
        string repr = "[";
        repr += "]";
        return repr;
    }
    else {
        string repr = "[";
        for (auto name: name_vector) {
            repr += name;
            repr += " ";
        }
        if (! repr.empty ()) repr.pop_back ();
        repr += "]";
        return repr;
    }
}

string
bind_vector_repr (env_t &env, bind_vector_t bind_vector)
{
    string repr = "";
    for (auto it = bind_vector.rbegin ();
         it != bind_vector.rend ();
         it++) {
        repr += "(";
        repr += to_string (distance (bind_vector.rbegin (), it));
        repr += " ";
        repr += it->first;
        repr += " = ";
        auto obj = it->second;
        if (obj == nullptr)
            repr += "_";
        else
            repr += obj->repr (env);
        repr += ") ";
    }
    return repr;
}

string
local_scope_repr (env_t &env, local_scope_t local_scope)
{
    string repr = "";
    repr += "  - [";
    repr += to_string (local_scope.size ());
    repr += "] ";
    repr += "local_scope - ";
    repr += "\n";
    for (auto it = local_scope.rbegin ();
         it != local_scope.rend ();
         it++) {
        repr += "    ";
        repr += to_string (distance (local_scope.rbegin (), it));
        repr += " ";
        repr += bind_vector_repr (env, *it);
        repr += "\n";
    }
    return repr;
}
