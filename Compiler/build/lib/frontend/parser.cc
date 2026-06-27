// A Bison parser, made by GNU Bison 3.8.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2021 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "parser.tab.hh"


// Unqualified %code blocks.
#line 47 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"

    namespace fdmj 
    {
        template<typename RHS>
        void calcLocation(location_t &current, const RHS &rhs, const std::size_t n);
    }
    
    #define YYLLOC_DEFAULT(Cur, Rhs, N) calcLocation(Cur, Rhs, N)
    #define yylex lexer.yylex
    Pos *p;

#line 58 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YY_USE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

#line 25 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
namespace fdmj {
#line 151 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"

  /// Build a parser object.
  ASTParser::ASTParser (ASTLexer &lexer_yyarg, const bool debug_yyarg, AST_YYSTYPE* result_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      lexer (lexer_yyarg),
      debug (debug_yyarg),
      result (result_yyarg)
  {}

  ASTParser::~ASTParser ()
  {}

  ASTParser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------.
  | symbol.  |
  `---------*/

  // basic_symbol.
  template <typename Base>
  ASTParser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  ASTParser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  ASTParser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (value_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}


  template <typename Base>
  ASTParser::symbol_kind_type
  ASTParser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  template <typename Base>
  bool
  ASTParser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  ASTParser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_kind.
  ASTParser::by_kind::by_kind () YY_NOEXCEPT
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  ASTParser::by_kind::by_kind (by_kind&& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  ASTParser::by_kind::by_kind (const by_kind& that) YY_NOEXCEPT
    : kind_ (that.kind_)
  {}

  ASTParser::by_kind::by_kind (token_kind_type t) YY_NOEXCEPT
    : kind_ (yytranslate_ (t))
  {}



  void
  ASTParser::by_kind::clear () YY_NOEXCEPT
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  ASTParser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  ASTParser::symbol_kind_type
  ASTParser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }


  ASTParser::symbol_kind_type
  ASTParser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }



  // by_state.
  ASTParser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  ASTParser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  ASTParser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  ASTParser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  ASTParser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  ASTParser::symbol_kind_type
  ASTParser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  ASTParser::stack_symbol_type::stack_symbol_type ()
  {}

  ASTParser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  ASTParser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  ASTParser::stack_symbol_type&
  ASTParser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }

  ASTParser::stack_symbol_type&
  ASTParser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  ASTParser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    YY_USE (yysym.kind ());
  }

#if YYDEBUG
  template <typename Base>
  void
  ASTParser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YY_USE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YY_USE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  ASTParser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  ASTParser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  ASTParser::yypop_ (int n) YY_NOEXCEPT
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  ASTParser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  ASTParser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  ASTParser::debug_level_type
  ASTParser::debug_level () const
  {
    return yydebug_;
  }

  void
  ASTParser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  ASTParser::state_type
  ASTParser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  ASTParser::yy_pact_value_is_default_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  ASTParser::yy_table_value_is_error_ (int yyvalue) YY_NOEXCEPT
  {
    return yyvalue == yytable_ninf_;
  }

  int
  ASTParser::operator() ()
  {
    return parse ();
  }

  int
  ASTParser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    // User initialization code.
#line 41 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
{
    #if YYDEBUG != 0
        set_debug_level(debug);
    #endif
}

#line 499 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // PROG: MAINMETHOD CLASSDECLLIST
#line 104 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  { 
#ifdef DEBUG
    cerr << "Program" << endl;
#endif
    result->root = new Program(p, (yystack_[1].value.mainMethod), (yystack_[0].value.classDeclList));
    //$$ = result->root;
  }
#line 643 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 3: // MAINMETHOD: PUBLIC INT MAIN '(' ')' '{' VARDECLLIST STMLIST '}'
#line 113 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "MainMethod" << endl;
#endif
    (yylhs.value.mainMethod) = new MainMethod(p, (yystack_[2].value.varDeclList), (yystack_[1].value.stmList)) ;
  }
#line 654 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 4: // TYPE: CLASS ID
#line 121 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Type CLASS ID" << endl;
#endif
    (yylhs.value.type) = new Type(p, (yystack_[0].value.idExp));
  }
#line 665 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 5: // TYPE: INT
#line 129 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Type INT" << endl;
#endif
    (yylhs.value.type) = new Type(p);
  }
#line 676 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 6: // TYPE: INT '[' ']'
#line 137 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Type INT[]" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.type) = new Type(p, new IntExp(p1, 0)); // 0 means array without arity
  }
#line 688 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 7: // TYPE: INT '[' NUMBER ']'
#line 146 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Type [number]" << endl;
#endif
    (yylhs.value.type) = new Type(p, (yystack_[1].value.intExp));
  }
#line 699 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 8: // VARDECLLIST: %empty
#line 154 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "VARDECLLIST emtpy" << endl;  
#endif
    (yylhs.value.varDeclList) = new vector<VarDecl*>();
  }
#line 710 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 9: // VARDECLLIST: VARDECL VARDECLLIST
#line 162 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "VARDECL VARDECLLIST" << endl;
#endif
    vector<VarDecl*> *v = (yystack_[0].value.varDeclList);
    v->push_back((yystack_[1].value.varDecl));
    rotate(v->begin(), v->end()-1, v->end());
    (yylhs.value.varDeclList) = v;
  }
#line 724 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 10: // VARDECL: TYPE ID ';'
#line 173 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "VarDecl TYPE ID" << endl;
#endif
    (yylhs.value.varDecl) = new VarDecl(p, (yystack_[2].value.type), (yystack_[1].value.idExp));
  }
#line 735 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 11: // VARDECL: TYPE ID '=' '{' NUMBERLIST '}' ';'
#line 181 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "VarDecl TYPE ID = {NUMBERLIST}" << endl;
#endif
    (yylhs.value.varDecl) = new VarDecl(p, (yystack_[6].value.type), (yystack_[5].value.idExp), (yystack_[2].value.intExpList)); 
  }
#line 746 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 12: // NUMBER: NONNEGATIVEINT
#line 189 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Number: NONNEGATIVEINT: " << (yystack_[0].value.i) << endl;
#endif
    (yylhs.value.intExp) = new IntExp(p, (yystack_[0].value.i));
  }
#line 757 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 13: // NUMBER: MINUS NONNEGATIVEINT
#line 197 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Number: -NONNEGATIVEINT: " << -(yystack_[0].value.i) <<  endl;  
#endif
    (yylhs.value.intExp) = new IntExp(p, -(yystack_[0].value.i));
  }
#line 768 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 14: // NUMBERLIST: %empty
#line 205 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NUMBERLIST empty" << endl;
#endif
    (yylhs.value.intExpList) = new vector<IntExp*>();
  }
#line 779 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 15: // NUMBERLIST: NUMBER NUMBERREST
#line 213 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NUMBER NUMBERREST" << endl;
#endif
    vector<IntExp*> *v = (yystack_[0].value.intExpList);
    v->push_back((yystack_[1].value.intExp));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.intExpList) = v;
  }
#line 793 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 16: // NUMBERREST: %empty
#line 224 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NUMBERREST empty" << endl;
#endif
    (yylhs.value.intExpList) = new vector<IntExp*>();
  }
#line 804 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 17: // NUMBERREST: ',' NUMBER NUMBERREST
#line 232 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NUMBER NUMBERREST" << endl;
#endif
    vector<IntExp*> *v = (yystack_[0].value.intExpList);
    v->push_back((yystack_[1].value.intExp));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.intExpList) = v;
  }
#line 818 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 18: // CLASSDECLLIST: %empty
#line 243 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "CLASSDECLLIST empty" << endl;
#endif
    (yylhs.value.classDeclList) = new vector<ClassDecl*>();
  }
#line 829 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 19: // CLASSDECLLIST: CLASSDECL CLASSDECLLIST
#line 251 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "CLASSDECL CLASSDECLLIST" << endl;
#endif
    vector<ClassDecl*> *v = (yystack_[0].value.classDeclList);
    v->push_back((yystack_[1].value.classDecl));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.classDeclList) = v;
  }
#line 843 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 20: // CLASSDECL: PUBLIC CLASS ID '{' VARDECLLIST METHODDECLLIST '}'
#line 262 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "ClassDecl: public class id { vlist, mlist }" << endl;
#endif
    (yylhs.value.classDecl) = new ClassDecl(p, (yystack_[4].value.idExp), (yystack_[2].value.varDeclList), (yystack_[1].value.methodDeclList));
  }
#line 854 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 21: // CLASSDECL: PUBLIC CLASS ID EXTENDS ID '{' VARDECLLIST METHODDECLLIST '}'
#line 270 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "ClassDecl: public class id extends id { vlist, mlist }" << endl;
#endif
    (yylhs.value.classDecl) = new ClassDecl(p, (yystack_[6].value.idExp), (yystack_[4].value.idExp), (yystack_[2].value.varDeclList), (yystack_[1].value.methodDeclList));
  }
#line 865 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 22: // METHODDECLLIST: %empty
#line 278 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "METHODDECLLIST empty" << endl;
#endif
    (yylhs.value.methodDeclList) = new vector<MethodDecl*>();
  }
#line 876 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 23: // METHODDECLLIST: METHODDECL METHODDECLLIST
#line 286 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "METHODDECL METHODDECLLIST" << endl;
#endif
    vector<MethodDecl*> *v = (yystack_[0].value.methodDeclList);
    v->push_back((yystack_[1].value.methodDecl));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.methodDeclList) = v;
  }
#line 890 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 24: // METHODDECL: PUBLIC TYPE ID '(' FORMALLIST ')' '{' VARDECLLIST STMLIST '}'
#line 297 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "MethodDecl: public type id (formallist) { vlist, slist }" << endl;
#endif
    (yylhs.value.methodDecl) = new MethodDecl(p, (yystack_[8].value.type), (yystack_[7].value.idExp), (yystack_[5].value.formalList), (yystack_[2].value.varDeclList), (yystack_[1].value.stmList));
  }
#line 901 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 25: // FORMALLIST: %empty
#line 305 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef  DEBUG
    cerr << "FORMALLIST empty" << endl;
#endif
    (yylhs.value.formalList) = new vector<Formal*>();
  }
#line 912 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 26: // FORMALLIST: TYPE ID FORMALREST
#line 313 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "TYPE ID FORMALREST" << endl;
#endif
    vector<Formal*> *v = (yystack_[0].value.formalList);
    Pos *p1 = new Pos(yystack_[2].location.sline, yystack_[2].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    v->push_back(new Formal(p1, (yystack_[2].value.type), (yystack_[1].value.idExp)));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.formalList) = v;
  }
#line 927 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 27: // FORMALREST: %empty
#line 325 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "FORMALREST empty" << endl;
#endif
    (yylhs.value.formalList) = new vector<Formal*>();
  }
#line 938 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 28: // FORMALREST: ',' TYPE ID FORMALREST
#line 333 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "',' TYPE ID FORMALREST" << endl;
#endif
    vector<Formal*> *v = (yystack_[0].value.formalList);
    Pos *p1 = new Pos(yystack_[2].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    v->push_back(new Formal(p1, (yystack_[2].value.type), (yystack_[1].value.idExp)));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.formalList) = v;
  }
#line 953 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 29: // STMLIST: %empty
#line 345 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "STMLIST empty" << endl;
#endif
    (yylhs.value.stmList) = new vector<Stm*>();
  }
#line 964 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 30: // STMLIST: STM STMLIST
#line 353 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "STM STMLIST" << endl;
#endif
    vector<Stm*> *v = (yystack_[0].value.stmList);
    v->push_back((yystack_[1].value.stm));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.stmList) = v;
  }
#line 978 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 31: // STM: '{' STMLIST '}'
#line 364 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "{ sl }" << endl;
#endif
    (yylhs.value.stm) = new Nested(p, (yystack_[1].value.stmList));
  }
#line 989 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 32: // STM: IF '(' EXP ')' STM ELSE STM
#line 372 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "If else" << endl;
#endif
    (yylhs.value.stm) = new If(p, (yystack_[4].value.exp), (yystack_[2].value.stm), (yystack_[0].value.stm));
  }
#line 1000 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 33: // STM: IF '(' EXP ')' STM
#line 380 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "If no else" << endl;
#endif
    (yylhs.value.stm) = new If(p, (yystack_[2].value.exp), (yystack_[0].value.stm));
  }
#line 1011 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 34: // STM: WHILE '(' EXP ')' STM
#line 388 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "While stm" << endl;
#endif
    (yylhs.value.stm) = new While(p, (yystack_[2].value.exp), (yystack_[0].value.stm));
  }
#line 1022 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 35: // STM: WHILE '(' EXP ')' ';'
#line 396 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "While no stm" << endl;
#endif
    (yylhs.value.stm) = new While(p, (yystack_[2].value.exp));
  }
#line 1033 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 36: // STM: EXP '=' EXP ';'
#line 404 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Assign" << endl;
#endif
    (yylhs.value.stm) = new Assign(p, (yystack_[3].value.exp), (yystack_[1].value.exp));
  }
#line 1044 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 37: // STM: EXP '.' ID '(' EXPLIST ')' ';'
#line 412 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "CallStm" << endl;
#endif
    (yylhs.value.stm) = new CallStm(p, (yystack_[6].value.exp), (yystack_[4].value.idExp), (yystack_[2].value.expList));
  }
#line 1055 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 38: // STM: CONTINUE ';'
#line 420 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Continue" << endl;
#endif
    (yylhs.value.stm) = new Continue(p);
  }
#line 1066 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 39: // STM: BREAK ';'
#line 428 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Break" << endl;
#endif
    (yylhs.value.stm) = new Break(p);
  }
#line 1077 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 40: // STM: RETURN EXP ';'
#line 436 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Return" << endl;
#endif
    (yylhs.value.stm) = new Return(p, (yystack_[1].value.exp));
  }
#line 1088 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 41: // STM: PUTINT '(' EXP ')' ';'
#line 444 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "PutInt" << endl;
#endif
    (yylhs.value.stm) = new PutInt(p, (yystack_[2].value.exp));
  }
#line 1099 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 42: // STM: PUTCH '(' EXP ')' ';'
#line 452 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "PutCh" << endl;
#endif
    (yylhs.value.stm) = new PutCh(p, (yystack_[2].value.exp));
  }
#line 1110 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 43: // STM: PUTARRAY '(' EXP ',' EXP ')' ';'
#line 460 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "PutArray" << endl;
#endif
    (yylhs.value.stm) = new PutArray(p, (yystack_[4].value.exp), (yystack_[2].value.exp));
  }
#line 1121 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 44: // STM: STARTTIME '(' ')' ';'
#line 468 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "StartTime" << endl;
#endif
    (yylhs.value.stm) = new Starttime(p);
  }
#line 1132 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 45: // STM: STOPTIME '(' ')' ';'
#line 476 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "StopTime" << endl;
#endif
    (yylhs.value.stm) = new Stoptime(p);
  }
#line 1143 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 46: // EXP: EXP ADD EXP
#line 484 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP ADD EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "+"), (yystack_[0].value.exp));
  }
#line 1155 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 47: // EXP: EXP MINUS EXP
#line 493 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP MINUS EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "-"), (yystack_[0].value.exp));
  }
#line 1167 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 48: // EXP: EXP TIMES EXP
#line 502 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP TIMES EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "*"), (yystack_[0].value.exp));
  }
#line 1179 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 49: // EXP: EXP DIVIDE EXP
#line 511 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP DIVIDE EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "/"), (yystack_[0].value.exp));
  }
#line 1191 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 50: // EXP: EXP OR EXP
#line 520 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP OR EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "||"), (yystack_[0].value.exp));
  }
#line 1203 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 51: // EXP: EXP AND EXP
#line 529 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP AND EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "&&"), (yystack_[0].value.exp));
  }
#line 1215 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 52: // EXP: EXP LT EXP
#line 538 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP LT EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "<"), (yystack_[0].value.exp));
  }
#line 1227 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 53: // EXP: EXP LE EXP
#line 547 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP LE EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "<="), (yystack_[0].value.exp));
  }
#line 1239 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 54: // EXP: EXP GT EXP
#line 556 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP GT EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, ">"), (yystack_[0].value.exp));
  }
#line 1251 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 55: // EXP: EXP GE EXP
#line 565 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP GE EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, ">="), (yystack_[0].value.exp));
  }
#line 1263 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 56: // EXP: EXP EQ EXP
#line 574 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP EQ EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "=="), (yystack_[0].value.exp));
  }
#line 1275 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 57: // EXP: EXP NE EXP
#line 583 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP NE EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new BinaryOp(p, (yystack_[2].value.exp), new OpExp(p1, "!="), (yystack_[0].value.exp));
  }
#line 1287 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 58: // EXP: EXP '[' EXP ']'
#line 592 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP [ EXP ]" << endl;
#endif
    (yylhs.value.exp) = new ArrayExp(p, (yystack_[3].value.exp), (yystack_[1].value.exp));
  }
#line 1298 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 59: // EXP: EXP '.' ID '(' EXPLIST ')'
#line 600 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP . ID ( EXPLIST )" << endl;
#endif
    (yylhs.value.exp) = new CallExp(p, (yystack_[5].value.exp), (yystack_[3].value.idExp), (yystack_[1].value.expList));
  }
#line 1309 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 60: // EXP: EXP '.' ID
#line 608 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP . ID" << endl;
#endif
    (yylhs.value.exp) = new ClassVar(p, (yystack_[2].value.exp), (yystack_[0].value.idExp));
  }
#line 1320 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 61: // EXP: NONNEGATIVEINT
#line 616 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NonNegativeInt: " << (yystack_[0].value.i) << endl;
#endif
    (yylhs.value.exp) = new IntExp(p, (yystack_[0].value.i));
  }
#line 1331 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 62: // EXP: LENGTH '(' EXP ')'
#line 624 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Length" << endl;
#endif
    (yylhs.value.exp) = new Length(p, (yystack_[1].value.exp));
  }
#line 1342 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 63: // EXP: THIS
#line 632 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "This" << endl;
#endif
    (yylhs.value.exp) = new This(p);
  }
#line 1353 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 64: // EXP: '!' EXP
#line 640 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "! EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) = new UnaryOp(p, new OpExp(p1, "!"), (yystack_[0].value.exp));
  }
#line 1365 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 65: // EXP: MINUS EXP
#line 649 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "- EXP" << endl;
#endif
    Pos *p1 = new Pos(yystack_[1].location.sline, yystack_[1].location.scolumn, yystack_[1].location.eline, yystack_[1].location.ecolumn);
    (yylhs.value.exp) =  new UnaryOp(p, new OpExp(p1, "-"), (yystack_[0].value.exp));
  }
#line 1377 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 66: // EXP: '(' EXP ')'
#line 658 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "( EXP )" << endl;
#endif
    (yylhs.value.exp) = (yystack_[1].value.exp);
  }
#line 1388 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 67: // EXP: NEW INT '[' EXP ']'
#line 666 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NEW INT [ EXP ]" << endl;
#endif
    (yylhs.value.exp) = new NewArray(p, (yystack_[1].value.exp));
  }
#line 1399 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 68: // EXP: NEW ID '(' ')'
#line 674 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "NEW ID ( )" << endl;
#endif
    (yylhs.value.exp) = new NewObject(p, (yystack_[2].value.idExp));
  }
#line 1410 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 69: // EXP: GETINT '(' ')'
#line 682 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "GetInt" << endl;
#endif
    (yylhs.value.exp) = new GetInt(p);
  }
#line 1421 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 70: // EXP: GETCH '(' ')'
#line 690 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "GetCh" << endl;
#endif
    (yylhs.value.exp) = new GetCh(p);
  }
#line 1432 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 71: // EXP: GETARRAY '(' EXP ')'
#line 698 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "GetArray" << endl;
#endif
    (yylhs.value.exp) = new GetArray(p, (yystack_[1].value.exp));
  }
#line 1443 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 72: // EXP: ID
#line 706 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "ID" << endl;
#endif
    (yylhs.value.exp) = (yystack_[0].value.idExp);
  }
#line 1454 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 73: // EXPLIST: %empty
#line 714 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXPLIST empty" << endl;
#endif
    (yylhs.value.expList) = new vector<Exp*>();
  }
#line 1465 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 74: // EXPLIST: EXP EXPREST
#line 722 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP EXPREST" << endl;
#endif
    vector<Exp*> *v = (yystack_[0].value.expList);
    v->push_back((yystack_[1].value.exp));
        rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.expList) = v;
  }
#line 1479 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 75: // EXPREST: %empty
#line 733 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXPREST empty" << endl;
#endif
    (yylhs.value.expList) = new vector<Exp*>();
  }
#line 1490 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 76: // EXPREST: ',' EXP EXPREST
#line 741 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "EXP EXPREST" << endl;
#endif
    vector<Exp*> *v = (yystack_[0].value.expList);
    v->push_back((yystack_[1].value.exp));
    rotate(v->begin(), v->end() - 1, v->end());
    (yylhs.value.expList) = v;
  }
#line 1504 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;

  case 77: // ID: IDENTIFIER
#line 752 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
  {
#ifdef DEBUG
    cerr << "Identifier: " << (yystack_[0].value.s) << endl;
#endif
    (yylhs.value.idExp) = new IdExp(p, (yystack_[0].value.s));
  }
#line 1515 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"
    break;


#line 1519 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        context yyctx (*this, yyla);
        std::string msg = yysyntax_error_ (yyctx);
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  ASTParser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  const char *
  ASTParser::symbol_name (symbol_kind_type yysymbol)
  {
    static const char *const yy_sname[] =
    {
    "end of file", "error", "invalid token", "PUBLIC", "INT", "MAIN",
  "CLASS", "EXTENDS", "IF", "ELSE", "WHILE", "CONTINUE", "BREAK", "RETURN",
  "PUTINT", "PUTCH", "PUTARRAY", "STARTTIME", "STOPTIME", "THIS", "LENGTH",
  "NEW", "GETINT", "GETCH", "GETARRAY", "NONNEGATIVEINT", "IDENTIFIER",
  "'('", "')'", "'['", "']'", "'{'", "'}'", "'='", "','", "';'", "'.'",
  "ADD", "MINUS", "TIMES", "DIVIDE", "EQ", "NE", "LT", "LE", "GT", "GE",
  "AND", "OR", "'!'", "UMINUS", "$accept", "PROG", "MAINMETHOD", "TYPE",
  "VARDECLLIST", "VARDECL", "NUMBER", "NUMBERLIST", "NUMBERREST",
  "CLASSDECLLIST", "CLASSDECL", "METHODDECLLIST", "METHODDECL",
  "FORMALLIST", "FORMALREST", "STMLIST", "STM", "EXP", "EXPLIST",
  "EXPREST", "ID", YY_NULLPTR
    };
    return yy_sname[yysymbol];
  }



  // ASTParser::context.
  ASTParser::context::context (const ASTParser& yyparser, const symbol_type& yyla)
    : yyparser_ (yyparser)
    , yyla_ (yyla)
  {}

  int
  ASTParser::context::expected_tokens (symbol_kind_type yyarg[], int yyargn) const
  {
    // Actual number of expected tokens
    int yycount = 0;

    const int yyn = yypact_[+yyparser_.yystack_[0].state];
    if (!yy_pact_value_is_default_ (yyn))
      {
        /* Start YYX at -YYN if negative to avoid negative indexes in
           YYCHECK.  In other words, skip the first -YYN actions for
           this state because they are default actions.  */
        const int yyxbegin = yyn < 0 ? -yyn : 0;
        // Stay within bounds of both yycheck and yytname.
        const int yychecklim = yylast_ - yyn + 1;
        const int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
        for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
          if (yycheck_[yyx + yyn] == yyx && yyx != symbol_kind::S_YYerror
              && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
            {
              if (!yyarg)
                ++yycount;
              else if (yycount == yyargn)
                return 0;
              else
                yyarg[yycount++] = YY_CAST (symbol_kind_type, yyx);
            }
      }

    if (yyarg && yycount == 0 && 0 < yyargn)
      yyarg[0] = symbol_kind::S_YYEMPTY;
    return yycount;
  }






  int
  ASTParser::yy_syntax_error_arguments_ (const context& yyctx,
                                                 symbol_kind_type yyarg[], int yyargn) const
  {
    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state merging
         (from LALR or IELR) and default reductions corrupt the expected
         token list.  However, the list is correct for canonical LR with
         one exception: it will still contain any token that will not be
         accepted due to an error action in a later state.
    */

    if (!yyctx.lookahead ().empty ())
      {
        if (yyarg)
          yyarg[0] = yyctx.token ();
        int yyn = yyctx.expected_tokens (yyarg ? yyarg + 1 : yyarg, yyargn - 1);
        return yyn + 1;
      }
    return 0;
  }

  // Generate an error message.
  std::string
  ASTParser::yysyntax_error_ (const context& yyctx) const
  {
    // Its maximum.
    enum { YYARGS_MAX = 5 };
    // Arguments of yyformat.
    symbol_kind_type yyarg[YYARGS_MAX];
    int yycount = yy_syntax_error_arguments_ (yyctx, yyarg, YYARGS_MAX);

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
      default: // Avoid compiler warnings.
        YYCASE_ (0, YY_("syntax error"));
        YYCASE_ (1, YY_("syntax error, unexpected %s"));
        YYCASE_ (2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_ (3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_ (4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_ (5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    std::ptrdiff_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += symbol_name (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const short ASTParser::yypact_ninf_ = -134;

  const signed char ASTParser::yytable_ninf_ = -1;

  const short
  ASTParser::yypact_[] =
  {
      13,    15,    22,    23,    29,  -134,    21,  -134,    23,    10,
      12,  -134,    14,  -134,    -2,    16,    12,    37,    37,    20,
      24,    12,    12,    52,    37,   217,    37,    -5,  -134,    17,
      37,    26,    52,  -134,    32,    33,    28,    41,    74,    51,
      53,    54,    55,    62,  -134,    63,     6,    65,    75,    76,
    -134,    74,   217,    74,    74,    47,   217,   377,  -134,    52,
    -134,  -134,    80,    78,    79,  -134,    12,  -134,  -134,    74,
      74,  -134,  -134,   397,    74,    74,    74,    81,    83,    74,
      85,    88,    91,    92,    74,   105,    89,    -1,    -1,  -134,
    -134,    74,    74,    12,    74,    74,    74,    74,    74,    74,
      74,    74,    74,    74,    74,    74,    93,  -134,  -134,    -7,
      97,   231,   252,  -134,    12,   273,   294,   417,    72,    94,
     315,    74,    98,  -134,  -134,   336,  -134,  -134,   437,   457,
     100,    77,    77,    -1,    -1,   536,   536,    48,    48,    48,
      48,   138,   517,  -134,    96,   103,    37,   217,   177,   101,
     102,   104,    74,  -134,  -134,  -134,   477,  -134,  -134,  -134,
    -134,    74,    -7,  -134,   121,    12,   108,   123,  -134,  -134,
      74,  -134,  -134,   357,  -134,   497,   110,    96,  -134,   125,
     126,   217,   133,   127,    74,  -134,   128,  -134,    37,  -134,
      37,  -134,  -134,  -134,   497,  -134,    12,   217,  -134,   125,
     132,  -134,  -134
  };

  const signed char
  ASTParser::yydefact_[] =
  {
       0,     0,     0,    18,     0,     1,     0,     2,    18,     0,
       0,    19,     0,    77,     0,     0,     0,     8,     8,     0,
       5,     0,     0,    22,     8,    29,     8,     0,     4,     0,
       0,     0,    22,     9,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    63,     0,     0,     0,     0,     0,
      61,     0,    29,     0,     0,     0,    29,     0,    72,    22,
      12,     6,     0,     0,     0,    10,     0,    20,    23,     0,
       0,    38,    39,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    65,    64,     3,
      30,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    13,     7,    14,
       0,     0,     0,    40,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    69,    70,     0,    66,    31,     0,     0,
      60,    46,    47,    48,    49,    56,    57,    52,    53,    54,
      55,    51,    50,    21,    16,     0,    25,     0,     0,    60,
       0,     0,     0,    44,    45,    62,     0,    68,    71,    58,
      36,    73,     0,    15,     0,     0,     0,    33,    35,    34,
      73,    41,    42,     0,    67,    75,     0,    16,    11,    27,
       0,     0,     0,     0,     0,    74,    59,    17,     0,    26,
       8,    32,    59,    43,    75,    37,     0,    29,    76,    27,
       0,    28,    24
  };

  const short
  ASTParser::yypgoto_[] =
  {
    -134,  -134,  -134,   -28,   -17,  -134,  -105,  -134,   -12,   158,
    -134,   -29,  -134,  -134,   -31,   -39,  -133,   -30,     0,   -25,
     -10
  };

  const unsigned char
  ASTParser::yydefgoto_[] =
  {
       0,     2,     3,    22,    23,    24,    63,   145,   163,     7,
       8,    31,    32,   166,   189,    55,    56,    57,   176,   185,
      58
  };

  const unsigned char
  ASTParser::yytable_[] =
  {
      14,    25,    66,    68,   144,    16,    19,    33,    73,    59,
      80,    28,    29,    86,   167,   169,     1,    90,    60,     4,
      60,    85,     5,    87,    88,    61,     6,    10,    91,    17,
     106,    62,    13,    62,     9,   114,    81,    12,    13,   111,
     112,    20,    15,    21,   115,   116,   117,    18,   191,   120,
      64,    26,    65,    27,   125,    30,   110,   177,    67,    69,
      70,   128,   129,    71,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,    72,    91,    74,    89,
      75,    76,    77,   130,   114,    94,    95,    96,    97,    78,
      79,   156,    82,    44,    45,    46,    47,    48,    49,    50,
      13,    51,    83,    84,   149,   107,    91,   153,   108,   118,
     109,   119,    53,   114,   121,   122,    96,    97,   165,   123,
     124,   127,   173,    54,   146,   143,   157,   161,   170,   154,
     162,   175,   181,   126,    91,   164,   180,   171,   186,   172,
     175,   114,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   194,   179,   178,   190,   200,   188,
     196,   192,   193,   195,   202,   187,    11,    91,   201,   198,
     182,     0,     0,   197,   114,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,    34,   199,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    13,    51,     0,     0,     0,    52,     0,
       0,     0,   168,     0,     0,    53,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    34,    54,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    13,    51,     0,     0,     0,    52,     0,
       0,     0,     0,     0,     0,    53,     0,     0,     0,   147,
      91,     0,     0,     0,     0,     0,    54,   114,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     148,    91,     0,     0,     0,     0,     0,     0,   114,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   150,    91,     0,     0,     0,     0,     0,     0,   114,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   151,    91,     0,     0,     0,     0,     0,     0,
     114,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   155,    91,     0,     0,     0,     0,     0,
       0,   114,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   158,    91,     0,     0,     0,     0,
       0,     0,   114,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   183,    91,     0,     0,     0,
       0,     0,     0,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,     0,     0,     0,
      92,     0,     0,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,     0,     0,     0,
       0,     0,   113,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,     0,     0,     0,
       0,   152,     0,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,   159,     0,     0,
       0,     0,     0,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,     0,     0,     0,
       0,     0,   160,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,   174,     0,     0,
       0,     0,     0,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,     0,     0,     0,
       0,   184,     0,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    91,     0,     0,     0,
       0,     0,     0,   114,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,    91,     0,     0,     0,     0,
       0,     0,   114,    94,    95,    96,    97,     0,     0,   100,
     101,   102,   103
  };

  const short
  ASTParser::yycheck_[] =
  {
      10,    18,    30,    32,   109,     7,    16,    24,    38,    26,
       4,    21,    22,    52,   147,   148,     3,    56,    25,     4,
      25,    51,     0,    53,    54,    30,     3,     6,    29,    31,
      59,    38,    26,    38,     5,    36,    46,    27,    26,    69,
      70,     4,    28,     6,    74,    75,    76,    31,   181,    79,
      33,    31,    35,    29,    84,     3,    66,   162,    32,    27,
      27,    91,    92,    35,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,    35,    29,    27,    32,
      27,    27,    27,    93,    36,    37,    38,    39,    40,    27,
      27,   121,    27,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    27,    27,   114,    25,    29,    35,    30,    28,
      31,    28,    38,    36,    29,    27,    39,    40,   146,    28,
      28,    32,   152,    49,    27,    32,    28,    27,    27,    35,
      34,   161,     9,    28,    29,    32,    28,    35,    28,    35,
     170,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,   184,   165,    35,    31,   197,    34,
     188,    28,    35,    35,    32,   177,     8,    29,   199,   194,
     170,    -1,    -1,   190,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,     8,   196,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    35,    -1,    -1,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     8,    49,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    -1,    -1,    -1,    31,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    -1,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    -1,    49,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    28,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    -1,    -1,    -1,
      33,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    -1,    -1,    -1,
      -1,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    30,    -1,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    -1,    -1,    -1,
      -1,    -1,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    30,    -1,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    -1,    -1,    -1,
      -1,    34,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    29,    -1,    -1,    -1,    -1,
      -1,    -1,    36,    37,    38,    39,    40,    -1,    -1,    43,
      44,    45,    46
  };

  const signed char
  ASTParser::yystos_[] =
  {
       0,     3,    52,    53,     4,     0,     3,    60,    61,     5,
       6,    60,    27,    26,    71,    28,     7,    31,    31,    71,
       4,     6,    54,    55,    56,    55,    31,    29,    71,    71,
       3,    62,    63,    55,     8,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    27,    31,    38,    49,    66,    67,    68,    71,    55,
      25,    30,    38,    57,    33,    35,    54,    32,    62,    27,
      27,    35,    35,    68,    27,    27,    27,    27,    27,    27,
       4,    71,    27,    27,    27,    68,    66,    68,    68,    32,
      66,    29,    33,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    62,    25,    30,    31,
      71,    68,    68,    35,    36,    68,    68,    68,    28,    28,
      68,    29,    27,    28,    28,    68,    28,    32,    68,    68,
      71,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    32,    57,    58,    27,    28,    28,    71,
      28,    28,    34,    35,    35,    28,    68,    28,    28,    30,
      35,    27,    34,    59,    32,    54,    64,    67,    35,    67,
      27,    35,    35,    68,    30,    68,    69,    57,    35,    71,
      28,     9,    69,    28,    34,    70,    28,    59,    34,    65,
      31,    67,    28,    35,    68,    35,    54,    55,    70,    71,
      66,    65,    32
  };

  const signed char
  ASTParser::yyr1_[] =
  {
       0,    51,    52,    53,    54,    54,    54,    54,    55,    55,
      56,    56,    57,    57,    58,    58,    59,    59,    60,    60,
      61,    61,    62,    62,    63,    64,    64,    65,    65,    66,
      66,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
      68,    68,    68,    69,    69,    70,    70,    71
  };

  const signed char
  ASTParser::yyr2_[] =
  {
       0,     2,     2,     9,     2,     1,     3,     4,     0,     2,
       3,     7,     1,     2,     0,     2,     0,     3,     0,     2,
       7,     9,     0,     2,    10,     0,     3,     0,     4,     0,
       2,     3,     7,     5,     5,     5,     4,     7,     2,     2,
       3,     5,     5,     7,     4,     4,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     4,     6,
       3,     1,     4,     1,     2,     2,     3,     5,     4,     3,
       3,     4,     1,     0,     2,     0,     3,     1
  };




#if YYDEBUG
  const short
  ASTParser::yyrline_[] =
  {
       0,   103,   103,   112,   120,   128,   136,   145,   154,   161,
     172,   180,   188,   196,   205,   212,   224,   231,   243,   250,
     261,   269,   278,   285,   296,   305,   312,   325,   332,   345,
     352,   363,   371,   379,   387,   395,   403,   411,   419,   427,
     435,   443,   451,   459,   467,   475,   483,   492,   501,   510,
     519,   528,   537,   546,   555,   564,   573,   582,   591,   599,
     607,   615,   623,   631,   639,   648,   657,   665,   673,   681,
     689,   697,   705,   714,   721,   733,   740,   751
  };

  void
  ASTParser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  ASTParser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  ASTParser::symbol_kind_type
  ASTParser::yytranslate_ (int t) YY_NOEXCEPT
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    49,     2,     2,     2,     2,     2,     2,
      27,    28,     2,     2,    34,     2,    36,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    35,
       2,    33,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    29,     2,    30,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,    32,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    50
    };
    // Last valid token kind.
    const int code_max = 294;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return static_cast <symbol_kind_type> (translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

#line 25 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"
} // fdmj
#line 2194 "/root/fducompilerh2026/fducompilerh2026/Compiler/build/lib/frontend/parser.cc"

#line 760 "/root/fducompilerh2026/fducompilerh2026/Compiler/lib/frontend/parser.yy"

/*
void yyerror(char *s) {
  fprintf(stderr, "%s\n",s);
}

int yywrap() {
  return(1);
}
*/

//%code 
namespace fdmj 
{
    template<typename RHS>
    inline void calcLocation(location_t &current, const RHS &rhs, const std::size_t n)
    {
        current = location_t(YYRHSLOC(rhs, 1).sline, YYRHSLOC(rhs, 1).scolumn, 
                                    YYRHSLOC(rhs, n).eline, YYRHSLOC(rhs, n).ecolumn);
        p = new Pos(current.sline, current.scolumn, current.eline, current.ecolumn);
    }
    
    void ASTParser::error(const location_t &location, const std::string &message)
    {
        std::cerr << "Error at lines " << location << ": " << message << std::endl;
    }

  Program* fdmjParser(ifstream &fp, const bool debug) {
    fdmj:AST_YYSTYPE result; 
    result.root = nullptr;
    fdmj::ASTLexer lexer(fp, debug);
    fdmj::ASTParser parser(lexer, debug, &result); //set up the parser
    if (parser() ) { //call the parser
      cout << "Error: parsing failed" << endl;
      return nullptr;
    }
    if (debug) cout << "Parsing successful" << endl;
    return result.root;
  }

  Program*  fdmjParser(const string &filename, const bool debug) {
    std::ifstream fp(filename);
    if (!fp) {
      cout << "Error: cannot open file " << filename << endl;
      return nullptr;
    }
    return fdmjParser(fp, debug);
  }
}
