use jojo::*;

pub type Num = f64;

impl Obj for Num {
    fn tag (&self) -> Tag { NUM_TAG }

    fn obj_map (&self) -> ObjMap {
        ObjMap::new ()
    }
}
