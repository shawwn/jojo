#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cassert>
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <stack>

using namespace std;

struct env_t;
struct obj_t;
struct jo_t;
struct jojo_t;
struct box_t;
struct frame_t;

using name_t = string;
using name_vector_t = vector <name_t>;
using bind_t = pair <name_t, shared_ptr <obj_t>>;
using bind_vector_t = vector <bind_t>; // index from end
using local_scope_t = vector <bind_vector_t>; // index from end
using jo_vector_t = vector <jo_t *>;
using tag_t = string;
using obj_map_t = map <name_t, shared_ptr <obj_t>>;
using obj_vector_t = vector <shared_ptr <obj_t>>;
using box_map_t = map <name_t, box_t *>;
using obj_stack_t = stack <shared_ptr <obj_t>>;
using frame_stack_t = stack <shared_ptr <frame_t>>;

#include "env.hpp"
#include "obj.hpp"
#include "jo.hpp"

string
jojo_repr (env_t &env, shared_ptr <jojo_t> jojo);

string
name_vector_repr (name_vector_t &name_vector);

string
bind_vector_repr (env_t &env, bind_vector_t bind_vector);

string
local_scope_repr (env_t &env, local_scope_t local_scope);
