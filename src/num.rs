use core::*;

pub type Num = f64;

impl Obj for Num {
    fn tag (&self) -> Tag { NUM_TAG }
    fn obj_dic (&self) -> ObjDic { ObjDic::new () }
}
