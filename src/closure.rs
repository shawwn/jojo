use jojo::*;

pub struct Closure {
    obj_map: ObjMap,
    name_vector: NameVec,
    jojo: Ptr <JoVec>,
    bind_vector: BindVec,
    local_scope: LocalScope,
}

impl Obj for Closure {
    fn tag (&self) -> Tag { CLOSURE_TAG }

    fn obj_map (&self) -> ObjMap {
        self.obj_map.clone ()
    }
}
