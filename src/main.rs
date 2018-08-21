    use std::collections::HashMap;
    use std::rc::Rc;
    type Name = String;

    type ObjRecord = Vec <(Name, Rc <Obj>)>;
    type ObjRef = usize; // index in to ObjRecord

    type TypeRecord = Vec <(Name, Rc <Type>)>;
    type Tag = usize; // index in to TypeRecord
    type TagMap = HashMap <Tag, Name>;

    type JoVec = Vec <Box <Jo>>;

    type ObjStack = Vec <Rc <Obj>>;

    type Bind = (Name, Rc <Obj>);
    type BindVec = Vec <Bind>; // index from end
    type LocalScope = Vec <BindVec>; // index from end

    type FrameStack = Vec <Rc <Frame>>;
    trait Obj {
        fn tag (&self) -> Tag;
        fn obj_map (&self) -> HashMap <Name, Rc <Obj>>;
        fn repr (&self, env: &Env) -> String;
        fn print (&self, env: &Env);
        fn eq (&self, env: &Env, obj: Rc <Obj>) -> bool;
        fn apply (&self, env: &Env, arity: usize);
        fn apply_to_arg_dict (&self, env: &Env);
    }
    trait Jo {
        fn exe (&self, env: &Env, local_scope: &LocalScope);
        fn repr (&self, env: &Env) -> String;
    }
    struct Env {
       obj_record: ObjRecord,
       obj_stack: ObjStack,
       frame_stack: FrameStack,
       type_record: TypeRecord,
       tag_map: TagMap,
    }
    struct Frame {
        index: usize,
        jojo: Rc <JoVec>,
        local_scope: LocalScope,
    }
  // fn step ();
  // fn run ();
  // fn box_map_report ();
  // fn frame_stack_report ();
  // fn obj_stack_report ();
  // fn report ();
  // fn run_with_base (size_t base);
  // fn double_report ();
  // fn step_and_report ();
    struct Type {

    }
    fn main () {
        println! ("jojo!");
    }
