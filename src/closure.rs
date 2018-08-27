use jojo::*;

pub struct Closure {
    obj_map: ObjMap,
    name_stack: NameStack,
    jojo: Ptr <JoVec>,
    local_scope: LocalScope,
}

impl Obj for Closure {
    fn tag (&self) -> Tag { CLOSURE_TAG }

    fn obj_map (&self) -> ObjMap { self.obj_map.clone () }

    fn apply (&self, env: &Env, arity: usize) {
        // [TODO]
    }
}
