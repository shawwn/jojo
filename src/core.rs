    use std::rc::Rc;
    use dic::Dic;
    // use scan::scan_word_vec;
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
      fn obj_dic_eq (
          lhs: &ObjDic,
          rhs: &ObjDic,
      ) -> bool {
          false
          // (lhs.len () == rhs.len () &&
          //  lhs.iter ()
          //  .zip (rhs.iter ())
          //  .all (|p| obj_dic_eq (p.0, p.1)))
      }
      pub fn local_scope_extend (
          local_scope: Ptr <LocalScope>,
          obj_dic: ObjDic,
      ) -> Ptr <LocalScope> {
          let mut obj_dic_vec = (*local_scope).clone ();
          obj_dic_vec.push (obj_dic);
          Ptr::new (obj_dic_vec)
      }
      fn local_scope_eq (
          lhs: &LocalScope,
          rhs: &LocalScope,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter ()
           .zip (rhs.iter ())
           .all (|p| obj_dic_eq (p.0, p.1)))
      }
      fn jojo_eq (
          lhs: &JoVec,
          rhs: &JoVec,
      ) -> bool {
          (lhs.len () == rhs.len () &&
           lhs.iter ()
           .zip (rhs.iter ())
           .all (|p| jo_eq (p.0.clone (), p.1.clone ())))
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
          let index = env.type_dic.ins (name, Some (Type::make (tag)));
          assert_eq! (tag, index);
      }
      pub const CLOSURE_T      : Tag = 0;
      pub const TYPE_T         : Tag = 1;
      pub const TRUE_T         : Tag = 2;
      pub const FALSE_T        : Tag = 3;
      pub const DATA_CONS_T    : Tag = 4;
      pub const PRIM_T         : Tag = 5;
      pub const NUM_T          : Tag = 6;
      pub const STR_T          : Tag = 7;
      pub const SYM_T          : Tag = 8;
      pub const NULL_T         : Tag = 9;
      pub const CONS_T         : Tag = 10;
      pub const VECT_T         : Tag = 11;
      pub const DICT_T         : Tag = 12;
      pub const MODULE_T       : Tag = 13;
      pub const KEYWORD_T      : Tag = 14;
      pub const MACRO_T        : Tag = 15;
      pub const TOP_KEYWORD_T  : Tag = 16;
      pub const NOTHING_T      : Tag = 17;
      pub const JUST_T         : Tag = 18;
      fn init_type_dic (env: &mut Env) {
          preserve_tag (env, CLOSURE_T      , "closure-t");
          preserve_tag (env, TYPE_T         , "type-t");
          preserve_tag (env, TRUE_T         , "true-t");
          preserve_tag (env, FALSE_T        , "false-t");
          preserve_tag (env, DATA_CONS_T    , "data-cons-t");
          preserve_tag (env, PRIM_T         , "prim-t");
          preserve_tag (env, NUM_T          , "num-t");
          preserve_tag (env, STR_T          , "str-t");
          preserve_tag (env, SYM_T          , "sym-t");
          preserve_tag (env, NULL_T         , "null-t");
          preserve_tag (env, CONS_T         , "cons-t");
          preserve_tag (env, VECT_T         , "vect-t");
          preserve_tag (env, DICT_T         , "dict-t");
          preserve_tag (env, MODULE_T       , "module-t");
          preserve_tag (env, KEYWORD_T      , "keyword-t");
          preserve_tag (env, MACRO_T        , "macro-t");
          preserve_tag (env, TOP_KEYWORD_T  , "top-keyword-t");
          preserve_tag (env, NOTHING_T      , "nothing-t");
          preserve_tag (env, JUST_T         , "just-t");
      }
    pub trait Obj {
        fn tag (&self) -> Tag;
        fn obj_dic (&self) -> Option <&ObjDic> { None }

        fn eq (&self, other: Ptr <Obj>) -> bool;

        fn get (&self, name: &str) -> Option <Ptr <Obj>> {
            if let Some (obj_dic) = self.obj_dic () {
                if let Some (obj) = obj_dic.get (name) {
                    Some (obj.clone ())
                } else {
                    None
                }
            } else {
                None
            }
        }

        fn repr (&self, env: &Env) -> String {
            format! ("#<{}>", name_of_tag (&env, self.tag ()))
        }

        fn print (&self, env: &Env) {
            println! ("{}", self.repr (&env));
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
    fn obj_to <T: Obj> (obj: Ptr <Obj>) -> Ptr <T> {
        let obj_ptr = Ptr::into_raw (obj);
        unsafe {
            let obj_ptr = obj_ptr as *const Obj as *const T;
            Ptr::from_raw (obj_ptr)
        }
    }
    fn obj_eq (
        lhs: Ptr <Obj>,
        rhs: Ptr <Obj>,
    ) -> bool {
        lhs.eq (rhs)
    }
    pub trait Jo {
        fn exe (&self, env: &mut Env, local_scope: Ptr <LocalScope>);

        fn repr (&self, _env: &Env) -> String {
            "#<unknown-jo>".to_string ()
        }
    }
    fn jo_eq (
        lhs: Ptr <Jo>,
        rhs: Ptr <Jo>,
    ) -> bool {
        let lhs_ptr = Ptr::into_raw (lhs);
        let rhs_ptr = Ptr::into_raw (rhs);
        lhs_ptr == rhs_ptr
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
                let index = frame.index;
                let jo = frame.jojo [frame.index] .clone ();
                frame.index += 1;
                if index + 1 < frame.jojo.len () {
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
    pub struct Frame {
        pub index: usize,
        pub jojo: Ptr <JoVec>,
        pub local_scope: Ptr <LocalScope>,
    }

    impl Frame {
        fn make (jo_vec: JoVec) -> Box <Frame> {
            Box::new (Frame {
                index: 0,
                jojo: Ptr::new (jo_vec),
                local_scope: Ptr::new (LocalScope::new ()),
            })
        }
    }
    pub struct Type {
        method_dic: ObjDic,
        tag_of_type: Tag,
        super_tag_vec: TagVec,
    }

    impl Type {
       fn make (tag: Tag) -> Ptr <Type> {
          Ptr::new (Type {
             method_dic: ObjDic::new (),
             tag_of_type: tag,
             super_tag_vec: TagVec::new (),
          })
       }
    }
    impl Obj for Type {
        fn tag (&self) -> Tag { TYPE_T }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.method_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Type> (other);
                (self.tag_of_type == other.tag_of_type &&
                 self.super_tag_vec == other.super_tag_vec)
            }
        }
    }
    pub struct Data {
        tag_of_type: Tag,
        field_dic: ObjDic,
    }

    impl Data {
        fn make (
            tag: Tag,
            vec: Vec <(&str, Ptr <Obj>)>,
        ) -> Ptr <Data> {
            Ptr::new (Data {
                tag_of_type: tag,
                field_dic: Dic::from (vec),
            })
        }
    }
    impl Data {
        fn unit (tag: Tag) -> Ptr <Data> {
            Ptr::new (Data {
                tag_of_type: tag,
                field_dic: ObjDic::new (),
            })
        }
    }
    impl Obj for Data {
        fn tag (&self) -> Tag { self.tag_of_type }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.field_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Data> (other);
                (self.tag_of_type == other.tag_of_type &&
                 obj_dic_eq (&self.field_dic, &other.field_dic))
            }
        }
    }
    pub struct DataCons {
        tag_of_type: Tag,
        field_dic: ObjDic,
    }

    impl DataCons {
        // [TODO]
        // DataCons::make (CONS_T, ["car", "cdr"])
        //     .set ("car", car)
        //     .set ("cdr", cdr)
        //     .to_data ()
    }
    impl Obj for DataCons {
        fn tag (&self) -> Tag { DATA_CONS_T }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.field_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<DataCons> (other);
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
            let tag_of_type = self.tag_of_type;
            let field_dic = obj_dic_pick_up (
                env, &self.field_dic, arity);
            if arity == lack {
                env.obj_stack.push (Ptr::new (Data {
                    tag_of_type,
                    field_dic,
                }));
            } else {
                env.obj_stack.push (Ptr::new (DataCons {
                    tag_of_type,
                    field_dic,
                }));
            }
        }
    }
    pub struct Closure {
        arg_dic: ObjDic,
        jojo: Ptr <JoVec>,
        local_scope: Ptr <LocalScope>,
    }
    impl Obj for Closure {
        fn tag (&self) -> Tag { CLOSURE_T }
        fn obj_dic (&self) -> Option <&ObjDic> { Some (&self.arg_dic) }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Closure> (other);
                (jojo_eq (&self.jojo, &other.jojo) &&
                 local_scope_eq (&self.local_scope, &other.local_scope) &&
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
            let jojo = self.jojo.clone ();
            let arg_dic = obj_dic_pick_up (env, &self.arg_dic, arity);
            let local_scope = self.local_scope.clone ();
            if arity == lack {
                env.frame_stack.push (Box::new (Frame {
                    index: 0,
                    jojo,
                    local_scope: local_scope_extend (
                        local_scope, arg_dic),
                }));
            } else {
                env.obj_stack.push (Ptr::new (Closure {
                    arg_dic,
                    jojo,
                    local_scope,
                }));
            }
        }
    }
    pub type PrimFn = fn (env: &mut Env, arg_dic: &ObjDic);
    fn prim_fn_eq (
        lhs: &PrimFn,
        rhs: &PrimFn,
    ) -> bool {
        (*lhs) as usize == (*rhs) as usize
    }
    pub struct Prim {
        arg_dic: ObjDic,
        fun: PrimFn,
    }
    impl Obj for Prim {
        fn tag (&self) -> Tag { PRIM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Prim> (other);
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
                env.obj_stack.push (Ptr::new (Prim {
                    arg_dic,
                    fun,
                }));
            }
        }
    }
    pub fn true_c () -> Ptr <Data> {
        Data::unit (TRUE_T)
    }
    pub fn false_c () -> Ptr <Data> {
        Data::unit (FALSE_T)
    }
    pub fn make_bool (b: bool) -> Ptr <Data> {
        if b {
            true_c ()
        }
        else {
            false_c ()
        }
    }
    pub struct Str (pub String);
    impl Obj for Str {
        fn tag (&self) -> Tag { STR_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Str> (other);
                (self.0 == other.0)
            }
        }
    }
    pub struct Sym (pub String);
    impl Obj for Sym {
        fn tag (&self) -> Tag { SYM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Sym> (other);
                (self.0 == other.0)
            }
        }
    }
    pub struct Num (pub f64);
    impl Obj for Num {
        fn tag (&self) -> Tag { NUM_T }

        fn eq (&self, other: Ptr <Obj>) -> bool {
            if self.tag () != other.tag () {
                false
            } else {
                let other = obj_to::<Num> (other);
                (self.0 == other.0)
            }
        }
    }
    pub fn null_c () -> Ptr <Data> {
       Data::unit (NULL_T)
    }
    pub fn cons_c (car: Ptr <Obj>, cdr: Ptr <Obj>) -> Ptr <Data> {
        Data::make (CONS_T, vec! [
            ("car", car),
            ("cdr", cdr),
        ])
    }
    fn car (cons: Ptr <Data>) -> Ptr <Obj> {
        assert_eq! (CONS_T, cons.tag ());
        cons.get ("car") .unwrap ()
    }
    fn cdr (cons: Ptr <Data>) -> Ptr <Obj> {
        assert_eq! (CONS_T, cons.tag ());
        cons.get ("cdr") .unwrap ()
    }
    fn list_p (x: Ptr <Obj>) -> bool {
        let tag = x.tag ();
        (NULL_T == tag ||
         CONS_T == tag)
    }
    fn unit_list (obj: Ptr <Obj>) -> Ptr <Obj> {
        cons_c (obj, null_c ())
    }
    #[test]
    fn test_step () {
        let mut env = Env::new ();

        let id = env.define (
            "s1", Ptr::new (Str ("bye".to_string ())));

        let jo_vec: JoVec = vec! [
            Ptr::new (RefJo {id}),
            Ptr::new (RefJo {id}),
        ];

        let frame = Frame::make (jo_vec);
        env.frame_stack.push (frame);

        env.run ();
        assert_eq! (2, env.obj_stack.len ());
        assert_eq! (
            "#<str-t>",
            env.obj_stack.pop ()
                .unwrap ()
                .repr (&env));
        assert_eq! (1, env.obj_stack.len ());
        assert_eq! (
            "#<str-t>",
            env.obj_stack.pop ()
                .unwrap ()
                .repr (&env));
        assert_eq! (0, env.obj_stack.len ());
    }
