#pragma once

#define P_RSHIFT_ASSIGN 1
#define P_LSHIFT_ASSIGN 2
#define P_PARMS 3
#define P_PRECOMPMERGE 4

#define P_LOGIC_AND 5
#define P_LOGIC_OR 6
#define P_LOGIC_GEQ 7
#define P_LOGIC_LEQ 8
#define P_LOGIC_EQ 9
#define P_LOGIC_UNEQ 10

#define P_MUL_ASSIGN 11
#define P_DIV_ASSIGN 12
#define P_MOD_ASSIGN 13
#define P_ADD_ASSIGN 14
#define P_SUB_ASSIGN 15
#define P_INC 16
#define P_DEC 17

#define P_BIN_AND_ASSIGN 18
#define P_BIN_OR_ASSIGN 19
#define P_BIN_XOR_ASSIGN 20
#define P_RSHIFT 21
#define P_LSHIFT 22

#define P_POINTERREF 23
#define P_CPP1 24
#define P_CPP2 25
#define P_MUL 26
#define P_DIV 27
#define P_MOD 28
#define P_ADD 29
#define P_SUB 30
#define P_ASSIGN 31

#define P_BIN_AND 32
#define P_BIN_OR 33
#define P_BIN_XOR 34
#define P_BIN_NOT 35

#define P_LOGIC_NOT 36
#define P_LOGIC_GREATER 37
#define P_LOGIC_LESS 38

#define P_REF 39
#define P_COMMA 40
#define P_SEMICOLON 41
#define P_COLON 42
#define P_QUESTIONMARK 43

#define P_PARENTHESESOPEN 44
#define P_PARENTHESESCLOSE 45
#define P_BRACEOPEN 46
#define P_BRACECLOSE 47
#define P_SQBRACKETOPEN 48
#define P_SQBRACKETCLOSE 49
#define P_BACKSLASH 50

#define P_PRECOMP 51
#define P_DOLLAR 52

namespace pc {
game::script_s* load_script_file(const char* filename);
game::script_s* load_script_memory(const char* ptr, int length,
                                   const char* name);
void free_script(game::script_s* script);
} // namespace pc
