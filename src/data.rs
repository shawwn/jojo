use core::*;

pub struct Data {
    tag: Tag,
    obj_dic: ObjDic,
}

impl Obj for Data {
    fn tag (&self) -> Tag { self.tag }
    fn obj_dic (&self) -> ObjDic { self.obj_dic.clone () }

    fn apply (&self, env: &mut Env, arity: usize) {
        let lack = self.obj_dic.lack ();
        if arity > lack {
            eprintln! ("- Data::apply");
            eprintln! ("  over-arity apply");
            eprintln! ("  arity > lack");
            eprintln! ("  arity : {}", arity);
            eprintln! ("  lack : {}", lack);
            panic! ("jojo fatal error!");
        }
        let tag = self.tag;
        let obj_dic = obj_dic_pick_up (env, &self.obj_dic, arity);
        env.obj_stack.push (Ptr::new (Data {
            tag,
            obj_dic,
        }));
    }
}
