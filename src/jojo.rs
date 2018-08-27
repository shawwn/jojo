  use std::collections::HashMap;
  use std::rc::Rc;
  pub type Ptr <T> = Rc <T>;

  pub type Name = String;

  pub type ObjMap = HashMap <Name, Ptr <Obj>>;

  pub struct ObjEntry { name: Name, obj: Ptr <Obj> }
  pub type ObjRecord = Vec <Option <ObjEntry>>;
  pub type ObjRef = usize; // index in to ObjRecord

  pub struct TypeEntry { name: Name, t: Ptr <Type> }
  pub type TypeRecord = Vec <Option <TypeEntry>>;
  pub type Tag = usize; // index in to TypeRecord

  pub type TagVec = Vec <Tag>;

  pub type JoVec = Vec <Ptr <Jo>>;

  pub type ObjStack = Vec <Ptr <Obj>>;

  pub type FrameStack = Vec <Box <Frame>>;

  pub type Bind = (Name, Ptr <Obj>);
  pub type BindVec = Vec <Bind>; // index from end
  pub type LocalScope = Vec <BindVec>; // index from end

  pub type NameVec = Vec <Name>;

  pub type StringVec = Vec <String>;
  pub struct Env {
      pub obj_record: ObjRecord,
      pub type_record: TypeRecord,
      pub obj_stack: ObjStack,
      pub frame_stack: FrameStack,
  }

  pub fn new_env () -> Env {
      let mut env = Env {
          obj_record: ObjRecord::new (),
          type_record: make_type_record (),
          obj_stack: ObjStack::new (),
          frame_stack: FrameStack::new (),
      };
      init_type_record (&mut env);
      env
  }

  fn make_type_record () -> TypeRecord {
      let mut type_record = TypeRecord::new ();
      for _ in 0..64 {
          type_record.push (None);
      }
      type_record
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

      fn obj_map (&self) -> ObjMap;

      fn get (&self, name: &Name) -> Option <Ptr <Obj>> {
          match self.obj_map () .get (name) {
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

      fn eq (&self, env: &Env, obj: Ptr <Obj>) -> bool {
          false
      }

      fn apply (&self, env: &Env, arity: usize) {
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

  pub fn define (env: &mut Env, name: &Name, obj: Ptr <Obj>) -> ObjRef {
      let obj_ref = env.obj_record.len ();
      let obj_entry = ObjEntry {
          name: name.clone (),
          obj: obj.clone (),
      };
      env.obj_record.push (Some (obj_entry));
      return obj_ref;
  }
  pub trait Jo {
      fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>);
      fn repr (&self, env: &Env) -> String;
  }
  pub struct Frame {
      index: usize,
      jojo: Ptr <JoVec>,
      local_scope: Ptr <LocalScope>,
  }
  pub struct Type {
      obj_map: ObjMap,
      tag_of_type: Tag,
      super_tag_vector: TagVec,
  }

  impl Obj for Type {
      fn tag (&self) -> Tag { TYPE_TAG }

      fn obj_map (&self) -> ObjMap { self.obj_map.clone () }
  }

  pub fn define_type (env: &mut Env, name: &Name, t: Ptr <Type>) -> Tag {
      let tag = env.type_record.len ();
      let type_entry = TypeEntry {
          name: name.clone (),
          t: t.clone (),
      };
      env.type_record.push (Some (type_entry));
      return tag;
  }
  pub fn name_of_tag (env: &Env, tag: Tag) -> Name {
      if tag >= env.type_record.len () {
          format! ("#<unknown-tag:{}>", tag.to_string ())
      } else {
          if let Some (type_entry) = &env.type_record [tag] {
              type_entry.name.clone ()
          } else {
              format! ("#<unknown-tag:{}>", tag.to_string ())
          }
      }
  }

  fn preserve_tag (env: &mut Env, tag: Tag, name_str: &str) {
      let t = Ptr::new (Type {
          obj_map: ObjMap::new (),
          tag_of_type: tag,
          super_tag_vector: TagVec::new (),
      });
      let type_entry = TypeEntry {
          name: Name::from (name_str),
          t,
      };
      env.type_record [tag] = Some (type_entry);
  }

  pub const CLOSURE_TAG      : Tag = 0;
  pub const TYPE_TAG         : Tag = 1;
  pub const TRUE_TAG         : Tag = 2;
  pub const FALSE_TAG        : Tag = 3;
  pub const PRIM_TAG         : Tag = 6;
  pub const NUM_TAG          : Tag = 7;
  pub const STR_TAG          : Tag = 8;
  pub const NULL_TAG         : Tag = 9;
  pub const CONS_TAG         : Tag = 10;
  pub const VECT_TAG         : Tag = 12;
  pub const DICT_TAG         : Tag = 13;
  pub const MODULE_TAG       : Tag = 14;
  pub const KEYWORD_TAG      : Tag = 15;
  pub const MACRO_TAG        : Tag = 16;
  pub const TOP_KEYWORD_TAG  : Tag = 17;
  pub const SYM_TAG          : Tag = 18;
  pub const NOTHING_TAG      : Tag = 19;
  pub const JUST_TAG         : Tag = 20;

  fn init_type_record (env: &mut Env) {
      preserve_tag (env, CLOSURE_TAG      , "closure-t");
      preserve_tag (env, TYPE_TAG         , "type-t");
      preserve_tag (env, TRUE_TAG         , "true-t");
      preserve_tag (env, FALSE_TAG        , "false-t");
      preserve_tag (env, PRIM_TAG         , "prim-t");
      preserve_tag (env, NUM_TAG          , "num-t");
      preserve_tag (env, STR_TAG          , "str-t");
      preserve_tag (env, NULL_TAG         , "null-t");
      preserve_tag (env, CONS_TAG         , "cons-t");
      preserve_tag (env, VECT_TAG         , "vect-t");
      preserve_tag (env, DICT_TAG         , "dict-t");
      preserve_tag (env, MODULE_TAG       , "module-t");
      preserve_tag (env, KEYWORD_TAG      , "keyword-t");
      preserve_tag (env, MACRO_TAG        , "macro-t");
      preserve_tag (env, TOP_KEYWORD_TAG  , "top-keyword-t");
      preserve_tag (env, SYM_TAG          , "sym-t");
      preserve_tag (env, NOTHING_TAG      , "nothing-t");
      preserve_tag (env, JUST_TAG         , "just-t");
  }
  pub struct Data {
      tag: Tag,
      obj_map: ObjMap,
      name_vector: NameVec,
  }

  impl Obj for Data {
      fn tag (&self) -> Tag { self.tag }

      fn obj_map (&self) -> ObjMap { self.obj_map.clone () }
  }