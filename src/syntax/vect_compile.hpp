#pragma once

#include "../libjojo.hpp"

shared_ptr <jojo_t>
vect_compile (env_t &env,
              local_ref_map_t &local_ref_map,
              shared_ptr <vect_o> vect);
