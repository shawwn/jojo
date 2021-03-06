    use std::sync::Arc;
    use std::sync::Weak;
    use std::sync::Mutex;

    use std::collections::HashMap;

    use std::path::Path;
    use std::path::PathBuf;

    use std::fs;
    use std::env;
    use dic::Dic;
    use crate::token;
  pub type Name = String;

  pub type ObjCell = Arc <Mutex <Option <Arc <Obj>>>>;
  pub type ObjCellDic = Dic <ObjCell>;
  pub type ObjId = Weak <Mutex <Option <Arc <Obj>>>>;

  pub type ObjDic = Dic <Arc <Obj>>;
  pub type TypeDic = Dic <Arc <Type>>;

  pub type ObjStack = Vec <Arc <Obj>>;
  pub type FrameStack = Vec <Box <Frame>>;

  pub type Scope = Vec <ObjDic>; // index from end

  pub type StringVec = Vec <String>;
  pub type CharVec = Vec <char>;
  pub type NameVec = Vec <Name>;
  pub type TagVec = Vec <Tag>;
  pub type ObjVec = Vec <Arc <Obj>>;
  pub type JoVec = Vec <Arc <Jo>>;
      fn vec_peek <T> (vec: &Vec <T>, index: usize) -> &T {
            let back_index = vec.len () - index - 1;
            &vec [back_index]
      }
      fn str_vec_join (vec: Vec <&str>, c: char) -> String {
          let mut string = String::new ();
          for s in vec {
              string = format! ("{}{}", s, c);
          }
          string.pop ();
          string
      }
      pub fn obj_vec_eq (
          lhs: &ObjVec,
          rhs: &ObjVec,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| obj_eq (&p.0, &p.1)))
      }
      pub fn obj_vec_rev (obj_vec: &ObjVec) -> ObjVec {
          obj_vec
              .clone ()
              .into_iter ()
              .rev ()
              .collect::<ObjVec> ()
      }
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
      pub fn obj_stack_eq (
          lhs: &ObjVec,
          rhs: &ObjVec,
      ) -> bool {
          obj_vec_eq (lhs, rhs)
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
      fn obj_dic_eq (
          lhs: &ObjDic,
          rhs: &ObjDic,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| ((p.0).0 == (p.1).0 &&
                      obj_eq (& (p.0).1, & (p.1).1))))
      }
      pub fn scope_extend (
          scope: &Scope,
          obj_dic: ObjDic,
      ) -> Arc <Scope> {
          let mut obj_dic_vec = scope.clone ();
          obj_dic_vec.push (obj_dic);
          Arc::new (obj_dic_vec)
      }
      pub fn scope_eq (
          lhs: &Scope,
          rhs: &Scope,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| obj_dic_eq (p.0, p.1)))
      }
      fn new_jojo () -> Arc <JoVec> {
          let jo_vec = JoVec::new ();
          Arc::new (jo_vec)
      }
      fn jojo_append (
          ante: &JoVec,
          succ: &JoVec,
      ) -> Arc <JoVec> {
          let mut jo_vec = ante.clone ();
          jo_vec.append (&mut succ.clone ());
          Arc::new (jo_vec)
      }
      pub fn jojo_eq (
          lhs: &JoVec,
          rhs: &JoVec,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| jo_eq (p.0.dup (),
                            p.1.dup ())))
      }
      fn frame_stack_eq (
          lhs: &FrameStack,
          rhs: &FrameStack,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter () .zip (rhs.iter ())
           .all (|p| frame_eq (&p.0, &p.1)))
      }
      #[derive(Clone)]
      #[derive(Debug)]
      #[derive(PartialEq)]
      #[derive(Eq)]
      #[derive(Hash)]
      pub struct Tag {
          module_path: PathBuf,
          index: usize,
      }
      pub fn name_of_tag (
          env: &Env,
          tag: Tag,
      ) -> Name {
          if tag.index >= env.obj_cell_dic.len () {
              format! ("#<unknown-tag:{}>", tag.index.to_string ())
          } else {
              let entry = env.obj_cell_dic.idx (tag.index);
              entry.name.clone ()
          }
      }
      fn preserve_index (
          env: &mut Env,
          index: usize,
          name: &str,
      ) {
          let next_index = env.obj_cell_dic.len ();
          let module_path = PathBuf::new ();
          let tag = Tag { module_path, index };
          env.define (name, Type::make (tag));
          assert_eq! (index, next_index);
      }
      pub const CLOSURE_T         : usize = 0;
      pub const TYPE_T            : usize = 1;
      pub const TRUE_T            : usize = 2;
      pub const FALSE_T           : usize = 3;
      pub const DATA_CONS_T       : usize = 4;
      pub const PRIM_T            : usize = 5;
      pub const NUM_T             : usize = 6;
      pub const STR_T             : usize = 7;
      pub const SYM_T             : usize = 8;
      pub const NULL_T            : usize = 9;
      pub const CONS_T            : usize = 10;
      pub const VECT_T            : usize = 11;
      pub const DICT_T            : usize = 12;
      pub const MODULE_T          : usize = 13;
      pub const KEYWORD_T         : usize = 14;
      pub const MACRO_T           : usize = 15;
      pub const TOP_KEYWORD_T     : usize = 16;
      pub const NONE_T            : usize = 17;
      pub const SOME_T            : usize = 18;
      fn init_prim_type (env: &mut Env) {
          preserve_index (env, CLOSURE_T         , "closure-t");
          preserve_index (env, TYPE_T            , "type-t");
          preserve_index (env, TRUE_T            , "true-t");
          preserve_index (env, FALSE_T           , "false-t");
          preserve_index (env, DATA_CONS_T       , "data-cons-t");
          preserve_index (env, PRIM_T            , "prim-t");
          preserve_index (env, NUM_T             , "num-t");
          preserve_index (env, STR_T             , "str-t");
          preserve_index (env, SYM_T             , "sym-t");
          preserve_index (env, NULL_T            , "null-t");
          preserve_index (env, CONS_T            , "cons-t");
          preserve_index (env, VECT_T            , "vect-t");
          preserve_index (env, DICT_T            , "dict-t");
          preserve_index (env, MODULE_T          , "module-t");
          preserve_index (env, KEYWORD_T         , "keyword-t");
          preserve_index (env, MACRO_T           , "macro-t");
          preserve_index (env, TOP_KEYWORD_T     , "top-keyword-t");
          preserve_index (env, NONE_T            , "none-t");
          preserve_index (env, SOME_T            , "some-t");
      }
      pub trait Dup {
         fn dup (&self) -> Self;
      }
      impl Dup for Arc <Obj> {
          fn dup (&self) -> Self {
              Arc::clone (self)
          }
      }
      impl Dup for Arc <Type> {
          fn dup (&self) -> Self {
              Arc::clone (self)
          }
      }
      // impl Dup for Arc <Type> {
      //     fn dup (&self) -> Self {
      //         Arc::clone (self)
      //     }
      // }
      impl Dup for Arc <Jo> {
          fn dup (&self) -> Self {
              Arc::clone (self)
          }
      }
      impl Dup for Arc <Scope> {
          fn dup (&self) -> Self {
              Arc::clone (self)
          }
      }
      impl Dup for Arc <JoVec> {
          fn dup (&self) -> Self {
              Arc::clone (self)
          }
      }
      impl Dup for Arc <ObjDic> {
          fn dup (&self) -> Self {
              Arc::clone (self)
          }
      }
      macro_rules! impl_core_type {
          ( $type:ty, $index:expr ) => {
              impl $type {

                  pub fn tag () -> Tag {
                      Tag {
                          module_path: PathBuf::new (),
                          index: $index,
                      }
                  }

                  pub fn cast (obj: Arc <Obj>) -> Arc <Self> {
                      assert! (Self::p (&obj));
                      unsafe {
                          obj_to::<Self> (obj)
                      }
                  }

                  pub fn p (x: &Arc <Obj>) -> bool {
                      let tag = x.tag ();
                      (Self::tag () == tag)
                  }
              }};
      }
      macro_rules! jojo {
          ( $( $x:expr ),* $(,)* ) => {{
              let jo_vec: JoVec = vec! [
                  $( Arc::new ($x) ),*
              ];
              Arc::new (jo_vec)
          }};
      }
      macro_rules! frame {
          ( $( $x:expr ),* $(,)* ) => {{
              let jo_vec: JoVec = vec! [
                  $( Arc::new ($x) ),*
              ];
              Frame::make (jo_vec)
          }};
      }
    pub struct Env {
        pub obj_cell_dic: ObjCellDic,
        pub obj_stack: ObjStack,
        pub frame_stack: FrameStack,
        pub current_dir: PathBuf,
        pub module_path: PathBuf,
    }

    impl Env {
        pub fn new () -> Env {
            let mut env = Env {
                obj_cell_dic: ObjCellDic::new (),
                obj_stack: ObjStack::new (),
                frame_stack: FrameStack::new (),
                current_dir: env::current_dir () .unwrap (),
                module_path: PathBuf::new (),
            };
            init_prim_type (&mut env);
            env
        }

        pub fn step (&mut self) {
            if let Some (mut frame) = self.frame_stack.pop () {
                let index = frame.index;
                let jo = frame.jojo [frame.index] .dup ();
                frame.index += 1;
                if index + 1 < frame.jojo.len () {
                    let scope = frame.scope.dup ();
                    self.frame_stack.push (frame);
                    jo.exe (self, scope);
                } else {
                    jo.exe (self, frame.scope);
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
    }
    impl Env {
        pub fn define (
            &mut self,
            name: &str,
            obj: Arc <Obj>,
        ) -> ObjId {
            if let Some (obj_cell) = self.obj_cell_dic.get (name) {
                let mut obj_ptr = obj_cell.lock () .unwrap ();
                *obj_ptr = Some (obj);
                Arc::downgrade (&obj_cell)
            } else {
                let obj_cell = Arc::new (Mutex::new (Some (obj)));
                let id = Arc::downgrade (&obj_cell);
                self.obj_cell_dic.ins (name, Some (obj_cell));
                id
            }
        }
    }
    impl Env {
        pub fn find_obj (
            &self,
            name: &str,
        ) -> Option <Arc <Obj>> {
            if let Some (obj_cell) = self.obj_cell_dic.get (name) {
                if let Some (ref obj) = *obj_cell.lock () .unwrap () {
                    Some (obj.dup ())
                } else {
                    None
                }
            } else {
                None
            }
        }
    }
    impl Env {
        pub fn idx_obj (
            &self,
            index: usize,
        ) -> Option <Arc <Obj>> {
            let entry = self.obj_cell_dic.idx (index);
            if let Some (obj_cell) = &entry.value {
                if let Some (ref obj) = *obj_cell.lock () .unwrap () {
                    Some (obj.dup ())
                } else {
                    None
                }
            } else {
                None
            }
        }
    }
    fn method_dic_extend (
        obj_dic: &ObjDic,
        name: &str,
        obj: Arc <Obj>,
    ) -> Arc <ObjDic> {
        let mut obj_dic = obj_dic.clone ();
        if obj_dic.has_name (name) {
            obj_dic.set (name, Some (obj));
        } else {
            obj_dic.ins (name, Some (obj));
        }
        Arc::new (obj_dic)
    }
    impl Env {
        pub fn assign (
            &mut self,
            type_name: &str,
            name: &str,
            obj: Arc <Obj>,
        ) {
            if type_name == "" {
                self.define (name, obj);
            } else {
                if let Some (typ) = self.find_obj (type_name) {
                    let typ = Type::cast (typ);
                    let new_typ = Arc::new (Type  {
                        method_dic: method_dic_extend (
                            &typ.method_dic, name, obj),
                        tag_of_type: typ.tag_of_type.clone (),
                        // super_tag_vec: typ.super_tag_vec.clone (),
                    });
                    self.define (type_name, new_typ);
                } else {
                    eprintln! ("- Env::assign");
                    eprintln! ("  unknown type_name : {}", type_name);
                    eprintln! ("  name : {}", name);
                    eprintln! ("  obj : {}", obj.repr (self));
                    panic! ("jojo fatal error!");
                }
            }
        }
    }
    pub struct Frame {
        pub index: usize,
        pub jojo: Arc <JoVec>,
        pub scope: Arc <Scope>,
    }
    fn frame_eq (
        lhs: &Frame,
        rhs: &Frame,
    ) -> bool {
        (lhs.index == rhs.index &&
         jojo_eq (&lhs.jojo, &rhs.jojo) &&
         scope_eq (&lhs.scope, &rhs.scope))
    }
    impl Frame {
        pub fn make (jo_vec: JoVec) -> Box <Frame> {
            Box::new (Frame {
                index: 0,
                jojo: Arc::new (jo_vec),
                scope: Arc::new (Scope::new ()),
            })
        }
    }
    pub trait Obj {
        fn tag (&self) -> Tag;

        fn typ (
            &self,
            env: &Env,
        ) -> Arc <Type> {
            let tag = self.tag ();
            assert! (tag.module_path == PathBuf::new ());
            let obj = env.idx_obj (tag.index) .unwrap ();
            Type::cast (obj)
        }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> { None }

        fn eq (&self, _other: Arc <Obj>) -> bool { false }

        fn get (
            &self,
            name: &str,
        ) -> Option <Arc <Obj>> {
            if let Some (obj_dic) = self.obj_dic () {
                if let Some (obj) = obj_dic.get (name) {
                    Some (obj.dup ())
                } else {
                    None
                }
            } else {
                None
            }
        }

        fn get_method (
            &self,
            env: &Env,
            name: &str,
        ) -> Option <Arc <Obj>> {
            let typ = self.typ (env);
            typ.get (name)
        }

        fn dot (
            &self,
            env: &Env,
            name: &str,
        ) -> Option <Arc <Obj>> {
            if let Some (obj) = self.get (name) {
                Some (obj)
            } else {
                self.get_method (env, name)
            }
        }

        fn repr (&self, env: &Env) -> String {
            format! ("#<{}>", name_of_tag (&env, self.tag ()))
        }

        fn print (&self, env: &Env) {
            print! ("{}", self.repr (&env));
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            eprintln! ("- Obj::apply");
            eprintln! ("  applying non applicable object");
            eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
            eprintln! ("  obj : {}", self.repr (env));
            eprintln! ("  arity : {}", arity);
            panic! ("jojo fatal error!");
        }

        fn apply_to_arg_dict (&self, env: &mut Env) {
            eprintln! ("- Obj::apply_to_arg_dict");
            eprintln! ("  applying non applicable object");
            eprintln! ("  tag : {}", name_of_tag (&env, self.tag ()));
            eprintln! ("  obj : {}", self.repr (&env));
            panic! ("jojo fatal error!");
        }
    }
    /// Before cast an obj to T, caller must check that
    ///   the obj has the tag of T.
    unsafe fn obj_to <T: Obj> (obj: Arc <Obj>) -> Arc <T> {
        let obj_ptr = Arc::into_raw (obj);
        let obj_ptr = obj_ptr as *const Obj as *const T;
        Arc::from_raw (obj_ptr)
    }
    pub fn obj_eq (
        lhs: &Arc <Obj>,
        rhs: &Arc <Obj>,
    ) -> bool {
        lhs.eq (rhs.dup ())
    }
    pub trait Jo {
        fn exe (&self, env: &mut Env, scope: Arc <Scope>);

        fn repr (&self, _env: &Env) -> String {
            "#<unknown-jo>".to_string ()
        }
    }
    pub fn jo_eq (
        lhs: Arc <Jo>,
        rhs: Arc <Jo>,
    ) -> bool {
        let lhs_ptr = Arc::into_raw (lhs);
        let rhs_ptr = Arc::into_raw (rhs);
        lhs_ptr == rhs_ptr
    }
    pub struct RefJo {
        id: ObjId,
    }

    impl Jo for RefJo {
        fn exe (&self, env: &mut Env, _: Arc <Scope>) {
            let obj_cell = self.id .upgrade () .unwrap ();
            let mutex_guard = obj_cell.lock () .unwrap ();
            if let Some (ref obj) = *mutex_guard {
                env.obj_stack.push (obj.dup ());
            } else {
                eprintln! ("- RefJo::exe");
                eprintln! ("  undefined id");
                panic! ("jojo fatal error!");
            }
        }
    }
    pub struct LocalRefJo {
        level: usize,
        index: usize,
    }

    impl Jo for LocalRefJo {
        fn exe (&self, env: &mut Env, scope: Arc <Scope>) {
            let obj_dic = vec_peek (&scope, self.level);
            let entry = obj_dic.idx (self.index);
            if let Some (ref obj) = entry.value {
                env.obj_stack.push (obj.dup ());
            } else {
                eprintln! ("- LocalRefJo::exe");
                eprintln! ("  undefined name : {}", entry.name);
                eprintln! ("  level : {}", self.level);
                eprintln! ("  index : {}", self.index);
                panic! ("jojo fatal error!");
            }
        }
    }
    pub struct ApplyJo {
        arity: usize,
    }

    impl Jo for ApplyJo {
        fn exe (&self, env: &mut Env, _: Arc <Scope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            obj.apply (env, self.arity);
        }
    }
    pub struct ApplyToArgDictJo;

    impl Jo for ApplyToArgDictJo {
        fn exe (&self, env: &mut Env, _: Arc <Scope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            obj.apply_to_arg_dict (env);
        }
    }
    fn method_p (obj: &Arc <Obj>) -> bool {
        if Closure::p (&obj) {
            let closure = Closure::cast (obj.dup ());
            if closure.arg_dic.len () == 0 {
                false
            } else {
                let entry = closure.arg_dic.idx (0);
                (entry.name == "self" &&
                 entry.value.is_none ())
            }
        } else {
            false
        }
    }
    fn method_merge_self (
        method: Arc <Obj>,
        self_obj: Arc <Obj>,
    ) -> Arc <Closure> {
        let method = Closure::cast (method);
        let mut arg_dic = (*method.arg_dic).clone ();
        arg_dic.idx_set_value (0, Some (self_obj));
        Arc::new (Closure {
            arg_dic: Arc::new (arg_dic),
            jojo: method.jojo.dup (),
            scope: method.scope.dup (),
        })
    }
    pub struct DotJo {
        name: String,
    }

    impl Jo for DotJo {
        fn exe (&self, env: &mut Env, _: Arc <Scope>) {
            let obj = env.obj_stack.pop () .unwrap ();
            let dot = obj.dot (env, &self.name) .unwrap ();
            if method_p (&dot) {
                env.obj_stack.push (method_merge_self (dot, obj));
            } else {
                env.obj_stack.push (dot);
            }
        }
    }
    pub struct LambdaJo {
        arg_dic: Arc <ObjDic>,
        jojo: Arc <JoVec>,
    }

    impl Jo for LambdaJo {
        fn exe (&self, env: &mut Env, scope: Arc <Scope>) {
            env.obj_stack.push (Arc::new (Closure {
                arg_dic: self.arg_dic.dup (),
                jojo: self.jojo.dup (),
                scope: scope.dup (),
            }));
        }
    }
    pub struct LitJo {
        obj: Arc <Obj>,
    }

    impl Jo for LitJo {
        fn exe (&self, env: &mut Env, _: Arc <Scope>) {
            env.obj_stack.push (self.obj.dup ());
        }
    }
    pub struct Type {
        method_dic: Arc <ObjDic>,
        tag_of_type: Tag,
        // super_tag_vec: TagVec,
    }

    impl_core_type! (Type, TYPE_T);

    impl Obj for Type {
        fn tag (&self) -> Tag { Type::tag () }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            Some (self.method_dic.dup ())
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Type::cast (other);
                // (self.tag_of_type == other.tag_of_type &&
                //  self.super_tag_vec == other.super_tag_vec)
                (self.tag_of_type == other.tag_of_type)
            }
        }
    }
    pub fn type_eq (
        lhs: &Arc <Type>,
        rhs: &Arc <Type>,
    ) -> bool {
        lhs.eq (rhs.dup ())
    }
    impl Type {
        fn make (tag: Tag) -> Arc <Type> {
            Arc::new (Type {
                method_dic: Arc::new (ObjDic::new ()),
                tag_of_type: tag,
                // super_tag_vec: TagVec::new (),
            })
        }
    }
    fn type_of (env: &Env, obj: Arc <Obj>) -> Arc <Type> {
        obj.typ (env)
    }
    pub struct Data {
        tag_of_type: Tag,
        typ_id: ObjId,
        field_dic: Arc <ObjDic>,
    }

    impl Obj for Data {
        fn tag (&self) -> Tag { self.tag_of_type.clone () }

        fn typ (
            &self,
            _env: &Env,
        ) -> Arc <Type> {
            let obj_cell = self.typ_id .upgrade () .unwrap ();
            let mutex_guard = obj_cell .lock () .unwrap ();
            if let Some (ref obj) = *mutex_guard {
                Type::cast (obj.dup ())
            } else {
                eprintln! ("- Data::typ");
                eprintln! ("  empty cell");
                panic! ("jojo fatal error!");
            }
        }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            Some (self.field_dic.dup ())
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                unsafe {
                    let other = obj_to::<Data> (other);
                    (self.tag_of_type == other.tag_of_type &&
                     obj_dic_eq (&self.field_dic, &other.field_dic))
                }
            }
        }
    }
    pub struct DataCons {
        tag_of_type: Tag,
        typ_id: ObjId,
        field_dic: Arc <ObjDic>,
    }

    impl_core_type! (DataCons, DATA_CONS_T);

    impl Obj for DataCons {
        fn tag (&self) -> Tag { Self::tag () }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            Some (self.field_dic.dup ())
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = DataCons::cast (other);
                (self.tag_of_type == other.tag_of_type &&
                 obj_dic_eq (&self.field_dic, &other.field_dic))
            }
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            let lack = self.field_dic.lack ();
            if arity > lack {
                eprintln! ("- DataCons::apply");
                eprintln! ("  over-arity apply");
                eprintln! ("  arity > lack");
                eprintln! ("  arity : {}", arity);
                eprintln! ("  lack : {}", lack);
                panic! ("jojo fatal error!");
            }
            let tag_of_type = self.tag_of_type.clone ();
            let typ_id = self.typ_id.clone ();
            let field_dic = obj_dic_pick_up (
                env, &self.field_dic, arity);
            if arity == lack {
                env.obj_stack.push (Arc::new (Data {
                    tag_of_type,
                    typ_id,
                    field_dic: Arc::new (field_dic),
                }));
            } else {
                env.obj_stack.push (Arc::new (DataCons {
                    tag_of_type,
                    typ_id,
                    field_dic: Arc::new (field_dic),
                }));
            }
        }
    }
    impl DataCons {
        pub fn make (
            tag: Tag,
            typ_id: ObjId,
            vec: Vec <String>,
        ) -> Arc <DataCons> {
            Arc::new (DataCons {
                tag_of_type: tag,
                typ_id,
                field_dic: Arc::new (Dic::from (vec)),
            })
        }
    }
    pub struct Closure {
        arg_dic: Arc <ObjDic>,
        jojo: Arc <JoVec>,
        scope: Arc <Scope>,
    }

    impl_core_type! (Closure, CLOSURE_T);

    impl Obj for Closure {
        fn tag (&self) -> Tag { Self::tag () }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            Some (self.arg_dic.dup ())
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Closure::cast (other);
                (jojo_eq (&self.jojo, &other.jojo) &&
                 scope_eq (&self.scope, &other.scope) &&
                 obj_dic_eq (&self.arg_dic, &other.arg_dic))
            }
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            let lack = self.arg_dic.lack ();
            if arity > lack {
                eprintln! ("- Closure::apply");
                eprintln! ("  over-arity apply");
                eprintln! ("  arity > lack");
                eprintln! ("  arity : {}", arity);
                eprintln! ("  lack : {}", lack);
                panic! ("jojo fatal error!");
            }
            let arg_dic = obj_dic_pick_up (env, &self.arg_dic, arity);
            if arity == lack {
                env.frame_stack.push (Box::new (Frame {
                    index: 0,
                    jojo: self.jojo.dup (),
                    scope: scope_extend (
                        &self.scope, arg_dic),
                }));
            } else {
                env.obj_stack.push (Arc::new (Closure {
                    arg_dic: Arc::new (arg_dic),
                    jojo: self.jojo.dup (),
                    scope: self.scope.dup (),
                }));
            }
        }
    }
    pub type PrimFn = fn (
        env: &mut Env,
        arg_dic: &ObjDic,
    );
    pub fn prim_fn_eq (
        lhs: &PrimFn,
        rhs: &PrimFn,
    ) -> bool {
        (*lhs) as usize == (*rhs) as usize
    }
    pub struct Prim {
        arg_dic: ObjDic,
        fun: PrimFn,
    }

    impl_core_type! (Prim, PRIM_T);

    impl Obj for Prim {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Prim::cast (other);
                (obj_dic_eq (&self.arg_dic, &other.arg_dic) &&
                 prim_fn_eq (&self.fun, &other.fun))
            }
        }

        fn apply (&self, env: &mut Env, arity: usize) {
            let lack = self.arg_dic.lack ();
            if arity > lack {
                eprintln! ("- Prim::apply");
                eprintln! ("  over-arity apply");
                eprintln! ("  arity > lack");
                eprintln! ("  arity : {}", arity);
                eprintln! ("  lack : {}", lack);
                panic! ("jojo fatal error!");
            }
            let fun = self.fun;
            let arg_dic = obj_dic_pick_up (env, &self.arg_dic, arity);
            if arity == lack {
                fun (env, &arg_dic);
            } else {
                env.obj_stack.push (Arc::new (Prim {
                    arg_dic,
                    fun,
                }));
            }
        }
    }
    impl Env {
        pub fn define_prim (
            &mut self,
            name: &str,
            name_vec: Vec <&str>,
            fun: PrimFn,
        ) -> ObjId {
            let arg_vec = name_vec. iter ()
                .map (|x| x.to_string ())
                .collect::<NameVec> ();
            self.define (name, Arc::new (Prim {
                arg_dic: Dic::from (arg_vec),
                fun,
            }))
        }
    }
    macro_rules! define_prim {
        ( $env:expr, $name:expr,
          [$arg0:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0)));
                });
        };

        ( $env:expr, $name:expr,
          [$arg0:expr, $arg1:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0, $arg1],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0),
                        arg_idx (arg, 1)));
                });
        };

        ( $env:expr, $name:expr,
          [$arg0:expr, $arg1:expr, $arg2:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0, $arg1, $arg2],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0),
                        arg_idx (arg, 1),
                        arg_idx (arg, 2)));
                });
        };

        ( $env:expr, $name:expr,
          [$arg0:expr, $arg1:expr, $arg2:expr, $arg3:expr],
          $fun:expr ) => {
            ($env).define_prim (
                $name,
                vec! [$arg0, $arg1, $arg2, $arg3],
                |env, arg| {
                    env.obj_stack.push ($fun (
                        arg_idx (arg, 0),
                        arg_idx (arg, 1),
                        arg_idx (arg, 2),
                        arg_idx (arg, 3)));
                });
        };
    }
    pub struct True;

    impl_core_type! (True, TRUE_T);

    impl Obj for True {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl True {
        fn make () -> Arc <True> {
            Arc::new (True {})
        }
    }
    pub struct False;

    impl_core_type! (False, FALSE_T);

    impl Obj for False {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl False {
        fn make () -> Arc <False> {
            Arc::new (False {})
        }
    }
    pub fn not (x: Arc <Obj>) -> Arc <Obj> {
        make_bool (False::p (&x))
    }
    pub fn make_bool (b: bool) -> Arc <Obj> {
        if b {
            True::make ()
        }
        else {
            False::make ()
        }
    }
    pub struct Str { pub str: String }

    impl_core_type! (Str, STR_T);

    impl Obj for Str {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Str::cast (other);
                (self.str == other.str)
            }
        }
    }
    impl Str {
        fn make (str: &str) -> Arc <Str> {
            Arc::new (Str { str: String::from (str) })
        }
    }
    fn str_length (str: Arc <Obj>) -> Arc <Num> {
        let str = Str::cast (str);
        Num::make (str.str.len () as f64)
    }
    fn str_append (
        ante: Arc <Obj>,
        succ: Arc <Obj>,
    ) -> Arc <Str> {
        let ante = Str::cast (ante);
        let succ = Str::cast (succ);
        Str::make (&format! ("{}{}", ante.str, succ.str))
    }
    fn str_slice (
        str: Arc <Obj>,
        begin: Arc <Obj>,
        end: Arc <Obj>,
    ) -> Arc <Str> {
        let str = Str::cast (str);
        let begin = Num::cast (begin);
        let end = Num::cast (end);
        let char_vec = str.str.chars() .collect::<CharVec> ();
        let begin = begin.num as usize;
        let end = end.num as usize;
        let slice = &char_vec [begin..end];
        Str::make (&slice .iter () .collect::<String> ())
    }
    fn str_ref (
        str: Arc <Obj>,
        index: Arc <Obj>,
    ) -> Arc <Str> {
        str_slice (str, index.dup (), inc (index))
    }
    fn str_head (str: Arc <Obj>) -> Arc <Str> {
        str_ref (str, Num::make (0.0))
    }
    fn str_rest (str: Arc <Obj>) -> Arc <Str> {
        let len = str_length (str.dup ());
        str_slice (str, Num::make (1.0), len)
    }
    pub struct Sym { pub sym: String }

    impl_core_type! (Sym, SYM_T);

    impl Obj for Sym {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Sym::cast (other);
                (self.sym == other.sym)
            }
        }
    }
    impl Sym {
        fn make (str: &str) -> Arc <Sym> {
            Arc::new (Sym { sym: String::from (str) })
        }
    }
    fn sym_length (sym: Arc <Obj>) -> Arc <Num> {
        let sym = Sym::cast (sym);
        Num::make (sym.sym.len () as f64)
    }
    fn sym_append (
        ante: Arc <Obj>,
        succ: Arc <Obj>,
    ) -> Arc <Sym> {
        let ante = Sym::cast (ante);
        let succ = Sym::cast (succ);
        Sym::make (&format! ("{}{}", ante.sym, succ.sym))
    }
    fn sym_slice (
        sym: Arc <Obj>,
        begin: Arc <Obj>,
        end: Arc <Obj>,
    ) -> Arc <Sym> {
        let sym = Sym::cast (sym);
        let begin = Num::cast (begin);
        let end = Num::cast (end);
        let char_vec = sym.sym.chars() .collect::<CharVec> ();
        let begin = begin.num as usize;
        let end = end.num as usize;
        let slice = &char_vec [begin..end];
        Sym::make (&slice .iter () .collect::<String> ())
    }
    fn sym_ref (
        sym: Arc <Obj>,
        index: Arc <Obj>,
    ) -> Arc <Sym> {
        sym_slice (sym, index.dup (), inc (index))
    }
    fn sym_head (sym: Arc <Obj>) -> Arc <Sym> {
        sym_ref (sym, Num::make (0.0))
    }
    fn sym_rest (sym: Arc <Obj>) -> Arc <Sym> {
        let len = sym_length (sym.dup ());
        sym_slice (sym, Num::make (1.0), len)
    }
    pub struct Num { pub num: f64 }

    impl_core_type! (Num, NUM_T);

    impl Obj for Num {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Num::cast (other);
                (self.num == other.num)
            }
        }

        fn repr (&self, _env: &Env) -> String {
            format! ("{}", self.num)
        }
    }
    impl Num {
        fn make (num: f64) -> Arc <Num> {
            Arc::new (Num { num })
        }
    }
      fn inc (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num + 1.0)
      }
      fn dec (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num - 1.0)
      }
      fn add (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num + y.num)
      }
      fn sub (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num - y.num)
      }
      fn mul (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num * y.num)
      }
      fn div (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num / y.num)
      }
      fn num_mod (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num % y.num)
      }
      fn max (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num.max (y.num))
      }
      fn min (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num.min (y.num))
      }

      fn abs (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.abs ())
      }
      fn neg (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (- x.num)
      }
      fn pow (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num.powf (y.num))
      }
      fn mul_add (x: Arc <Obj>, y: Arc <Obj>, z: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          let z = Num::cast (z);
          Num::make (x.num.mul_add (y.num, z.num))
      }
      fn reciprocal (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.recip ())
      }
      fn sqrt (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.sqrt ())
      }
      fn cbrt (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.cbrt ())
      }
      fn hypot (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num.hypot (y.num))
      }
      fn lt (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Obj> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          make_bool (x.num < y.num)
      }
      fn gt (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Obj> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          make_bool (x.num > y.num)
      }
      fn lteq (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Obj> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          make_bool (x.num <= y.num)
      }
      fn gteq (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Obj> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          make_bool (x.num >= y.num)
      }
      fn even_p (x: Arc <Obj>) -> Arc <Obj> {
          let x = Num::cast (x);
          make_bool (x.num % 2.0 == 0.0)
      }
      fn odd_p (x: Arc <Obj>) -> Arc <Obj> {
          let x = Num::cast (x);
          make_bool (x.num % 2.0 == 1.0)
      }
      fn num_integer_part (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.trunc ())
      }
      fn num_fractional_part (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.fract ())
      }
      fn num_sign (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.signum ())
      }
      fn num_floor (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.floor ())
      }
      fn num_ceil (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.ceil ())
      }
      fn num_round (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.round ())
      }
      fn num_sin (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.sin ())
      }
      fn num_cos (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.cos ())
      }
      fn num_tan (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.tan ())
      }
      fn num_asin (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.asin ())
      }
      fn num_acos (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.acos ())
      }
      fn num_atan (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.atan ())
      }
      fn num_atan2 (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num.atan2 (y.num))
      }
      fn num_sinh (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.sinh ())
      }
      fn num_cosh (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.cosh ())
      }
      fn num_tanh (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.tanh ())
      }
      fn num_asinh (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.asinh ())
      }
      fn num_acosh (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.acosh ())
      }
      fn num_atanh (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.atanh ())
      }
      fn num_exp (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.exp ())
      }
      fn num_exp2 (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.exp2 ())
      }
      fn num_exp_m1 (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.exp_m1 ())
      }
      fn num_ln (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.ln ())
      }
      fn num_ln_1p (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.ln_1p ())
      }
      fn num_log (x: Arc <Obj>, y: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          let y = Num::cast (y);
          Num::make (x.num.log (y.num))
      }
      fn num_log2 (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.log2 ())
      }
      fn num_log10 (x: Arc <Obj>) -> Arc <Num> {
          let x = Num::cast (x);
          Num::make (x.num.log10 ())
      }
    pub struct Null;

    impl_core_type! (Null, NULL_T);

    impl Obj for Null {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl Null {
        fn make () -> Arc <Null> {
            Arc::new (Null {})
        }
    }
    pub fn null () -> Arc <Obj> {
       Null::make ()
    }
    pub struct Cons {
        car: Arc <Obj>,
        cdr: Arc <Obj>,
    }

    impl_core_type! (Cons, CONS_T);

    impl Obj for Cons {
        fn tag (&self) -> Tag { Self::tag () }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            let mut obj_dic = ObjDic::new ();
            obj_dic.ins ("car", Some (self.car.dup ()));
            obj_dic.ins ("cdr", Some (self.cdr.dup ()));
            Some (Arc::new (obj_dic))
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Cons::cast (other);
                (obj_eq (&self.car, &other.car) &&
                 obj_eq (&self.cdr, &other.cdr))
            }
        }

        // fn repr (&self, env: &Env) -> String {
        //     sexp_repr (env, Arc::new (Cons {
        //         car: self.car.dup (),
        //         cdr: self.cdr.dup (),
        //     }))
        // }
    }
    impl Cons {
        fn make (car: Arc <Obj>, cdr: Arc <Obj>) -> Arc <Cons> {
            Arc::new (Cons { car, cdr })
        }
    }
    pub fn cons (car: Arc <Obj>, cdr: Arc <Obj>) -> Arc <Obj> {
        Cons::make (car, cdr)
    }
    pub fn car (cons: Arc <Obj>) -> Arc <Obj> {
        assert! (Cons::p (&cons));
        cons.get ("car") .unwrap ()
    }
    pub fn cdr (cons: Arc <Obj>) -> Arc <Obj> {
        assert! (Cons::p (&cons));
        cons.get ("cdr") .unwrap ()
    }
    pub fn list_p (x: &Arc <Obj>) -> bool {
        (Null::p (x) ||
         Cons::p (x))
    }
    fn car_as_sym (cons: Arc <Obj>) -> Arc <Sym> {
        assert! (Cons::p (&cons));
        let head = car (cons);
        Sym::cast (head)
    }
    fn list_size (mut list: Arc <Obj>) -> usize {
        assert! (list_p (&list));
        let mut size = 0;
        while ! Null::p (&list) {
            size += 1;
            list = cdr (list);
        }
        size
    }
    fn list_length (list: Arc <Obj>) -> Arc <Num> {
        assert! (list_p (&list));
        Num::make (list_size (list) as f64)
    }
    fn list_reverse (mut list: Arc <Obj>) -> Arc <Obj> {
        assert! (list_p (&list));
        let mut rev = null ();
        while ! Null::p (&list) {
            let obj = car (list.dup ());
            rev = cons (obj, rev);
            list = cdr (list);
        }
        rev
    }
    fn list_reverse_append (
        ante: Arc <Obj>,
        succ: Arc <Obj>,
    ) -> Arc <Obj> {
        let mut list = ante;
        let mut result = succ;
        while ! Null::p (&list) {
            let obj = car (list.dup ());
            result = cons (obj, result);
            list = cdr (list);
        }
        result
    }
    fn list_append (
        ante: Arc <Obj>,
        succ: Arc <Obj>,
    ) -> Arc <Obj> {
        list_reverse_append (list_reverse (ante), succ)
    }
    pub fn unit_list (obj: Arc <Obj>) -> Arc <Obj> {
        cons (obj, null ())
    }
    pub struct JNone;

    impl_core_type! (JNone, NONE_T);

    impl Obj for JNone {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                true
            }
        }
    }
    impl JNone {
        fn make () -> Arc <JNone> {
            Arc::new (JNone {})
        }
    }
    pub fn none () -> Arc <JNone> {
        JNone::make ()
    }
    pub struct JSome {
        value: Arc <Obj>,
    }

    impl_core_type! (JSome, SOME_T);

    impl Obj for JSome {
        fn tag (&self) -> Tag { Self::tag () }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            let mut obj_dic = ObjDic::new ();
            obj_dic.ins ("value", Some (self.value.dup ()));
            Some (Arc::new (obj_dic))
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = JSome::cast (other);
                (obj_eq (&self.value, &other.value))
            }
        }
    }
    impl JSome {
        fn make (value: Arc <Obj>) -> Arc <JSome> {
            Arc::new (JSome { value })
        }
    }
    pub fn some (value: Arc <Obj>) -> Arc <JSome> {
        JSome::make (value)
    }
    pub fn option_p (x: &Arc <Obj>) -> bool {
        (JNone::p (&x) && JSome::p (&x))
    }
    pub struct Vect { pub obj_vec: ObjVec }

    impl_core_type! (Vect, VECT_T);

    impl Obj for Vect {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Vect::cast (other);
                (obj_vec_eq (&self.obj_vec, &other.obj_vec))
            }
        }
    }
    impl Vect {
        fn make (obj_vec: &ObjVec) -> Arc <Vect> {
            Arc::new (Vect { obj_vec: obj_vec.clone () })
        }
    }
    pub fn vect_to_list (vect: Arc <Obj>) -> Arc <Obj> {
        let vect = Vect::cast (vect);
        let obj_vec = &vect.obj_vec;
        let mut result = null ();
        for x in obj_vec .iter () .rev () {
            result = cons (x.dup (), result);
        }
        result
    }
    fn list_to_vect (mut list: Arc <Obj>) -> Arc <Vect> {
        let mut obj_vec = ObjVec::new ();
        while Cons::p (&list) {
            obj_vec.push (car (list.dup ()));
            list = cdr (list);
        }
        Vect::make (&obj_vec)
    }
    struct CollectVectJo {
        counter: usize,
    }

    impl Jo for CollectVectJo {
        fn exe (&self, env: &mut Env, _: Arc <Scope>) {
            let mut obj_vec = ObjVec::new ();
            for _ in 0..self.counter {
                let obj = env.obj_stack.pop () .unwrap ();
                obj_vec.push (obj);
            }
            let obj_vec = obj_vec_rev (&obj_vec);
            env.obj_stack.push (Vect::make (&obj_vec));
        }
    }
    fn vect_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        vect: Arc <Vect>,
    ) -> Arc <JoVec> {
        let sexp_list = vect_to_list (vect);
        let counter = list_size (sexp_list.dup ());
        let jojo = sexp_list_compile (
            env, static_scope, sexp_list);
        let ending_jojo = jojo! [
            CollectVectJo { counter },
        ];
        jojo_append (&jojo, &ending_jojo)
    }
    fn name_vect_to_name_vec (name_vect: Arc <Vect>) -> NameVec {
        name_vect.obj_vec .iter ()
            .map (|x| {
                let sym = Sym::cast (x.dup ());
                sym.sym.to_string ()
            })
            .collect::<NameVec> ()
    }
    fn vect_length (vect: Arc <Obj>) -> Arc <Obj> {
        let vect = Vect::cast (vect);
        Num::make (vect.obj_vec.len () as f64)
    }
    fn vect_append (
        ante: Arc <Obj>,
        succ: Arc <Obj>,
    ) -> Arc <Obj> {
        let ante = Vect::cast (ante);
        let succ = Vect::cast (succ);
        let mut ante_obj_vec = ante.obj_vec.clone ();
        let mut succ_obj_vec = succ.obj_vec.clone ();
        ante_obj_vec.append (&mut succ_obj_vec);
        Vect::make (&ante_obj_vec)
    }
    fn vect_slice (
        vect: Arc <Obj>,
        begin: Arc <Obj>,
        end: Arc <Obj>,
    ) -> Arc <Obj> {
        let vect = Vect::cast (vect);
        let begin = Num::cast (begin);
        let end = Num::cast (end);
        let begin = begin.num as usize;
        let end = end.num as usize;
        let obj_vec = ObjVec::from (&vect.obj_vec [begin..end]);
        Vect::make (&obj_vec)
    }
    fn vect_ref (
        vect: Arc <Obj>,
        index: Arc <Obj>,
    ) -> Arc <Obj> {
        let vect = Vect::cast (vect);
        let index = Num::cast (index);
        let index = index.num as usize;
        let obj = &vect.obj_vec[index];
        obj.dup ()
    }
    fn vect_head (
        vect: Arc <Obj>,
    ) -> Arc <Obj> {
        let index = Num::make (0.0);
        vect_ref (vect, index)
    }
    fn vect_rest (
        vect: Arc <Obj>,
    ) -> Arc <Obj> {
        let begin = Num::make (1.0);
        let end = vect_length (vect.dup ());
        vect_slice (vect, begin, end)
    }
    fn vect_reverse (
        vect: Arc <Obj>,
    ) -> Arc <Vect> {
        let vect = Vect::cast (vect);
        let obj_vec = obj_vec_rev (&vect.obj_vec);
        Vect::make (&obj_vec)
    }
    fn unit_vect (
        obj: Arc <Obj>,
    ) -> Arc <Obj> {
        let mut obj_vec = ObjVec::new ();
        obj_vec.push (obj);
        Vect::make (&obj_vec)
    }
    pub struct Dict {
        pub obj_dic: Arc <ObjDic>,
    }

    impl_core_type! (Dict, DICT_T);

    impl Obj for Dict {
        fn tag (&self) -> Tag { Self::tag () }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            Some (self.obj_dic.dup ())
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Dict::cast (other);
                (obj_dic_eq (&self.obj_dic, &other.obj_dic))
            }
        }
    }
    impl Dict {
        fn make (obj_dic: &ObjDic) -> Arc <Dict> {
            Arc::new (Dict { obj_dic: Arc::new (obj_dic.clone ()) })
        }
    }
    struct CollectDictJo {
        counter: usize,
    }

    impl Jo for CollectDictJo {
        fn exe (&self, env: &mut Env, _: Arc <Scope>) {
            let mut obj_dic = ObjDic::new ();
            for _ in 0..self.counter {
                // note the order!
                let obj = env.obj_stack.pop () .unwrap ();
                let key = env.obj_stack.pop () .unwrap ();
                let sym = Sym::cast (key);
                let name = sym.sym .as_str ();
                obj_dic.ins (name, Some (obj));
            }
            env.obj_stack.push (Dict::make (&obj_dic));
        }
    }
    fn dict_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        dict: Arc <Dict>,
    ) -> Arc <JoVec> {
        let sexp_list = dict_to_flat_list_reverse (dict);
        let counter = list_size (sexp_list.dup ());
        let counter = counter / 2;
        let jojo = sexp_list_compile (
            env, static_scope, sexp_list);
        let ending_jojo = jojo! [
            CollectDictJo { counter },
        ];
        jojo_append (&jojo, &ending_jojo)
    }
    pub fn dict_to_list_reverse (dict: Arc <Obj>) -> Arc <Obj> {
        let dict = Dict::cast (dict);
        let mut list = null ();
        let obj_dic = &dict.obj_dic;
        for kv in obj_dic.iter () {
            let sym = Sym::make (kv.0);
            let obj = kv.1;
            let pair = cons (sym, unit_list (obj.dup ()));
            list = cons (pair, list);
        }
        list
    }
    pub fn dict_to_list (dict: Arc <Obj>) -> Arc <Obj> {
        let dict = Dict::cast (dict);
        let list = dict_to_list_reverse (dict);
        list_reverse (list)
    }
    fn list_to_dict (mut list: Arc <Obj>) -> Arc <Dict> {
        assert! (list_p (&list));
        let mut obj_dic = ObjDic::new ();
        while ! Null::p (&list) {
           let pair = car (list.dup ());
           let sym = car_as_sym (pair.dup ());
           let name = &sym.sym;
           let obj = car (cdr (pair.dup ()));
           obj_dic.ins (name, Some (obj));
           list = cdr (list);
        }
        Dict::make (&obj_dic)
    }
    fn dict_to_flat_list_reverse (dict: Arc <Obj>) -> Arc <Obj> {
        let dict = Dict::cast (dict);
        let mut list = null ();
        for kv in dict.obj_dic.iter () {
            let key = cons (
                Sym::make ("quote"),
                unit_list (Sym::make (kv.0)));
            let obj = kv.1.dup ();
            list = cons (obj, list);
            list = cons (key, list);
        }
        list
    }
    fn dict_length (dict: Arc <Obj>) -> Arc <Num> {
        let dict = Dict::cast (dict);
        Num::make (dict.obj_dic.len () as f64)
    }
    fn dict_key_list_reverse (dict: Arc <Obj>) -> Arc <Obj> {
        let dict = Dict::cast (dict);
        let mut list = null ();
        for name in dict.obj_dic.keys () {
            let sym = Sym::make (name);
            list = cons (sym, list);
        }
        list
    }
    fn dict_value_list_reverse (dict: Arc <Obj>) -> Arc <Obj> {
        let dict = Dict::cast (dict);
        let mut list = null ();
        for value in dict.obj_dic.values () {
            list = cons (value.dup (), list);
        }
        list
    }
    fn dict_key_list (dict: Arc <Obj>) -> Arc <Obj> {
        list_reverse (dict_key_list_reverse (dict))
    }
    fn dict_value_list (dict: Arc <Obj>) -> Arc <Obj> {
        list_reverse (dict_value_list_reverse (dict))
    }
    fn dict_insert (
        dict: Arc <Obj>,
        key: Arc <Obj>,
        value: Arc <Obj>,
    ) -> Arc <Obj> {
        let dict = Dict::cast (dict);
        let key = Sym::cast (key);
        let name = &key.sym;
        let mut obj_dic = (*dict.obj_dic).clone ();
        if obj_dic.has_name (name) {
            obj_dic.set (name, Some (value));
        } else {
            obj_dic.ins (name, Some (value));
        }
        Dict::make (&obj_dic)
    }
    fn dict_merge (
        ante: Arc <Obj>,
        succ: Arc <Obj>,
    ) -> Arc <Obj> {
        let ante = Dict::cast (ante);
        let succ = Dict::cast (succ);
        let mut obj_dic = (*ante.obj_dic).clone ();
        for kv in succ.obj_dic.iter () {
            let name = kv.0;
            let value = kv.1.dup ();
            if obj_dic.has_name (name) {
                obj_dic.set (name, Some (value));
            } else {
                obj_dic.ins (name, Some (value));
            }
        }
        Dict::make (&obj_dic)
    }
    fn dict_find (
        dict: Arc <Obj>,
        key: Arc <Obj>,
    ) -> Arc <Obj> {
        let dict = Dict::cast (dict);
        let key = Sym::cast (key);
        let name = &key.sym;
        if let Some (value) = dict.obj_dic.get (name) {
            some (value.dup ())
        } else {
            none ()
        }
    }
    pub fn parse_sexp (token: &token::Token) -> Arc <Obj> {
        match token {
            token::Token::List { token_vec, .. } => parse_sexp_list (token_vec),
            token::Token::Vect { token_vec, .. } => parse_sexp_vect (token_vec),
            token::Token::Dict { token_vec, .. } => parse_sexp_dict (token_vec),
            token::Token::QuotationMark { mark_name, token, .. } =>
                cons (Sym::make (mark_name),
                        unit_list (parse_sexp (token))),
            token::Token::Num { num, .. } => Num::make (*num),
            token::Token::Str { str, .. } => Str::make (str),
            token::Token::Sym { sym, .. } => Sym::make (sym),
        }
    }
    pub fn parse_sexp_list (token_vec: &token::TokenVec) -> Arc <Obj> {
        let mut list = null ();
        token_vec
            .iter ()
            .rev ()
            .map (parse_sexp)
            .for_each (|obj| {
                list = cons (obj, list.dup ());
            });
        list
    }
    pub fn parse_sexp_vect (token_vec: &token::TokenVec) -> Arc <Obj> {
        let obj_vec = token_vec
            .iter ()
            .map (parse_sexp)
            .collect::<ObjVec> ();
        Vect::make (&obj_vec)
    }
    fn sexp_list_prefix_assign_with_last_sexp (
        sexp_list: Arc <Obj>,
        last_sexp: Arc <Obj>,
    ) -> Arc <Obj> {
        if Null::p (&sexp_list) {
            unit_list (last_sexp)
        } else {
            let head = car (sexp_list.dup ());
            if sym_sexp_as_str_p (&head, "=") {
                let next = car (cdr (sexp_list.dup ()));
                let rest = cdr (cdr (sexp_list));
                let new_last_sexp = cons (
                    head, cons (
                        last_sexp,
                        unit_list (next)));
                cons (
                    new_last_sexp,
                    sexp_list_prefix_assign (rest))
            }
            else
            {
                let rest = cdr (sexp_list);
                cons (
                    last_sexp,
                    sexp_list_prefix_assign_with_last_sexp (rest, head))
            }
        }
    }
    pub fn sexp_list_prefix_assign (sexp_list: Arc <Obj>) -> Arc <Obj> {
        if Null::p (&sexp_list) {
            sexp_list
        } else {
            sexp_list_prefix_assign_with_last_sexp (
                cdr (sexp_list.dup ()),
                car (sexp_list))
        }
    }
    pub fn parse_sexp_dict (token_vec: &token::TokenVec) -> Arc <Obj> {
        let mut sexp_list = parse_sexp_list (token_vec);
        sexp_list = sexp_list_prefix_assign (sexp_list);
        let mut obj_dic = ObjDic::new ();
        while (Cons::p (&sexp_list)) {
            let sexp = car (sexp_list.dup ());
            let name = car (cdr (sexp.dup ()));
            let name = Sym::cast (name);
            let value = car (cdr (cdr (sexp.dup ())));
            obj_dic.ins (&name.sym, Some (value));
            sexp_list = cdr (sexp_list.dup ())
        }
        Dict::make (&obj_dic)
    }
    pub fn sexp_repr (env: &Env, sexp: Arc <Obj>) -> String {
        if (Null::p (&sexp)) {
            format! ("()")
        } else if (Cons::p (&sexp)) {
            format! ("({})", sexp_list_repr (env, sexp))
        } else if (Vect::p (&sexp)) {
            let v = Vect::cast (sexp);
            let l = vect_to_list (v);
            format! ("[{}]", sexp_list_repr (env, l))
        } else if (Dict::p (&sexp)) {
            let d = Dict::cast (sexp);
            let l = dict_to_list (d);
            let v = list_to_vect (l);
            let obj_vec = v.obj_vec
                .iter ()
                .map (|x| cons (Sym::make ("="), x.dup ()))
                .collect ();
            let v = Vect::make (&obj_vec);
            let l = vect_to_list (v);
            format! ("{{{}}}", sexp_list_repr (env, l))
        } else if (Str::p (&sexp)) {
            let str = Str::cast (sexp);
            format! ("\"{}\"", str.str)
        } else if (Sym::p (&sexp)) {
            let sym = Sym::cast (sexp);
            sym.sym.clone ()
        } else {
            sexp.repr (env)
        }
    }
    pub fn sexp_list_repr (env: &Env, sexp_list: Arc <Obj>) -> String {
        if Null::p (&sexp_list) {
            format! ("")
        } else if Null::p (&cdr (sexp_list.dup ())) {
            sexp_repr (env, car (sexp_list))
        } else if (! Cons::p (&cdr (sexp_list.dup ()))) {
            format! ("{} . {}",
                     sexp_repr (env, car (sexp_list.dup ())),
                     sexp_repr (env, cdr (sexp_list)))
        } else {
            format! ("{} {}",
                     sexp_repr (env, car (sexp_list.dup ())),
                     sexp_list_repr (env, cdr (sexp_list)))
        }
    }
    fn sym_sexp_as_str_p (sexp: &Arc <Obj>, str: &str) -> bool {
        if ! Sym::p (&sexp) {
            false
        } else {
            let sym = Sym::cast (sexp.dup ());
            (sym.sym .as_str () == str)
        }
    }
    pub type KeywordFn = fn (
        env: &mut Env,
        static_scope: &StaticScope,
        body: Arc <Obj>,
    ) -> Arc <JoVec>;
    pub fn keyword_fn_eq (
        lhs: &KeywordFn,
        rhs: &KeywordFn,
    ) -> bool {
        (*lhs) as usize == (*rhs) as usize
    }
    struct Keyword {
        fun: KeywordFn,
    }

    impl_core_type! (Keyword, KEYWORD_T);

    impl Obj for Keyword {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Keyword::cast (other);
                (keyword_fn_eq (&self.fun, &other.fun))
            }
        }
    }
    impl Keyword {
        fn make (fun: KeywordFn) -> Arc <Keyword> {
            Arc::new (Keyword {
                fun,
            })
        }
    }
    fn find_keyword (
        env: &Env,
        name: &str,
    ) -> Option <Arc <Keyword>> {
        if let Some (obj) = env.find_obj (name) {
            if Keyword::p (&obj) {
                let keyword = Keyword::cast (obj.dup ());
                Some (keyword)
            } else {
                None
            }
        } else {
            None
        }
    }
    fn keyword_sexp_p (env: &Env, sexp: &Arc <Obj>) -> bool {
        if ! Cons::p (&sexp) {
            return false;
        }
        let head = car (sexp.dup ());
        if ! Sym::p (&head) {
            false
        } else {
            let sym = Sym::cast (head);
            let name = &sym.sym;
            if let Some (_) = find_keyword (env, name) {
                true
            } else {
                false
            }
        }
    }
    fn keyword_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp: Arc <Obj>,
    ) -> Arc <JoVec> {
        let sym = car_as_sym (sexp.dup ());
        let name = &sym.sym;
        let keyword = find_keyword (env, name) .unwrap ();
        let body = cdr (sexp);
        (keyword.fun) (env, static_scope, body)
    }
    impl Env {
        pub fn define_keyword (
            &mut self,
            name: &str,
            fun: KeywordFn,
        ) -> ObjId {
            self.define (name, Keyword::make (fun))
        }
    }
    struct Macro {
        obj: Arc <Obj>,
    }

    impl_core_type! (Macro, MACRO_T);

    impl Obj for Macro {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Macro::cast (other);
                (obj_eq (&self.obj, &other.obj))
            }
        }
    }
    fn find_macro (
        env: &Env,
        name: &str,
    ) -> Option <Arc <Macro>> {
        if let Some (obj) = env.find_obj (name) {
            if Macro::p (&obj) {
                let mac = Macro::cast (obj.dup ());
                Some (mac)
            } else {
                None
            }
        } else {
            None
        }
    }
    fn macro_sexp_p (env: &Env, sexp: &Arc <Obj>) -> bool {
        if ! Cons::p (&sexp) {
            return false;
        }
        let head = car (sexp.dup ());
        if ! Sym::p (&head) {
            false
        } else {
            let sym = Sym::cast (head);
            let name = &sym.sym;
            if let Some (_) = find_macro (env, name) {
                true
            } else {
                false
            }
        }
    }
    fn macro_eval (
        env: &mut Env,
        sexp: Arc <Obj>,
    ) -> Arc <Obj> {
        let sym = car_as_sym (sexp.dup ());
        let name = &sym.sym;
        let mac = find_macro (env, name) .unwrap ();
        let body = cdr (sexp);
        env.obj_stack.push (body);
        let base = env.frame_stack.len ();
        mac.obj.apply (env, 1);
        env.run_with_base (base);
        env.obj_stack.pop () .unwrap ()
    }
    fn macro_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp: Arc <Obj>,
    ) -> Arc <JoVec> {
        let new_sexp = macro_eval (env, sexp);
        sexp_compile (env, static_scope, new_sexp)
    }
    impl Env {
        pub fn define_prim_macro (
            &mut self,
            name: &str,
            fun: PrimFn,
        ) -> ObjId {
            let arg_dic = Dic::from (vec! [ "body" ]);
            let obj = Arc::new (Prim { arg_dic, fun });
            let mac = Arc::new (Macro { obj });
            self.define (name, mac)
        }
    }
    pub struct StaticRef {
        level: usize,
        index: usize,
    }
    fn static_ref_level_up (static_ref: &StaticRef) -> StaticRef {
        StaticRef {
          level: static_ref.level + 1,
          index: static_ref.index,
        }
    }
    pub type StaticScope = HashMap <Name, StaticRef>;
    fn static_scope_extend (
        old_static_scope: &StaticScope,
        name_vec: &NameVec,
    ) -> StaticScope {
        let mut static_scope: StaticScope = old_static_scope
            .iter ()
            .map (|kv| (kv.0.clone (), static_ref_level_up (kv.1)))
            .collect ();
        for (index, name) in name_vec .iter () .enumerate () {
            let static_ref = StaticRef { level: 0, index: index };
            static_scope.insert (name.clone (), static_ref);
        }
        static_scope
    }
    fn lit_compile (
        _env: &Env,
        _static_scope: &StaticScope,
        sexp: Arc <Obj>,
    ) -> Arc <JoVec> {
        jojo! [
            LitJo { obj: sexp },
        ]
    }
      fn dot_in_word_p (word: &str) -> bool {
          ((! token::str_word_p (word)) &&
           (word.find (".") != None))
      }
      fn dot_in_word_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          word: &str,
      ) -> Arc <JoVec> {
          let mut iter = word.split ('.');
          let name = iter.next () .unwrap ();
          let head_jojo = if name == "" {
              jojo! []
          } else {
              let sym = Sym::make (name);
              sym_compile (env, static_scope, sym)
          };
          let mut jo_vec = JoVec::new ();
          for name in iter {
              let jo = Arc::new (DotJo { name: String::from (name) });
              jo_vec.push (jo);
          }
          jojo_append (&head_jojo, &jo_vec)
      }
      fn ref_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          name: &str,
      ) -> Arc <JoVec> {
          if let Some (static_ref) = static_scope.get (name) {
              jojo! [
                  LocalRefJo {
                      level: static_ref.level,
                      index: static_ref.index,
                  }
              ]
          } else {
              if let Some (obj_cell) = env.obj_cell_dic.get (name) {
                  let id = Arc::downgrade (obj_cell);
                  jojo! [ RefJo { id } ]
              } else {
                  let obj_cell: ObjCell = Arc::new (Mutex::new (None));
                  let id = Arc::downgrade (&obj_cell);
                  env.obj_cell_dic.ins (name, Some (obj_cell));
                  jojo! [ RefJo { id } ]
              }
          }
      }
      fn sym_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          sym: Arc <Sym>,
      ) -> Arc <JoVec> {
          let word = &sym.sym;
          if dot_in_word_p (word) {
              dot_in_word_compile (env, static_scope, word)
          } else {
              ref_compile (env, static_scope, word)
          }
      }
      fn apply_to_arg_dict_sexp_p (
          _env: &Env,
          sexp: &Arc <Obj>,
      ) -> bool {
          if ! Cons::p (sexp) {
              return false;
          }
          let mut body = sexp_list_prefix_assign (cdr (sexp.dup ()));
          while ! Null::p (&body) {
              let head = car (body.dup ());
              if Cons::p (&head) {
                  let head_car = car (head);
                  if sym_sexp_as_str_p (&head_car, "=") {
                      return true;
                  }
              } else {
                  return false;
              }
              body = cdr (body);
          }
          return false;
      }
      fn sexp_list_assign_to_pair (sexp_list: Arc <Obj>) -> Arc <Obj> {
          if Null::p (& sexp_list) {
              sexp_list
          } else {
              cons (cdr (car (sexp_list.dup ())),
                      sexp_list_assign_to_pair (cdr (sexp_list)))
          }
      }
      fn sexp_list_to_dict (sexp_list: Arc <Obj>) -> Arc <Dict> {
          list_to_dict (
              sexp_list_assign_to_pair (
                  sexp_list_prefix_assign (sexp_list)))
      }
      pub fn apply_to_arg_dict_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          sexp: Arc <Obj>,
      ) -> Arc <JoVec> {
          let head = car (sexp.dup ());
          let body = cdr (sexp);
          let jojo = jojo! [
              ApplyToArgDictJo {},
          ];
          let head_jojo = sexp_compile (env, static_scope, head);
          let dict = sexp_list_to_dict (body);
          let body_jojo = dict_compile (env, static_scope, dict);
          let jojo = jojo_append (&head_jojo, &jojo);
          let jojo = jojo_append (&body_jojo, &jojo);
          jojo
      }
      fn dot_word_p (word: &str) -> bool {
          (word.len () >= 1 &&
           word.starts_with ("."))
      }
      fn arity_of_body (mut body: Arc <Obj>) -> usize {
          assert! (list_p (&body));
          let mut arity = 0;
          while ! Null::p (&body) {
              let head = car (body.dup ());
              if ! Sym::p (&head) {
                  arity += 1;
              } else {
                  let sym = Sym::cast (head.dup ());
                  let word = sym.sym .as_str ();
                  match word {
                      "drop" => arity -= 1,
                      "dup" | "over" | "tuck" => arity += 1,
                      "swap" => {}
                      _ if dot_word_p (word) => {}
                      _ => arity += 1,
                  }
              }
              body = cdr (body);
          }
          arity
      }
      fn apply_sexp_p (
          _env: &Env,
          sexp: &Arc <Obj>,
      ) -> bool {
          Cons::p (sexp)
      }
      pub fn apply_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          sexp: Arc <Obj>,
      ) -> Arc <JoVec> {
          let head = car (sexp.dup ());
          let body = cdr (sexp);
          let arity = arity_of_body (body.dup ());
          let jojo = jojo! [
              ApplyJo { arity },
          ];
          let head_jojo = sexp_compile (env, static_scope, head);
          let body_jojo = sexp_list_compile (env, static_scope, body);
          let jojo = jojo_append (&head_jojo, &jojo);
          let jojo = jojo_append (&body_jojo, &jojo);
          jojo
      }
    pub fn sexp_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp: Arc <Obj>,
    ) -> Arc <JoVec> {
        if Str::p (&sexp) || Num::p (&sexp) {
            lit_compile (env, static_scope, sexp)
        } else if Sym::p (&sexp) {
            let sym = Sym::cast (sexp);
            sym_compile (env, static_scope, sym)
        } else if Vect::p (&sexp) {
            let vect = Vect::cast (sexp);
            vect_compile (env, static_scope, vect)
        } else if Dict::p (&sexp) {
            let dict = Dict::cast (sexp);
            dict_compile (env, static_scope, dict)
        } else if keyword_sexp_p (env, &sexp) {
            keyword_compile (env, static_scope, sexp)
        } else if macro_sexp_p (env, &sexp) {
            macro_compile (env, static_scope, sexp)
        } else if apply_to_arg_dict_sexp_p (env, &sexp) {
            apply_to_arg_dict_compile (env, static_scope, sexp)
        } else if apply_sexp_p (env, &sexp) {
            apply_compile (env, static_scope, sexp)
        } else {
            eprintln! ("- sexp_compile");
            eprintln! ("  unknown sexp : {}", sexp_repr (env, sexp));
            panic! ("jojo fatal error!");
        }
    }
    pub fn sexp_list_compile (
        env: &mut Env,
        static_scope: &StaticScope,
        sexp_list: Arc <Obj>,
    ) -> Arc <JoVec> {
        if Null::p (&sexp_list) {
            new_jojo ()
        } else {
            assert! (Cons::p (&sexp_list));
            let head_jojo = sexp_compile (
                env, static_scope, car (sexp_list.dup ()));
            let body_jojo = sexp_list_compile (
                env, static_scope, cdr (sexp_list));
            jojo_append (&head_jojo, &body_jojo)
        }
    }
    struct Module {
        obj_cell_dic: ObjCellDic,
        obj_dic: Arc <ObjDic>,
        module_path: PathBuf,
        current_dir: PathBuf,
    }

    impl_core_type! (Module, MODULE_T);

    impl Obj for Module {
        fn tag (&self) -> Tag { Self::tag () }

        fn obj_dic (&self) -> Option <Arc <ObjDic>> {
            Some (self.obj_dic.dup ())
        }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = Module::cast (other);
                (self.module_path == other.module_path)
            }
        }
    }
    impl Module {
        pub fn make (module_env: Env) -> Arc <Module> {
            let mut obj_dic = ObjDic::new ();
            for kv in module_env.obj_cell_dic.iter () {
                let name = kv.0;
                let obj_cell = kv.1;
                let mutex_guard = obj_cell .lock () .unwrap ();
                if let Some (ref obj) = *mutex_guard {
                    obj_dic.ins (name, Some (obj.dup ()));
                }
            }
            Arc::new (Module {
                obj_cell_dic: module_env.obj_cell_dic.clone (),
                module_path: module_env.module_path.clone (),
                current_dir: module_env.current_dir.clone (),
                obj_dic: Arc::new (obj_dic),
            })
        }
    }
    // make_module
    // m_module
    pub type TopKeywordFn = fn (
        env: &mut Env,
        body: Arc <Obj>,
    );
    pub fn top_keyword_fn_eq (
        lhs: &TopKeywordFn,
        rhs: &TopKeywordFn,
    ) -> bool {
        (*lhs) as usize == (*rhs) as usize
    }
    struct TopKeyword {
        fun: TopKeywordFn,
    }

    impl_core_type! (TopKeyword, TOP_KEYWORD_T);

    impl Obj for TopKeyword {
        fn tag (&self) -> Tag { Self::tag () }

        fn eq (&self, other: Arc <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = TopKeyword::cast (other);
                (top_keyword_fn_eq (&self.fun, &other.fun))
            }
        }
    }
    impl TopKeyword {
        fn make (fun: TopKeywordFn) -> Arc <TopKeyword> {
            Arc::new (TopKeyword {
                fun,
            })
        }
    }
    fn find_top_keyword (
        env: &Env,
        name: &str,
    ) -> Option <Arc <TopKeyword>> {
        if let Some (obj) = env.find_obj (name) {
            if TopKeyword::p (&obj) {
                let top_keyword = TopKeyword::cast (obj.dup ());
                Some (top_keyword)
            } else {
                None
            }
        } else {
            None
        }
    }
    fn top_keyword_sexp_p (env: &Env, sexp: &Arc <Obj>) -> bool {
        if ! Cons::p (&sexp) {
            return false;
        }
        let head = car (sexp.dup ());
        if ! Sym::p (&head) {
            false
        } else {
            let sym = Sym::cast (head);
            let name = &sym.sym;
            if let Some (_) = find_top_keyword (env, name) {
                true
            } else {
                false
            }
        }
    }
    impl Env {
        pub fn define_top_keyword (
            &mut self,
            name: &str,
            fun: TopKeywordFn,
        ) -> ObjId {
            self.define (name, TopKeyword::make (fun))
        }
    }
    fn jojo_run (
        env: &mut Env,
        scope: &Scope,
        jojo: Arc <JoVec>,
    ) {
        let base = env.frame_stack.len ();
        let frame = Frame {
            index: 0,
            jojo,
            scope: Arc::new (scope.clone ()),
        };
        env.frame_stack.push (Box::new (frame));
        env.run_with_base (base);
    }
    fn jojo_eval (
        env: &mut Env,
        scope: &Scope,
        jojo: Arc <JoVec>,
    ) -> Arc <Obj> {
        jojo_run (env, scope, jojo);
        env.obj_stack.pop () .unwrap ()
    }
    fn jojo_run_in_new_frame (
        env: &mut Env,
        jojo: Arc <JoVec>,
    ) {
        let base = env.frame_stack.len ();
        let jo_vec = (*jojo).clone ();
        env.frame_stack.push (Frame::make (jo_vec));
        env.run_with_base (base);
    }
    fn jojo_eval_in_new_frame (
        env: &mut Env,
        jojo: Arc <JoVec>,
    ) -> Arc <Obj> {
        jojo_run_in_new_frame (env, jojo);
        env.obj_stack.pop () .unwrap ()
    }
    fn sexp_run (
        env: &mut Env,
        sexp: Arc <Obj>,
    ) {
        if top_keyword_sexp_p (env, &sexp) {
            eprintln! ("- sexp_run");
            eprintln! ("  can not handle top_keyword_sexp");
            eprintln! ("  only `top_sexp_run` can handle top_keyword_sexp");
            eprintln! ("  sexp : {}", sexp_repr (env, sexp));
            panic! ("jojo fatal error!");
        } else {
            let static_scope = StaticScope::new ();
            let jojo = sexp_compile (env, &static_scope, sexp);
            jojo_run_in_new_frame (env, jojo);
        }
    }
    fn sexp_list_run (
        env: &mut Env,
        sexp_list: Arc <Obj>,
    ) {
        if Cons::p (&sexp_list) {
            sexp_run (env, car (sexp_list.dup ()));
            sexp_list_run (env, cdr (sexp_list));
        }
    }
    fn sexp_eval (
        env: &mut Env,
        sexp: Arc <Obj>,
    ) -> Arc <Obj> {
        let size_before = env.obj_stack.len ();
        sexp_run (env, sexp.dup ());
        let size_after = env.obj_stack.len ();
        if size_after - size_before == 1 {
            env.obj_stack.pop () .unwrap ()
        } else {
            eprintln! ("- sexp_eval mismatch");
            eprintln! ("  sexp must eval to one value");
            eprintln! ("  sexp : {}", sexp_repr (env, sexp));
            eprintln! ("  stack size before : {}", size_before);
            eprintln! ("  stack size after : {}", size_after);
            panic! ("jojo fatal error!");
        }
    }
    fn top_sexp_run (
        env: &mut Env,
        sexp: Arc <Obj>,
    ) {
        if top_keyword_sexp_p (env, &sexp) {
            let sym = car_as_sym (sexp.dup ());
            let name = &sym.sym;
            let top_keyword = find_top_keyword (env, name) .unwrap ();
            let body = cdr (sexp);
            (top_keyword.fun) (env, body);
        } else {
            let static_scope = StaticScope::new ();
            let jojo = sexp_compile (env, &static_scope, sexp);
            jojo_run_in_new_frame (env, jojo);
            env.obj_stack.pop ();
        }
    }
    fn top_sexp_list_run_without_infix_assign (
        env: &mut Env,
        sexp_list: Arc <Obj>,
    ) {
        if Cons::p (&sexp_list) {
            top_sexp_run (env, car (sexp_list.dup ()));
            top_sexp_list_run_without_infix_assign (
                env, cdr (sexp_list));
        }
    }
    fn top_sexp_list_run (
        env: &mut Env,
        sexp_list: Arc <Obj>,
    ) {
        top_sexp_list_run_without_infix_assign (
            env, sexp_list_prefix_assign (sexp_list));
    }
      fn prefix_of_word (word: &str) -> String {
          let vec = word.split ('.') .collect::<Vec <&str>> ();
          if vec.len () == 1 {
              String::new ()
          } else {
              assert! (vec.len () == 2);
              vec [0] .to_string ()
          }
      }
      fn name_of_word (word: &str) -> String {
          let vec = word.split ('.') .collect::<Vec <&str>> ();
          vec [vec.len () - 1] .to_string ()
      }
      fn assign_data_p (body: &Arc <Obj>) -> bool {
          (Cons::p (&body) &&
           Sym::p (&car (body.dup ())) &&
           Cons::p (&cdr (body.dup ())) &&
           Cons::p (&car (cdr (body.dup ()))) &&
           sym_sexp_as_str_p (&car (car (cdr (body.dup ()))), "data"))
      }
      fn name_t2c (name: &str) -> String {
          let mut name = name.to_string ();
          assert! (name.ends_with ("-t"));
          name.pop ();
          name.push ('c');
          name
      }
      fn tk_assign_data (
          env: &mut Env,
          body: Arc <Obj>,
      ) {
          let sym = car_as_sym (body.dup ());
          let type_name = sym.sym.clone ();
          let data_name = name_t2c (&type_name);
          let rest = cdr (body);
          let data_body = cdr (car (rest));
          let name_vect = list_to_vect (data_body);
          let name_vec = name_vect_to_name_vec (name_vect);
          let index = env.obj_cell_dic.len ();
          let module_path = env.module_path.clone ();
          let tag = Tag { module_path, index };
          let typ_id = env.define (
              &type_name, Type::make (tag.clone ()));
          env.define (
              &data_name, DataCons::make (tag, typ_id, name_vec));
      }
      fn assign_lambda_sugar_p (body: &Arc <Obj>) -> bool {
          (Cons::p (&body) &&
           Cons::p (&car (body.dup ())))
      }
      fn assign_lambda_desugar (body: Arc <Obj>) -> Arc <Obj> {
          let head = car (body.dup ());
          let name = car (head.dup ());
          let arg_list = cdr (head);
          let rest = cdr (body);
          cons (name, unit_list (
              cons (Sym::make ("lambda"),
                      cons (list_to_vect (arg_list),
                              rest))))
      }
      fn tk_assign_value (
          env: &mut Env,
          body: Arc <Obj>,
      ) {
          let sym = car_as_sym (body.dup ());
          let name = name_of_word (&sym.sym);
          let prefix = prefix_of_word (&sym.sym);
          let rest = cdr (body);
          let rest_cdr = cdr (rest.dup ());
          assert! (Null::p (&rest_cdr));
          let sexp = car (rest);
          let obj = sexp_eval (env, sexp);
          env.assign (&prefix, &name, obj);
      }
      fn tk_assign (
          env: &mut Env,
          body: Arc <Obj>,
      ) {
          if assign_data_p (&body) {
              tk_assign_data (env, body);
          } else if assign_lambda_sugar_p (&body) {
              tk_assign_value (env, assign_lambda_desugar (body));
          } else {
              tk_assign_value (env, body);
          }
      }
      fn assign_sexp_p (sexp: &Arc <Obj>) -> bool {
          (Cons::p (sexp) &&
           sym_sexp_as_str_p (&car (sexp.dup ()), "="))
      }
      fn assign_sexp_normalize (sexp: Arc <Obj>) -> Arc <Obj> {
          let head = car (sexp.dup ());
          let body = cdr (sexp.dup ());
          if assign_lambda_sugar_p (&body) {
              cons (head, assign_lambda_desugar (body))
          } else {
              sexp
          }
      }
      fn do_body_trans (body: Arc <Obj>) -> Arc <Obj> {
          if Null::p (&body) {
              return body;
          }
          let sexp = car (body.dup ());
          let rest = cdr (body.dup ());
          if Null::p (&rest) {
              body
          } else if (assign_sexp_p (&sexp)) {
              let sexp = assign_sexp_normalize (sexp);
              let mut obj_vec = ObjVec::new ();
              obj_vec.push (cdr (sexp));
              unit_list (cons (
                  Sym::make ("let"),
                  cons (Vect::make (&obj_vec),
                        rest)))
          } else {
              cons (sexp, cons (unit_list (Sym::make ("drop")),
                                do_body_trans (rest)))
          }
      }
      fn k_do (
          env: &mut Env,
          static_scope: &StaticScope,
          mut body: Arc <Obj>,
      ) -> Arc <JoVec> {
          body = sexp_list_prefix_assign (body.dup ());
          body = do_body_trans (body.dup ());
          sexp_list_compile (env, static_scope, body)
      }
      fn k_lambda (
          env: &mut Env,
          old_static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let head = car (body.dup ());
          assert! (Vect::p (&head));
          let name_vect = Vect::cast (head);
          let name_vec = name_vect_to_name_vec (name_vect);
          let rest = cdr (body);
          let static_scope = static_scope_extend (
              old_static_scope, &name_vec);
          let jojo = sexp_compile (
              env, &static_scope, cons (Sym::make ("do"), rest));
          jojo! [
              LambdaJo {
                  arg_dic: Arc::new (Dic::from (name_vec)),
                  jojo,
              }
          ]
      }
      struct MacroMakerJo;

      impl Jo for MacroMakerJo {
          fn exe (&self, env: &mut Env, _scope: Arc <Scope>) {
              let obj = env.obj_stack.pop () .unwrap ();
              if Closure::p (&obj) {
                  let mac = Arc::new (Macro { obj });
                  env.obj_stack.push (mac);
              } else {
                  eprintln! ("- MacroMakerJo::exe");
                  eprintln! ("  obj is not closure");
                  eprintln! ("  can only make macro from closure");
                  eprintln! ("  obj : {}", obj.repr (env));
                  panic! ("jojo fatal error!");
              }
          }
      }
      fn k_macro (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let lambda_jojo = k_lambda (env, static_scope, body);
          let ending_jojo = jojo! [
              MacroMakerJo { }
          ];
          jojo_append (&lambda_jojo, &ending_jojo)
      }
      fn sexp_quote_compile (
          _env: &mut Env,
          sexp: Arc <Obj>,
      ) -> Arc <JoVec> {
          jojo! [
              LitJo { obj: sexp }
          ]
      }
      fn k_quote (
          env: &mut Env,
          _static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          assert! (Cons::p (&body));
          assert! (Null::p (&cdr (body.dup ())));
          let sexp = car (body);
          sexp_quote_compile (env, sexp)
      }
      type JojoMap = HashMap <Tag, Arc <JoVec>>;
      struct CaseJo {
          jojo_map: JojoMap,
          default_jojo: Option <Arc <JoVec>>,
      }

      impl Jo for CaseJo {
          fn exe (&self, env: &mut Env, scope: Arc <Scope>) {
              let obj = env.obj_stack.pop () .unwrap ();
              let tag = obj.tag ();
              if let Some (jojo) = self.jojo_map.get (&tag) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: jojo.dup (),
                      scope: scope.dup (),
                  }));
              } else if let Some (ref jojo) = self.default_jojo {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: jojo.dup (),
                      scope: scope.dup (),
                  }));
              } else {
                  eprintln! ("- CaseJo::exe");
                  eprintln! ("  tag mismatch");
                  eprintln! ("  tag : {}", name_of_tag (env, tag));
                  eprintln! ("  obj : {}", obj.repr (env));
                  panic! ("jojo fatal error!");
              }
          }
      }
      pub fn case_compile (
          env: &mut Env,
          static_scope: &StaticScope,
          mut body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let mut jojo_map = JojoMap::new ();
          let mut default_jojo: Option <Arc <JoVec>> = None;
          while ! Null::p (&body) {
              let clause = car (body.dup ());
              let sym = car_as_sym (clause.dup ());
              let rest = cdr (clause);
              let type_name = &sym.sym;
              if (type_name == "_") {
                  let jojo = sexp_list_compile (env, static_scope, rest);
                  default_jojo = Some (jojo);
                  body = cdr (body);
              } else {
                  if let Some (typ) = env.find_obj (type_name) {
                      let typ = Type::cast (typ);
                      let tag = typ.tag_of_type.clone ();
                      let jojo = sexp_list_compile (env, static_scope, rest);
                      jojo_map.insert (tag, jojo);
                      body = cdr (body);
                  } else {
                      eprintln! ("- case_compile");
                      eprintln! ("  unknown type_name : {}", type_name);
                      panic! ("jojo fatal error!");
                  }
              }
          }
          jojo! [
              CaseJo { jojo_map, default_jojo }
          ]
      }
      fn k_case (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let head = car (body.dup ());
          let rest = cdr (body);
          let head_jojo = sexp_compile (env, static_scope, head);
          let rest_jojo = case_compile (env, static_scope, rest);
          jojo_append (&head_jojo, &rest_jojo)
      }
      struct CollectListJo {
          counter: usize,
      }

      impl Jo for CollectListJo {
          fn exe (&self, env: &mut Env, _: Arc <Scope>) {
              let mut list = null ();
              for _ in 0..self.counter {
                  let obj = env.obj_stack.pop () .unwrap ();
                  list = cons (obj, list)
              }
              env.obj_stack.push (list);
          }
      }
      fn k_list (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let sexp_list = body;
          let counter = list_size (sexp_list.dup ());
          let jojo = sexp_list_compile (
              env, static_scope, sexp_list);
          let ending_jojo = jojo! [
              CollectListJo { counter },
          ];
          jojo_append (&jojo, &ending_jojo)
      }
      fn k_note (
          _env: &mut Env,
          _static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          jojo! [
              LitJo { obj: cons (Sym::make ("note"), body) },
          ]
      }
      struct AssertJo {
          body: Arc <Obj>,
          jojo: Arc <JoVec>,
      }

      impl Jo for AssertJo {
          fn exe (&self, env: &mut Env, scope: Arc <Scope>) {
              let base = env.frame_stack.len ();
              env.frame_stack.push (Box::new (Frame {
                  index: 0,
                  jojo: self.jojo.dup (),
                  scope: scope.dup (),
              }));
              env.run_with_base (base);
              let result = env.obj_stack.pop () .unwrap ();
              if True::p (&result) {
                  return;
              } else {
                  // env.frame_stack_report ();
                  // env.obj_stack_report ();
                  eprintln! ("- assert fail : ");
                  eprintln! ("  {} : ", sexp_list_repr (env, self.body.dup ()));
                  panic! ("jojo fatal error!");
              }
          }
      }
      fn k_assert (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let jojo = sexp_list_compile (env, &static_scope, body.dup ());
          jojo! [
              AssertJo { body, jojo }
          ]
      }
      struct IfJo {
          pred_jojo: Arc <JoVec>,
          then_jojo: Arc <JoVec>,
          else_jojo: Arc <JoVec>,
      }

      impl Jo for IfJo {
          fn exe (&self, env: &mut Env, scope: Arc <Scope>) {
              let result = jojo_eval (env, &scope, self.pred_jojo.dup ());
              if True::p (&result) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: self.then_jojo.dup (),
                      scope,
                  }));
              } else if False::p (&result) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: self.else_jojo.dup (),
                      scope,
                  }));
              } else {
                  eprintln! ("- IfJo::exe");
                  eprintln! ("  pred_jojo run to non bool value");
                  panic! ("jojo fatal error!");
              }
          }
      }
      fn k_if (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let size = list_size (body.dup ());
          assert! (size == 3);
          let pred_sexp = car (body.dup ());
          let then_sexp = car (cdr (body.dup ()));
          let else_sexp = car (cdr (cdr (body)));
          let pred_jojo = sexp_compile (env, static_scope, pred_sexp);
          let then_jojo = sexp_compile (env, static_scope, then_sexp);
          let else_jojo = sexp_compile (env, static_scope, else_sexp);
          jojo! [
              IfJo {
                  pred_jojo,
                  then_jojo,
                  else_jojo,
              }
          ]
      }
      struct WhenJo {
          pred_jojo: Arc <JoVec>,
          then_jojo: Arc <JoVec>,
      }

      impl Jo for WhenJo {
          fn exe (&self, env: &mut Env, scope: Arc <Scope>) {
              let result = jojo_eval (env, &scope, self.pred_jojo.dup ());
              if True::p (&result) {
                  env.frame_stack.push (Box::new (Frame {
                      index: 0,
                      jojo: self.then_jojo.dup (),
                      scope,
                  }));
              } else if False::p (&result) {
                  env.obj_stack.push (result);
              } else {
                  eprintln! ("- WhenJo::exe");
                  eprintln! ("  pred_jojo run to non bool value");
                  panic! ("jojo fatal error!");
              }
          }
      }
      fn k_when (
          env: &mut Env,
          static_scope: &StaticScope,
          body: Arc <Obj>,
      ) -> Arc <JoVec> {
          let size = list_size (body.dup ());
          assert! (size == 2);
          let pred_sexp = car (body.dup ());
          let then_sexp = car (cdr (body.dup ()));
          let pred_jojo = sexp_compile (env, static_scope, pred_sexp);
          let then_jojo = sexp_compile (env, static_scope, then_sexp);
          jojo! [
              WhenJo {
                  pred_jojo,
                  then_jojo,
              }
          ]
      }
      fn m_let (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let head = car (body.dup ());
          let rest = cdr (body);
          let binding_vect = vect_reverse (head);
          let mut sexp = cons (Sym::make ("do"), rest);
          for binding in &binding_vect.obj_vec {
              let name = car (binding.dup ());
              let obj_sexp = car (cdr (binding.dup ()));
              sexp = cons (
                  cons (Sym::make ("lambda"),
                        cons (unit_vect (name),
                              unit_list (sexp))),
                  unit_list (obj_sexp));
          }
          env.obj_stack.push (sexp);
      }
      fn sexp_quote_and_unquote (
          env: &Env,
          sexp: Arc <Obj>,
      ) -> Arc <Obj> {
          if Str::p (&sexp) || Num::p (&sexp) {
              sexp
          } else if Sym::p (&sexp) {
              cons (Sym::make ("quote"),
                    unit_list (sexp))
          } else if Null::p (&sexp) {
              cons (Sym::make ("quote"),
                    unit_list (sexp))
          } else if Vect::p (&sexp) {
              let list = vect_to_list (sexp);
              cons (Sym::make ("list-to-vect"),
                    unit_list (sexp_list_quote_and_unquote (env, list)))
          } else if Dict::p (&sexp) {
              let list = dict_to_list (sexp);
              cons (Sym::make ("list-to-dict"),
                    unit_list (sexp_list_quote_and_unquote (env, list)))
          } else {
              assert! (Cons::p (&sexp));
              let head = car (sexp.dup ());
              if sym_sexp_as_str_p (&head, "unquote") {
                  let rest = cdr (sexp.dup ());
                  assert! (Cons::p (&rest));
                  assert! (Null::p (&cdr (rest.dup ())));
                  car (rest)
              } else {
                  sexp_list_quote_and_unquote (
                      env,
                      sexp)
              }
          }
      }
      fn sexp_list_quote_and_unquote (
          env: &Env,
          sexp_list: Arc <Obj>,
      ) -> Arc <Obj> {
          if Null::p (&sexp_list) {
              unit_list (Sym::make ("*"))
          } else {
              assert! (Cons::p (&sexp_list));
              let mut sexp = car (sexp_list.dup ());
              if Cons::p (&sexp)
                  && sym_sexp_as_str_p (&car (sexp.dup ()),
                                        "unquote-splicing")
              {
                  let rest = cdr (sexp);
                  assert! (Cons::p (&rest));
                  assert! (Null::p (&cdr (rest.dup ())));
                  sexp = car (rest);
              } else {
                  sexp = cons (
                      Sym::make ("*"),
                      unit_list (sexp_quote_and_unquote (env, sexp)));
              }
              cons (Sym::make ("list-append"),
                    cons (
                        sexp,
                        unit_list (
                            sexp_list_quote_and_unquote (
                                env, cdr (sexp_list)))))
          }
      }
      fn m_quasiquote (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          assert! (Cons::p (&body));
          assert! (Null::p (&cdr (body.dup ())));
          let sexp = car (body);
          let new_sexp = sexp_quote_and_unquote (env, sexp);
          env.obj_stack.push (new_sexp);
      }
      fn sexp_list_and (
          env: &mut Env,
          sexp_list: Arc <Obj>,
      ) -> Arc <Obj> {
          if Null::p (&sexp_list) {
              Sym::make ("true")
          } else if Null::p (&cdr (sexp_list.dup ())) {
              car (sexp_list)
          } else {
              let head = car (sexp_list.dup ());
              let rest = cdr (sexp_list);
              cons (
                  Sym::make ("if"),
                  cons (
                      cons (Sym::make ("not"), unit_list (head)),
                      cons (
                          Sym::make ("false"),
                          unit_list (sexp_list_and (env, rest)))))
          }
      }
      fn m_and (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let sexp = sexp_list_and (env, body);
          env.obj_stack.push (sexp);
      }
      fn sexp_list_or (
          env: &mut Env,
          sexp_list: Arc <Obj>,
      ) -> Arc <Obj> {
          if Null::p (&sexp_list) {
              Sym::make ("false")
          } else if Null::p (&cdr (sexp_list.dup ())) {
              car (sexp_list)
          } else {
              let head = car (sexp_list.dup ());
              let rest = cdr (sexp_list);
              cons (
                  Sym::make ("if"),
                  cons (
                      head,
                      cons (
                          Sym::make ("true"),
                          unit_list (sexp_list_or (env, rest)))))
          }
      }
      fn m_or (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let sexp = sexp_list_or (env, body);
          env.obj_stack.push (sexp);
      }
      fn vect_list_cond (
          env: &mut Env,
          vect_list: Arc <Obj>,
      ) -> Arc <Obj> {
          assert! (! Null::p (&vect_list));
          let head = car (vect_list.dup ());
          let rest = cdr (vect_list);
          let list = vect_to_list (head);
          let question = car (list.dup ());
          let answer = cons (Sym::make ("do"), cdr (list));
          if (Null::p (&rest)) {
              if (sym_sexp_as_str_p (&question, "else")) {
                  answer
              } else {
                  let result = null ();
                  let result = cons (answer, result);
                  let result = cons (question, result);
                  let result = cons (Sym::make ("when"), result);
                  result
              }
          } else {
              let result = unit_list (vect_list_cond (env, rest));
              let result = cons (answer, result);
              let result = cons (question, result);
              let result = cons (Sym::make ("if"), result);
              result
          }
      }
      fn m_cond (
          env: &mut Env,
          arg: &ObjDic,
      ) {
          let body = arg_idx (arg, 0);
          let sexp = vect_list_cond (env, body);
          env.obj_stack.push (sexp);
      }
    fn arg_idx (arg_dic: &ObjDic, index: usize) -> Arc <Obj> {
        let entry = arg_dic.idx (index);
        if let Some (value) = &entry.value {
            value.dup ()
        } else {
            eprintln! ("- arg_idx");
            eprintln! ("  unknown index : {}", index);
            panic! ("jojo fatal error!");
        }
    }
    fn expose_type (env: &mut Env) {
        env.define_prim ("type-of", vec! ["obj"], |env, arg| {
            env.obj_stack.push (type_of (env, arg_idx (arg, 0)));
        });
    }
    fn expose_bool (env: &mut Env) {
        env.define ("true", True::make ());
        env.define ("false", False::make ());;
        define_prim! (env, "not", ["x"], not);;
    }
    fn expose_num (env: &mut Env) {
        define_prim! (env, "inc", ["x"], inc);
        define_prim! (env, "dec", ["x"], dec);
        define_prim! (env, "add", ["x", "y"], add);
        define_prim! (env, "sub", ["x", "y"], sub);
        define_prim! (env, "mul", ["x", "y"], mul);
        define_prim! (env, "div", ["x", "y"], div);
        define_prim! (env, "mod", ["x", "y"], num_mod);
        define_prim! (env, "max", ["x", "y"], max);
        define_prim! (env, "min", ["x", "y"], min);
        define_prim! (env, "abs", ["x"], abs);
        define_prim! (env, "neg", ["x"], neg);
        define_prim! (env, "pow", ["x", "y"], pow);
        define_prim! (env, "mul-add", ["x", "y", "z"], mul_add);
        define_prim! (env, "reciprocal", ["x"], reciprocal);
        define_prim! (env, "sqrt", ["x"], sqrt);
        define_prim! (env, "cbrt", ["x"], cbrt);
        define_prim! (env, "hypot", ["x", "y"], hypot);

        define_prim! (env, "lt", ["x", "y"], lt);
        define_prim! (env, "gt", ["x", "y"], gt);
        define_prim! (env, "lteq", ["x", "y"], lteq);
        define_prim! (env, "gteq", ["x", "y"], gteq);
        define_prim! (env, "even-p", ["x"], even_p);
        define_prim! (env, "odd-p", ["x"], odd_p);

        define_prim! (env, "num-integer-part", ["x"], num_integer_part);
        define_prim! (env, "num-fractional-part", ["x"], num_fractional_part);
        define_prim! (env, "num-sign", ["x"], num_sign);
        define_prim! (env, "num-floor", ["x"], num_floor);
        define_prim! (env, "num-ceil", ["x"], num_ceil);
        define_prim! (env, "num-round", ["x"], num_round);

        define_prim! (env, "num-sin", ["x"], num_sin);
        define_prim! (env, "num-cos", ["x"], num_cos);
        define_prim! (env, "num-tan", ["x"], num_tan);
        define_prim! (env, "num-asin", ["x"], num_asin);
        define_prim! (env, "num-acos", ["x"], num_acos);
        define_prim! (env, "num-atan", ["x"], num_atan);
        define_prim! (env, "num-atan2", ["x", "y"], num_atan2);
        define_prim! (env, "num-sinh", ["x"], num_sinh);
        define_prim! (env, "num-cosh", ["x"], num_cosh);
        define_prim! (env, "num-tanh", ["x"], num_tanh);
        define_prim! (env, "num-asinh", ["x"], num_asinh);
        define_prim! (env, "num-acosh", ["x"], num_acosh);
        define_prim! (env, "num-atanh", ["x"], num_atanh);

        define_prim! (env, "num-exp", ["x"], num_exp);
        define_prim! (env, "num-exp2", ["x"], num_exp2);
        define_prim! (env, "num-exp-m1", ["x"], num_exp_m1);
        define_prim! (env, "num-ln", ["x"], num_ln);
        define_prim! (env, "num-ln-1p", ["x"], num_ln_1p);
        define_prim! (env, "num-log", ["x", "y"], num_log);
        define_prim! (env, "num-log2", ["x"], num_log2);
        define_prim! (env, "num-log10", ["x"], num_log10);
    }
    fn expose_str (env: &mut Env) {
        define_prim! (env, "str-length", ["str"], str_length);
        define_prim! (env, "str-append", ["ante", "succ"], str_append);
        define_prim! (env, "str-slice", ["str", "begin", "end"], str_slice);
        define_prim! (env, "str-ref", ["str", "index"], str_ref);
        define_prim! (env, "str-head", ["str"], str_head);
        define_prim! (env, "str-rest", ["str"], str_rest);
    }
    fn expose_sym (env: &mut Env) {
        define_prim! (env, "sym-length", ["sym"], sym_length);
        define_prim! (env, "sym-append", ["ante", "succ"], sym_append);
        define_prim! (env, "sym-slice", ["sym", "begin", "end"], sym_slice);
        define_prim! (env, "sym-ref", ["sym", "index"], sym_ref);
        define_prim! (env, "sym-head", ["sym"], sym_head);
        define_prim! (env, "sym-rest", ["sym"], sym_rest);
    }
    fn expose_list (env: &mut Env) {
        env.define ("null", null ());
        define_prim! (env, "cons", ["car", "cdr"], cons);
        define_prim! (env, "car", ["pair"], car);
        define_prim! (env, "cdr", ["pair"], cdr);
        define_prim! (env, "list-length", ["list"], list_length);
        define_prim! (env, "list-reverse", ["list"], list_reverse);
        define_prim! (env, "list-reverse-append", ["ante", "succ"], list_reverse_append);
        define_prim! (env, "list-append", ["ante", "succ"], list_append);
        define_prim! (env, "unit-list", ["obj"], unit_list);
    }
    fn expose_vect (env: &mut Env) {
        define_prim! (env, "list-to-vect", ["list"], list_to_vect);
        define_prim! (env, "vect-to-list", ["vect"], vect_to_list);
        define_prim! (env, "vect-length", ["vect"], vect_length);
        define_prim! (env, "vect-append", ["ante", "succ"], vect_append);
        define_prim! (env, "vect-slice", ["vect", "begin", "end"], vect_slice);
        define_prim! (env, "vect-ref", ["vect", "index"], vect_ref);
        define_prim! (env, "vect-head", ["vect"], vect_head);
        define_prim! (env, "vect-rest", ["vect"], vect_rest);
        define_prim! (env, "vect-reverse", ["vect"], vect_reverse);
        define_prim! (env, "unit-vect", ["obj"], unit_vect);
    }
    fn expose_option (env: &mut Env) {
        env.define ("none", JNone::make ());
        define_prim! (env, "some", ["value"], some);
    }
    fn expose_dict (env: &mut Env) {
        define_prim! (env, "list-to-dict", ["list"], list_to_dict);
        define_prim! (env, "dict-to-list", ["dict"], dict_to_list);
        define_prim! (env, "dict-to-list-reverse", ["dict"], dict_to_list_reverse);
        define_prim! (env, "dict-length", ["dict"], dict_length);
        define_prim! (env, "dict-key-list-reverse", ["dict"], dict_key_list_reverse);
        define_prim! (env, "dict-key-list", ["dict"], dict_key_list);
        define_prim! (env, "dict-value-list-reverse", ["dict"], dict_value_list_reverse);
        define_prim! (env, "dict-value-list", ["dict"], dict_value_list);
        define_prim! (env, "dict-insert", ["dict", "key", "value"], dict_insert);
        define_prim! (env, "dict-merge", ["ante", "succ"], dict_merge);
        define_prim! (env, "dict-find", ["dict", "key"], dict_find);
    }
    fn expose_stack (env: &mut Env) {
        env.define_prim ("drop", vec! [], |env, _| {
            env.obj_stack.pop ();
        });
    }
    fn expose_syntax (env: &mut Env) {
        env.define_top_keyword ("=", tk_assign);
        env.define_keyword ("do", k_do);
        env.define_keyword ("lambda", k_lambda);
        env.define_keyword ("macro", k_macro);
        env.define_keyword ("case", k_case);
        env.define_keyword ("quote", k_quote);
        env.define_keyword ("*", k_list);
        env.define_keyword ("note", k_note);
        env.define_keyword ("assert", k_assert);
        env.define_keyword ("if", k_if);
        env.define_keyword ("when", k_when);
        env.define_prim_macro ("let", m_let);
        env.define_prim_macro ("quasiquote", m_quasiquote);
        env.define_prim_macro ("and", m_and);
        env.define_prim_macro ("or", m_or);
        env.define_prim_macro ("cond", m_cond);
    }
    fn expose_module (env: &mut Env) {
        env.define_prim ("import", vec! ["path"], |env, arg| {
            let path = Str::cast (arg_idx (arg, 0));
            let path = Path::new (&path.str);
            let module_path = respect_module_path (env, &path);
            let module_env = Env::from_module_path (&module_path);
            let module = Module::make (module_env);
            env.obj_stack.push (module);
        });
    }
    fn expose_misc (env: &mut Env) {
        env.define_prim ("repr", vec! ["obj"], |env, arg| {
            let obj = arg_idx (arg, 0);
            obj.print (env);
            env.obj_stack.push (obj);
        });
        env.define_prim ("print", vec! ["obj"], |env, arg| {
            let obj = arg_idx (arg, 0);
            obj.print (env);
            env.obj_stack.push (obj);
        });
        env.define_prim ("println", vec! ["obj"], |env, arg| {
            let obj = arg_idx (arg, 0);
            obj.print (env);
            println! ("");
            env.obj_stack.push (obj);
        });
        env.define_prim ("eq", vec! ["lhs", "rhs"], |env, arg| {
            let lhs = arg_idx (arg, 0);
            let rhs = arg_idx (arg, 1);
            env.obj_stack.push (make_bool (obj_eq (&lhs, &rhs)));
        });
    }
    fn expose_core (env: &mut Env) {
        expose_type (env);
        expose_bool (env);
        expose_num (env);
        expose_str (env);
        expose_sym (env);
        expose_list (env);
        expose_vect (env);
        expose_option (env);
        expose_dict (env);
        // expose_sexp (env);
        // expose_top_keyword (env);
        // expose_keyword (env);
        // expose_system (env);
        expose_module (env);
        expose_syntax (env);
        expose_stack (env);
        expose_misc (env);
    }
    fn code_run (
        env: &mut Env,
        code: &str,
    ) {
        let token_vec = token::scan (code);
        let sexp_list = parse_sexp_list (&token_vec);
        top_sexp_list_run (env, sexp_list);
    }
    fn code_from_module_path (
        module_path: &Path,
    ) -> String {
        let code = fs::read_to_string (module_path)
            .expect ("fail to read file");
        code
    }
    fn respect_current_dir (
        env: &Env,
        path: &Path,
    ) -> PathBuf {
        if path.is_absolute () {
            path.to_path_buf ()
        } else {
            let mut path_buf = env.current_dir.clone ();
            path_buf.push (path);
            path_buf
        }
    }
    fn respect_module_path (
        env: &Env,
        path: &Path,
    ) -> PathBuf {
        if path.is_absolute () {
            path.to_path_buf ()
        } else {
            let mut path_buf = env.module_path.clone ();
            path_buf.pop ();
            path_buf.push (path);
            path_buf
        }
    }
    impl Env {
        pub fn from_module_path (module_path: &Path) -> Env {
            let mut env = Env::new ();
            env.module_path = respect_current_dir (&env, module_path);
            expose_core (&mut env);
            let code = code_from_module_path (&env.module_path);
            code_run (&mut env, &code);
            env
        }
    }
    fn assert_pop (env: &mut Env, obj: Arc <Obj>) {
        let pop = env.obj_stack.pop () .unwrap ();
        assert! (obj_eq (&obj, &pop));
    }
    #[test]
    fn test_step () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));

        env.frame_stack.push (frame! [
            RefJo { id: world.clone () },
            RefJo { id: bye.clone () },
            RefJo { id: world.clone () },
        ]);

        env.run ();

        assert_eq! (3, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_apply () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));

        env.frame_stack.push (frame! [
            RefJo { id: bye.clone () },
            RefJo { id: world.clone () },
            LambdaJo { arg_dic: Arc::new (Dic::from (vec! [ "x", "y" ])),
                       jojo: jojo! [
                           LocalRefJo { level: 0, index: 1 },
                           LocalRefJo { level: 0, index: 0 },
                       ] },
            ApplyJo { arity: 2 },
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye.clone () },
            RefJo { id: world.clone () },
            LambdaJo { arg_dic: Arc::new (Dic::from (vec! [ "x", "y" ])),
                       jojo: jojo! [
                           LocalRefJo { level: 0, index: 1 },
                           LocalRefJo { level: 0, index: 0 },
                       ] },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 },
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_data () {
        let mut env = Env::new ();

        let last_cry = env.define (
            "last-cry",
            cons (Str::make ("bye"),
                    Str::make ("world")));

        env.frame_stack.push (frame! [
            RefJo { id: last_cry.clone () },
            DotJo { name: String::from ("cdr") },
            RefJo { id: last_cry.clone () },
            DotJo { name: String::from ("car") },
            RefJo { id: last_cry.clone () },
        ]);

        env.run ();
        assert_eq! (3, env.obj_stack.len ());
        assert_pop (&mut env,
                    cons (Str::make ("bye"),
                          Str::make ("world")));
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_data_cons () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));
        let tag = Tag {
            module_path: PathBuf::new (),
            index: env.obj_cell_dic.len (),
        };
        let typ_id = env.define (
            "cont-t", Type::make (tag));
        let cons = env.define (
            "cons-c", DataCons::make (Cons::tag (), typ_id, vec! [
                String::from ("car"),
                String::from ("cdr"),
            ]));

        env.frame_stack.push (frame! [
            RefJo { id: bye.clone () },
            RefJo { id: world.clone () },
            RefJo { id: cons.clone () },
            ApplyJo { arity: 2 },
            DotJo { name: String::from ("car") },
        ]);

        env.run ();
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye.clone () },
            RefJo { id: world.clone () },
            RefJo { id: cons.clone () },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 },
            DotJo { name: String::from ("car") }
        ]);

        env.run ();
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_prim () {
        let mut env = Env::new ();

        let bye = env.define (
            "bye", Str::make ("bye"));
        let world = env.define (
            "world", Str::make ("world"));
        let swap = env.define (
            "swap", Arc::new (Prim {
                arg_dic: Dic::from (vec! [ "x", "y" ]),
                fun: |env, arg_dic| {
                    let x = arg_dic.get ("x") .unwrap () .dup ();
                    let y = arg_dic.get ("y") .unwrap () .dup ();
                    env.obj_stack.push (y);
                    env.obj_stack.push (x);
                },
            }));

        env.frame_stack.push (frame! [
            RefJo { id: bye.clone () },
            RefJo { id: world.clone () },
            RefJo { id: swap.clone () },
            ApplyJo { arity: 2 }
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (0, env.obj_stack.len ());

        // curry

        env.frame_stack.push (frame! [
            RefJo { id: bye.clone () },
            RefJo { id: world.clone () },
            RefJo { id: swap.clone () },
            ApplyJo { arity: 1 },
            ApplyJo { arity: 1 }
        ]);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("world"));
        assert_eq! (1, env.obj_stack.len ());
        assert_pop (&mut env, Str::make ("bye"));
        assert_eq! (0, env.obj_stack.len ());
    }
    #[test]
    fn test_sexp () {
        fn pr (code: &str) -> String {
            // parse and repr
            let env = Env::new ();
            let token_vec = token::scan (code);
            let sexp_list = parse_sexp_list (&token_vec);
            sexp_list_repr (&env, sexp_list)
        }

        fn pr_eq (lhs: &str, rhs: &str) -> bool {
            let lhs_pr = pr (lhs);
            rhs == &lhs_pr
        }

        fn pr_same (code: &str) -> bool {
            pr_eq (code, code)
        }

        assert! (pr_same ("()"));
        assert! (pr_same ("[]"));
        assert! (pr_same ("{}"));
        assert! (pr_same ("a b c"));
        assert! (pr_same ("a (b) c"));
        assert! (pr_same ("(a (b) c)"));
        assert! (pr_same ("[a b c]"));
        assert! (pr_same ("a [b] c"));
        assert! (pr_same ("[a [b] c]"));
        assert! (pr_same ("{(= a 1) (= b 2) (= c 3)}"));
        assert! (pr_eq ("{a = 1, b = 2, c = 3}",
                        "{(= a 1) (= b 2) (= c 3)}"));

        println! ("\n<test_sexp>"); {
            fn p (code: &str) {
                let env = Env::new ();
                let token_vec = token::scan (code);
                let sexp_list = parse_sexp_list (&token_vec);
                println! ("{} -> {}",
                          code,
                          sexp_list_repr (&env, sexp_list));
            };
            p ("()");
            p ("[]");
            p ("{}");
            p ("a b c");
            p ("a (b) c");
            p ("(a (b) c)");
            p ("[a b c]");
            p ("a [b] c");
            p ("[a [b] c]");
            p ("{(= a 1) (= b 2) (= c 3)}");
        }
        println! ("</test_sexp>");
    }
