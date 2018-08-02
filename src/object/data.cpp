#include "data.hpp"

data_o::
data_o (env_t &env,
        tag_t tag,
        obj_map_t obj_map)
{
    this->tag = tag;
    this->obj_map = obj_map;
}
