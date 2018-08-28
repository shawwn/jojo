  // use std::collections::HashMap;
  use std::rc::Rc;
  use dic::Dic;
  pub type Ptr <T> = Rc <T>;

  pub type Name = String;

  pub type Id = usize; // index in to ObjDic
  pub type ObjDic = Dic <Ptr <Obj>>;

  pub type Tag = usize; // index in to TypeDic
  pub type TypeDic = Dic <Ptr <Type>>;

  pub type ObjStack = Vec <Ptr <Obj>>;
  pub type FrameStack = Vec <Box <Frame>>;

  pub type LocalScope = Vec <ObjDic>; // index from end

  pub type TagVec = Vec <Tag>;
  pub type JoVec = Vec <Ptr <Jo>>;
  pub struct Env {
      pub obj_dic: ObjDic,
      pub type_dic: TypeDic,
      pub obj_stack: ObjStack,
      pub frame_stack: FrameStack,
  }

  pub fn new_env () -> Env {
      let mut env = Env {
          obj_dic: ObjDic::new (),
          type_dic: TypeDic::new (),
          obj_stack: ObjStack::new (),
          frame_stack: FrameStack::new (),
      };
      init_type_dic (&mut env);
      env
  }

  pub fn env_step (env: &mut Env) {
      if let Some (mut frame) = env.frame_stack.pop () {
          let jo = frame.jojo [frame.index] .clone ();
          frame.index += 1;
          if frame.index < frame.jojo.len () {
              let local_scope = frame.local_scope.clone ();
              env.frame_stack.push (frame);
              jo.exe (env, local_scope);
          } else {
              jo.exe (env, frame.local_scope);
          }
      }
  }

  pub fn env_run (env: &mut Env) {
      while ! env.frame_stack.is_empty () {
          env_step (env);
      }
  }

  pub fn env_run_with_base (env: &mut Env, base: usize) {
      while env.frame_stack.len () > base {
          env_step (env);
      }
  }
  pub trait Obj {
      fn tag (&self) -> Tag;
      fn obj_dic (&self) -> ObjDic;

      fn get (&self, name: &Name) -> Option <Ptr <Obj>> {
          match self.obj_dic () .get (name) {
              Some (obj) => Some (obj.clone ()),
              None => None,
          }
      }

      fn repr (&self, env: &Env) -> String {
          format! ("#<{}>", name_of_tag (&env, self.tag ()))
      }

      fn print (&self, env: &Env) {
          println! ("{}", self.repr (&env));
      }

      fn eq (&self, _env: &Env, _obj: Ptr <Obj>) -> bool {
          false
      }

      fn apply (&self, env: &Env, _arity: usize) {
          eprintln! ("- Obj::apply");
          eprintln! ("  applying non applicable object");
          eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
          eprintln! ("  obj : {}", self.repr (env));
          panic! ("jojo fatal error!");
      }

      fn apply_to_arg_dict (&self, env: &Env) {
          eprintln! ("- Obj::apply_to_arg_dict");
          eprintln! ("  applying non applicable object");
          eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
          eprintln! ("  obj : {}", self.repr (&env));
          panic! ("jojo fatal error!");
      }
  }

  pub fn define (
      env: &mut Env,
      name: &Name,
      obj: Ptr <Obj>,
  ) -> Id {
      env.obj_dic.ins (name, Some (obj.clone ()))
  }
  pub struct Frame {
      index: usize,
      jojo: Ptr <JoVec>,
      local_scope: Ptr <LocalScope>,
  }
  pub trait Jo {
      fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>);
      fn repr (&self, env: &Env) -> String;
  }
  pub struct Type {
      obj_dic: ObjDic,
      tag_of_type: Tag,
      super_tag_vec: TagVec,
  }

  impl Obj for Type {
      fn tag (&self) -> Tag { TYPE_TAG }
      fn obj_dic (&self) -> ObjDic { self.obj_dic.clone () }
  }

  pub fn define_type (
      env: &mut Env,
      name: &Name,
      typ: Ptr <Type>,
  ) -> Tag {
      env.type_dic.ins (name, Some (typ))
  }
  pub fn name_of_tag (env: &Env, tag: Tag) -> Name {
      if tag >= env.type_dic.len () {
          format! ("#<unknown-tag:{}>", tag.to_string ())
      } else {
          let entry = env.type_dic.idx (tag);
          entry.name.clone ()
      }
  }

  fn preserve_tag (env: &mut Env, tag: Tag, name: &str) {
      let typ = Ptr::new (Type {
          obj_dic: ObjDic::new (),
          tag_of_type: tag,
          super_tag_vec: TagVec::new (),
      });
      let index = env.type_dic.ins (name, Some (typ));
      assert_eq! (tag, index);
  }

  pub const CLOSURE_TAG      : Tag = 0;
  pub const TYPE_TAG         : Tag = 1;
  pub const TRUE_TAG         : Tag = 2;
  pub const FALSE_TAG        : Tag = 3;
  pub const PRIM_TAG         : Tag = 4;
  pub const NUM_TAG          : Tag = 5;
  pub const STR_TAG          : Tag = 6;
  pub const SYM_TAG          : Tag = 7;
  pub const NULL_TAG         : Tag = 8;
  pub const CONS_TAG         : Tag = 9;
  pub const VECT_TAG         : Tag = 10;
  pub const DICT_TAG         : Tag = 11;
  pub const MODULE_TAG       : Tag = 12;
  pub const KEYWORD_TAG      : Tag = 13;
  pub const MACRO_TAG        : Tag = 14;
  pub const TOP_KEYWORD_TAG  : Tag = 15;
  pub const NOTHING_TAG      : Tag = 16;
  pub const JUST_TAG         : Tag = 17;

  fn init_type_dic (env: &mut Env) {
      preserve_tag (env, CLOSURE_TAG      , "closure-t");
      preserve_tag (env, TYPE_TAG         , "type-t");
      preserve_tag (env, TRUE_TAG         , "true-t");
      preserve_tag (env, FALSE_TAG        , "false-t");
      preserve_tag (env, PRIM_TAG         , "prim-t");
      preserve_tag (env, NUM_TAG          , "num-t");
      preserve_tag (env, STR_TAG          , "str-t");
      preserve_tag (env, SYM_TAG          , "sym-t");
      preserve_tag (env, NULL_TAG         , "null-t");
      preserve_tag (env, CONS_TAG         , "cons-t");
      preserve_tag (env, VECT_TAG         , "vect-t");
      preserve_tag (env, DICT_TAG         , "dict-t");
      preserve_tag (env, MODULE_TAG       , "module-t");
      preserve_tag (env, KEYWORD_TAG      , "keyword-t");
      preserve_tag (env, MACRO_TAG        , "macro-t");
      preserve_tag (env, TOP_KEYWORD_TAG  , "top-keyword-t");
      preserve_tag (env, NOTHING_TAG      , "nothing-t");
      preserve_tag (env, JUST_TAG         , "just-t");
  }
  pub struct Data {
      tag: Tag,
      obj_dic: ObjDic,
  }

  impl Obj for Data {
      fn tag (&self) -> Tag { self.tag }
      fn obj_dic (&self) -> ObjDic { self.obj_dic.clone () }

      fn apply (&self, env: &Env, arity: usize) {

      }
  }
