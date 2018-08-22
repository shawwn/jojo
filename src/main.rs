    #![feature(nll)]
    use std::collections::HashMap;
    use std::rc::Rc;
  type Name = String;

  type ObjRecord = Vec <(Name, Rc <Obj>)>;
  type ObjRef = usize; // index in to ObjRecord

  type TypeRecord = Vec <(Name, Rc <Type>)>;
  type Tag = usize; // index in to TypeRecord
  type TagMap = HashMap <Tag, Name>;

  type JoVec = Vec <Rc <Jo>>;

  type ObjStack = Vec <Rc <Obj>>;

  type Bind = (Name, Rc <Obj>);
  type BindVec = Vec <Bind>; // index from end
  type LocalScope = Vec <BindVec>; // index from end

  type FrameStack = Vec <Box <Frame>>;
    trait Obj
    {
        fn tag (&self) -> Tag;
        fn obj_map (&self) -> HashMap <Name, Rc <Obj>>;
        fn repr (&self, env: &Env) -> String;
        fn print (&self, env: &Env);
        fn eq (&self, env: &Env, obj: Rc <Obj>) -> bool;
        fn apply (&self, env: &Env, arity: usize);
        fn apply_to_arg_dict (&self, env: &Env);
    }
    trait Jo
    {
        fn exe (&self, env: &mut Env, local_scope: Rc <LocalScope>);
        fn repr (&self, env: &Env) -> String;
    }
    struct Frame
    {
        index: usize,
        jojo: Rc <JoVec>,
        local_scope: Rc <LocalScope>,
    }
    struct Env
    {
        obj_record: ObjRecord,
        obj_stack: ObjStack,
        frame_stack: FrameStack,
        type_record: TypeRecord,
        tag_map: TagMap,
    }
    fn env_step (env: &mut Env)
    {
        if let Some(mut frame) = env.frame_stack.pop ()
        {
            let jo = frame.jojo [frame.index] .clone ();
            frame.index += 1;
            if frame.index < frame.jojo.len () {
                let local_scope = frame.local_scope.clone ();
                env.frame_stack.push (frame);
                jo.exe (env, local_scope);
            }
            else {
                jo.exe (env, frame.local_scope);
            }
        }
    }
    fn env_run (env: &mut Env)
    {
        while ! env.frame_stack.is_empty () {
            env_step (env);
        }
    }
    fn env_run_with_base (env: &mut Env, base: usize)
    {
        while env.frame_stack.len () > base {
            env_step (env);
        }
    }
    struct Type
    {

    }
    fn main ()
    {
        println! ("jojo!");
    }
