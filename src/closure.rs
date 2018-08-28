use jojo::*;

pub struct Closure {
    obj_dic: ObjDic,
    jojo: Ptr <JoVec>,
    local_scope: LocalScope,
}

impl Obj for Closure {
    fn tag (&self) -> Tag { CLOSURE_TAG }
    fn obj_dic (&self) -> ObjDic { self.obj_dic.clone () }

    fn apply (&self, env: &Env, arity: usize) {
        // [TODO]
    }
}
