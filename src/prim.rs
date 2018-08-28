use core::*;

type PrimFn = fn (env: &mut Env, obj_dic: &ObjDic);

pub struct Prim {
    obj_dic: ObjDic,
    fun: PrimFn,
}

impl Obj for Prim {
    fn tag (&self) -> Tag { PRIM_TAG }
    fn obj_dic (&self) -> ObjDic { self.obj_dic.clone () }

    fn apply (&self, env: &mut Env, arity: usize) {
        let lack = self.obj_dic.lack ();
        if arity > lack {
            eprintln! ("- Prim::apply");
            eprintln! ("  over-arity apply");
            eprintln! ("  arity > lack");
            eprintln! ("  arity : {}", arity);
            eprintln! ("  lack : {}", lack);
            panic! ("jojo fatal error!");
        }
        let fun = self.fun;
        let obj_dic = obj_dic_pick_up (env, &self.obj_dic, arity);
        if arity == lack {
            fun (env, &obj_dic);
        } else {
            env.obj_stack.push (Ptr::new (Prim {
                obj_dic,
                fun,
            }));
        }
    }
}
