use core::*;

pub struct Closure {
    obj_dic: ObjDic,
    jojo: Ptr <JoVec>,
    local_scope: Ptr <LocalScope>,
}

impl Obj for Closure {
    fn tag (&self) -> Tag { CLOSURE_TAG }
    fn obj_dic (&self) -> ObjDic { self.obj_dic.clone () }

    fn apply (&self, env: &mut Env, arity: usize) {
        let lack = self.obj_dic.lack ();
        if arity > lack {
            eprintln! ("- Closure::apply");
            eprintln! ("  over-arity apply");
            eprintln! ("  arity > lack");
            eprintln! ("  arity : {}", arity);
            eprintln! ("  lack : {}", lack);
            panic! ("jojo fatal error!");
        }
        let jojo = self.jojo.clone ();
        let obj_dic = obj_dic_pick_up (env, &self.obj_dic, arity);
        let local_scope = self.local_scope.clone ();
        if arity == lack {
            env.frame_stack.push (Box::new (Frame {
                index: 0,
                jojo,
                local_scope: local_scope_extend (
                    local_scope, obj_dic),
            }));
        } else {
            env.obj_stack.push (Ptr::new (Closure {
                obj_dic,
                jojo,
                local_scope,
            }));
        }
    }
}
