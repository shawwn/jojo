  use std::rc::Rc;
  use dic::Dic;

  pub use bool::*;
  pub use num::*;
  pub use data::*;
  pub use closure::*;
  pub use prim::*;
  pub use bool::*;
  pub use str::*;
  pub use sym::*;
  pub use num::*;
  pub use list::*;
  pub use vect::*;
  pub use dict::*;
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
  pub type ObjVec = Vec <Ptr <Obj>>;
    pub fn obj_stack_pop_to_vec (
        env: &mut Env,
        len: usize,
    ) -> ObjVec {
        let mut obj_vec = ObjVec::new ();
        (0..len)
            .into_iter ()
            .for_each (|_| obj_vec.push (
                env.obj_stack.pop () .unwrap ()));
        obj_vec
    }
    pub fn obj_dic_eat_obj_vec (
        obj_dic: &ObjDic,
        obj_vec: ObjVec,
    ) -> ObjDic {
        let mut obj_dic = obj_dic.clone ();
        obj_vec
            .into_iter ()
            .rev ()
            .for_each (|obj| obj_dic.eat (obj));
        obj_dic
    }

    pub fn obj_dic_pick_up (
        env: &mut Env,
        obj_dic: &ObjDic,
        arity: usize,
    ) -> ObjDic {
        obj_dic_eat_obj_vec (
            obj_dic,
            obj_stack_pop_to_vec (env, arity))
    }
    pub fn local_scope_extend (
        local_scope: Ptr <LocalScope>,
        obj_dic: ObjDic,
    ) -> Ptr <LocalScope> {
        let mut obj_dic_vec = (*local_scope).clone ();
        obj_dic_vec.push (obj_dic);
        Ptr::new (obj_dic_vec)
    }
    pub fn name_of_tag (
        env: &Env,
        tag: Tag,
    ) -> Name {
        if tag >= env.type_dic.len () {
            format! ("#<unknown-tag:{}>", tag.to_string ())
        } else {
            let entry = env.type_dic.idx (tag);
            entry.name.clone ()
        }
    }

    fn preserve_tag (
        env: &mut Env,
        tag: Tag,
        name: &str,
    ) {
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
  pub struct Env {
      pub obj_dic: ObjDic,
      pub type_dic: TypeDic,
      pub obj_stack: ObjStack,
      pub frame_stack: FrameStack,
  }

  impl Env {
      pub fn new () -> Env {
          let mut env = Env {
              obj_dic: ObjDic::new (),
              type_dic: TypeDic::new (),
              obj_stack: ObjStack::new (),
              frame_stack: FrameStack::new (),
          };
          init_type_dic (&mut env);
          env
      }

      pub fn step (&mut self) {
          if let Some (mut frame) = self.frame_stack.pop () {
              let jo = frame.jojo [frame.index] .clone ();
              frame.index += 1;
              if frame.index < frame.jojo.len () {
                  let local_scope = frame.local_scope.clone ();
                  self.frame_stack.push (frame);
                  jo.exe (self, local_scope);
              } else {
                  jo.exe (self, frame.local_scope);
              }
          }
      }

      pub fn run (&mut self) {
          while ! self.frame_stack.is_empty () {
              self.step ();
          }
      }

      pub fn run_with_base (&mut self, base: usize) {
          while self.frame_stack.len () > base {
              self.step ();
          }
      }

      pub fn define (
          &mut self,
          name: &str,
          obj: Ptr <Obj>,
      ) -> Id {
          self.obj_dic.ins (name, Some (obj.clone ()))
      }

      pub fn define_type (
          &mut self,
          name: &str,
          typ: Ptr <Type>,
      ) -> Tag {
          self.type_dic.ins (name, Some (typ.clone ()))
      }
  }
  pub trait Obj {
      fn tag (&self) -> Tag;
      fn obj_dic (&self) -> ObjDic;

      fn get (&self, name: &str) -> Option <Ptr <Obj>> {
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

      fn apply (&self, env: &mut Env, arity: usize) {
          eprintln! ("- Obj::apply");
          eprintln! ("  applying non applicable object");
          eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
          eprintln! ("  obj : {}", self.repr (env));
          eprintln! ("  arity : {}", arity);
          panic! ("jojo fatal error!");
      }

      // fn apply_to_arg_dict (&self, env: &mut Env) {
      //     eprintln! ("- Obj::apply_to_arg_dict");
      //     eprintln! ("  applying non applicable object");
      //     eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
      //     eprintln! ("  obj : {}", self.repr (&env));
      //     panic! ("jojo fatal error!");
      // }
  }
  pub struct Frame {
      pub index: usize,
      pub jojo: Ptr <JoVec>,
      pub local_scope: Ptr <LocalScope>,
  }
    pub trait Jo {
        fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>);

        fn repr (&self, _env: &Env) -> String {
            "#<unknown-jo>".to_string ()
        }
    }
    struct RefJo {
        id: Id,
    }

    impl Jo for RefJo {
        fn exe (&self, env: &mut Env, _local_scope: Ptr <LocalScope>) {
            let entry = env.obj_dic.idx (self.id);
            if let Some (obj) = &entry.value {
                env.obj_stack.push (obj.clone ());
            } else {
                eprintln! ("- RefJo::exe");
                eprintln! ("  undefined name : {}", entry.name);
                eprintln! ("  id : {}", self.id);
                panic! ("jojo fatal error!");
            }
        }
    }
    struct LocalRefJo {
        level: usize,
        index: usize,
    }

    impl Jo for LocalRefJo {
        fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>) {
            let i = local_scope.len () - self.level - 1;
            let obj_dic = &local_scope [i];
            let i = obj_dic.len () - self.index - 1;
            let entry = obj_dic.idx (i);
            if let Some (obj) = &entry.value {
                env.obj_stack.push (obj.clone ());
            } else {
                eprintln! ("- LocalRefJo::exe");
                eprintln! ("  undefined name : {}", entry.name);
                eprintln! ("  level : {}", self.level);
                eprintln! ("  index : {}", self.index);
                panic! ("jojo fatal error!");
            }

        }
    }
    struct ApplyJo {
        arity: usize,
    }

    impl Jo for ApplyJo {
        fn exe (&self, env: &mut Env, _local_scope: Ptr <LocalScope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            obj.apply (env, self.arity);
        }
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
    #[test]
    fn test_step () {
        let mut env = Env::new ();
        let id = env.define ("t1", Ptr::new (Type {
            obj_dic: ObjDic::new (),
            tag_of_type: TYPE_TAG,
            super_tag_vec: TagVec::new (),
        }));

        let jo_vec: JoVec = vec! [
            Ptr::new (RefJo {id}),
            Ptr::new (RefJo {id}),
        ];

        let frame = Box::new (Frame {
            index: 0,
            jojo: Ptr::new (jo_vec),
            local_scope: Ptr::new (LocalScope::new ()),
        });
        env.frame_stack.push (frame);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
    }
