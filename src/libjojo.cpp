#include "libjojo.hpp"

template <typename Out>
void
string_split (const string &s, char delim, Out result)
{
    stringstream ss (s);
    string item;
    while (getline (ss, item, delim)) {
        *(result++) = item;
    }
}

vector <string>
string_split (const string &s, char delim)
{
    vector <string> elems;
    string_split (s, delim, back_inserter (elems));
    return elems;
}

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

size_t
number_of_obj_in_bind_vector (bind_vector_t &bind_vector)
{
    size_t sum = 0;
    auto begin = bind_vector.begin ();
    auto end = bind_vector.end ();
    for (auto it = begin; it != end; it++)
        if (it->second)
            sum++;
    return sum;
}

void
bind_vector_insert_obj (bind_vector_t &bind_vector,
                        shared_ptr <obj_t> obj)
{
    auto begin = bind_vector.rbegin ();
    auto end = bind_vector.rend ();
    for (auto it = begin; it != end; it++) {
        if (it->second == nullptr) {
            it->second = obj;
            return;
        }
    }
    cout << "- fatal error : bind_vector_insert_obj" << "\n"
         << "  the bind_vector is filled" << "\n";
    exit (1);
}

bind_vector_t
bind_vector_merge_obj_vector (bind_vector_t &old_bind_vector,
                              obj_vector_t &obj_vector)
{
    auto bind_vector = old_bind_vector;
    for (auto obj: obj_vector)
        bind_vector_insert_obj (bind_vector, obj);
    return bind_vector;
}

obj_vector_t
pick_up_obj_vector (env_t &env, size_t counter)
{
    auto obj_vector = obj_vector_t ();
    while (counter > 0) {
        counter--;
        auto obj = env.obj_stack.top ();
        obj_vector.push_back (obj);
        env.obj_stack.pop ();
    }
    reverse (obj_vector.begin (),
             obj_vector.end ());
    return obj_vector;
}

local_scope_t
local_scope_extend (local_scope_t old_local_scope,
                    bind_vector_t bind_vector)
{
    auto local_scope = old_local_scope;
    local_scope.push_back (bind_vector);
    return local_scope;
}

bool
bind_equal (env_t &env,
            bind_t &lhs,
            bind_t &rhs)
{
    if (lhs.first != rhs.first) return false;
    return lhs.second->equal (env, rhs.second);
}

bool
bind_vector_equal (env_t &env,
                   bind_vector_t &lhs,
                   bind_vector_t &rhs)
{
    if (lhs.size () != rhs.size ()) return false;
    auto size = lhs.size ();
    auto index = 0;
    while (index < size) {
        if (! bind_equal (env, lhs [index], rhs [index]))
            return false;
        index++;
    }
    return true;
}

bool
local_scope_equal (env_t &env,
                   local_scope_t &lhs,
                   local_scope_t &rhs)
{
    if (lhs.size () != rhs.size ()) return false;
    auto size = lhs.size ();
    auto index = 0;
    while (index < size) {
        if (! bind_vector_equal (env, lhs [index], rhs [index]))
            return false;
        index++;
    }
    return true;
}

bool
obj_map_equal (env_t &env, obj_map_t &lhs, obj_map_t &rhs)
{
    if (lhs.size () != rhs.size ()) return false;
    for (auto &kv: lhs) {
        auto name = kv.first;
        auto it = rhs.find (name);
        if (it == rhs.end ()) return false;
        if (! kv.second->equal (env, it->second)) return false;
    }
    return true;
}

string
obj_map_repr (env_t &env, obj_map_t &obj_map)
{
    string repr = "";
    for (auto &kv: obj_map) {
        auto name = kv.first;
        repr += name;
        repr += " = ";
        auto obj = kv.second;
        repr += obj->repr (env);
        repr += " ";
    }
    if (! repr.empty ()) repr.pop_back ();
    return repr;
}

name_vector_t
name_vector_obj_map_lack (name_vector_t &old_name_vector,
                          obj_map_t &obj_map)
{
    auto name_vector = name_vector_t ();
    for (auto name: old_name_vector) {
        auto it = obj_map.find (name);
        // not found == lack
        if (it == obj_map.end ())
            name_vector.push_back (name);
    }
    return name_vector;
}

name_vector_t
name_vector_obj_map_arity_lack (name_vector_t &old_name_vector,
                                obj_map_t &obj_map,
                                size_t arity)
{
    auto name_vector = name_vector_obj_map_lack
        (old_name_vector, obj_map);
    auto lack = name_vector.size ();
    auto counter = lack - arity;
    while (counter > 0) {
        counter--;
        name_vector.pop_back ();
    }
    return name_vector;
}

obj_map_t
pick_up_obj_map_and_merge (env_t &env,
                           name_vector_t &lack_name_vector,
                           obj_map_t &old_obj_map)
{
    auto obj_map = old_obj_map;
    auto begin = lack_name_vector.rbegin ();
    auto end = lack_name_vector.rend ();
    for (auto it = begin; it != end; it++) {
        name_t name = *it;
        auto obj = env.obj_stack.top ();
        env.obj_stack.pop ();
        obj_map [name] = obj;
    }
    return obj_map;
}

string
name_vector_and_obj_map_repr (env_t &env,
                              name_vector_t &name_vector,
                              obj_map_t &obj_map)
{
    string repr = "";
    for (auto &name: name_vector) {
        auto it = obj_map.find (name);
        if (it == obj_map.end ()) {
            repr += name;
            repr += " = _ ";
        }
    }
    for (auto &kv: obj_map) {
        auto name = kv.first;
        repr += name;
        repr += " = ";
        auto obj = kv.second;
        repr += obj->repr (env);
        repr += " ";
    }
    if (! repr.empty ()) repr.pop_back ();
    return repr;
}

template <class T>
T
vector_rev_ref (vector <T> vect, size_t rev_index)
{
    size_t size = vect.size ();
    size_t index = size - rev_index - 1;
    return vect [index];
}

bind_vector_t
bind_vector_from_name_vector (name_vector_t &name_vector)
{
    auto bind_vector = bind_vector_t ();
    auto begin = name_vector.begin ();
    auto end = name_vector.end ();
    for (auto it = begin; it != end; it++)
        bind_vector.push_back (make_pair (*it, nullptr));
    return bind_vector;
}

shared_ptr <frame_t>
new_frame_from_jojo (shared_ptr <jojo_t> jojo)
{
    return make_shared <frame_t>
        (jojo, local_scope_t ());
}

shared_ptr <frame_t>
new_frame_from_jo_vector (jo_vector_t jo_vector)
{
    auto jojo = make_shared <jojo_t> (jo_vector);
    return make_shared <frame_t>
        (jojo, local_scope_t ());
}

bool
obj_equal (env_t &env,
           shared_ptr <obj_t> &lhs,
           shared_ptr <obj_t> &rhs)
{
    return lhs->equal (env, rhs);
}

local_ref_map_t
local_ref_map_extend (env_t &env,
                      local_ref_map_t &old_local_ref_map,
                      name_vector_t &name_vector)
{
    auto local_ref_map = local_ref_map_t ();
    for (auto &kv: old_local_ref_map) {
        auto name = kv.first;
        auto old_local_ref = kv.second;
        auto local_ref = local_ref_t ();
        local_ref.first = old_local_ref.first + 1;
        local_ref.second = old_local_ref.second;
        local_ref_map.insert (make_pair (name, local_ref));
    }
    auto index = 0;
    auto size = name_vector.size ();
    while (index < size) {
        auto name = name_vector [index];
        auto local_ref = local_ref_t ();
        local_ref.first = 0;
        local_ref.second = index;
        local_ref_map.insert (make_pair (name, local_ref));
        index++;
    }
    return local_ref_map;
}

void
assert_pop_eq (env_t &env, shared_ptr <obj_t> obj)
{
    auto that = env.obj_stack.top ();
    assert (obj->equal (env, that));
    env.obj_stack.pop ();
}

void
assert_tos_eq (env_t &env, shared_ptr <obj_t> obj)
{
    auto that = env.obj_stack.top ();
    assert (obj->equal (env, that));
}

void
assert_stack_size (env_t &env, size_t size)
{
    assert (env.obj_stack.size () == size);
}
