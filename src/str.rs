use core::*;

impl Obj for String {
    fn tag (&self) -> Tag { STR_TAG }
    fn obj_dic (&self) -> ObjDic { ObjDic::new () }
}
