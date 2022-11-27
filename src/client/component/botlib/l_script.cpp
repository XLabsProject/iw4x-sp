#include <std_include.hpp>
#include "l_script.hpp"

namespace pc {
// clang-format off
game::punctuation_s default_punctuations[] =
{
    { ">>=", P_RSHIFT_ASSIGN, nullptr },
    { "<<=", P_LSHIFT_ASSIGN, nullptr },

    { "...", P_PARMS, nullptr },
 //  define merge operator
    { "##", P_PRECOMPMERGE, nullptr },
 //  logic operators
    { "&&", P_LOGIC_AND, nullptr },
    { "||", P_LOGIC_OR, nullptr },
    { ">=", P_LOGIC_GEQ, nullptr },
    { "<=", P_LOGIC_LEQ, nullptr },
    { "==", P_LOGIC_EQ, nullptr },
    { "!=", P_LOGIC_UNEQ, nullptr },
 //  arithmatic operators
    { "*=", P_MUL_ASSIGN, nullptr },
    { "/=",P_DIV_ASSIGN,  nullptr },
    { "%=", P_MOD_ASSIGN, nullptr },
    { "+=", P_ADD_ASSIGN, nullptr },
    { "-=", P_SUB_ASSIGN,nullptr },
    { "++", P_INC, nullptr },
    { "--", P_DEC, nullptr },
 //  binary operators
    { "&=", P_BIN_AND_ASSIGN, nullptr },
    { "|=", P_BIN_OR_ASSIGN, nullptr },
    { "^=", P_BIN_XOR_ASSIGN, nullptr },
    { ">>", P_RSHIFT, nullptr },
    { "<<", P_LSHIFT, nullptr },
 //  reference operators
    { "->", P_POINTERREF, nullptr },
 //  C++
    { "::", P_CPP1, nullptr },
    { ".*", P_CPP2, nullptr },
 //  arithmatic operators
    { "*", P_MUL, nullptr },
    { "/", P_DIV,nullptr },
    { "%", P_MOD, nullptr },
    { "+", P_ADD, nullptr },
    { "-", P_SUB, nullptr },
    { "=", P_ASSIGN, nullptr },
 //  binary operators
    { "&", P_BIN_AND, nullptr },
    { "|", P_BIN_OR, nullptr },
    { "^", P_BIN_XOR, nullptr },
    { "~", P_BIN_NOT, nullptr },
 //  logic operators
    { "!", P_LOGIC_NOT, nullptr },
    { ">", P_LOGIC_GREATER, nullptr },
    { "<", P_LOGIC_LESS, nullptr },
 //  reference operator
    { ".", P_REF, nullptr },
 //  seperators
    { ",", P_COMMA, nullptr },
    { ";", P_SEMICOLON, nullptr },
 //  label indication
    { ":", P_COLON, nullptr },
 //  if statement
    { "?", P_QUESTIONMARK, nullptr },
 //  embracements
    { "(", P_PARENTHESESOPEN, nullptr },
    { ")", P_PARENTHESESCLOSE, nullptr },
    { "{", P_BRACEOPEN, nullptr },
    { "}", P_BRACECLOSE, nullptr },
    { "[", P_SQBRACKETOPEN, nullptr },
    { "]", P_SQBRACKETCLOSE, nullptr },
 //
    { "\\", P_BACKSLASH, nullptr },
 //  precompiler operator
    { "#", P_PRECOMP, nullptr },
    { "$", P_DOLLAR, nullptr },
    { nullptr, 0, nullptr },
};
// clang-format on

void create_punctuation_table(game::script_s* script,
                              game::punctuation_s* punctuations) {
  game::punctuation_s* p;

  // get memory for the table
  if (!script->punctuationtable) {
    script->punctuationtable = static_cast<game::punctuation_s**>(
        game::GetMemory(256 * sizeof(game::punctuation_s*)));
  }

  std::memset(script->punctuationtable, 0, 256 * sizeof(game::punctuation_s*));

  // add the punctuations in the list to the punctuation table
  for (auto i = 0; punctuations[i].p; ++i) {
    auto* newp = &punctuations[i];
    game::punctuation_s* lastp = nullptr;

    // sort the punctuations in this table entry on length (longer punctuations
    // first)
    for (p = script->punctuationtable[(unsigned int)newp->p[0]]; p;
         p = p->next) {
      if (std::strlen(p->p) < std::strlen(newp->p)) {
        newp->next = p;
        if (lastp) {
          lastp->next = newp;
        } else {
          script->punctuationtable[(unsigned int)newp->p[0]] = newp;
        }

        break;
      }

      lastp = p;
    }

    if (!p) {
      newp->next = nullptr;
      if (lastp) {
        lastp->next = newp;
      } else {
        script->punctuationtable[(unsigned int)newp->p[0]] = newp;
      }
    }
  }
}

void set_script_punctuations(game::script_s* script) {
  create_punctuation_table(script, default_punctuations);
  script->punctuations = default_punctuations;
}

game::script_s* load_script_file(const char* filename) {
  int fp;
  char pathname[game::MAX_QPATH];

  sprintf_s(pathname, "%s", filename);
  const auto length = game::FS_FOpenFileRead(pathname, &fp);
  if (!fp) {
    return nullptr;
  }

  auto* buffer = game::GetClearedMemory(sizeof(game::script_s) + length + 1);
  auto* script = static_cast<game::script_s*>(buffer);
  strncpy_s(script->filename, filename, _TRUNCATE);

  script->buffer = static_cast<char*>(buffer) + sizeof(game::script_s);
  script->buffer[length] = '\0';
  script->length = length;
  // pointer in script buffer
  script->script_p = script->buffer;
  // pointer in script buffer before reading token
  script->lastscript_p = script->buffer;
  // pointer to end of script buffer
  script->end_p = &script->buffer[length];
  // set if there's a token available in script->token
  script->tokenavailable = 0;

  script->line = 1;
  script->lastline = 1;

  set_script_punctuations(script);

  game::FS_Read(script->buffer, length, fp);
  game::FS_FCloseFile(fp);

  script->length = game::Com_Compress(script->buffer);
  return script;
}

game::script_s* load_script_memory(const char* ptr, int length,
                                   const char* name) {
  auto* buffer = game::GetClearedMemory(sizeof(game::script_s) + length + 1);
  auto* script = static_cast<game::script_s*>(buffer);

  strncpy_s(script->filename, name, _TRUNCATE);
  script->buffer = static_cast<char*>(buffer) + sizeof(game::script_s);
  script->buffer[length] = '\0';
  script->length = length;
  // pointer in script buffer
  script->script_p = script->buffer;
  // pointer in script buffer before reading token
  script->lastscript_p = script->buffer;
  // pointer to end of script buffer
  script->end_p = &script->buffer[length];
  // set if there's a token available in script->token
  script->tokenavailable = 0;

  script->line = 1;
  script->lastline = 1;

  set_script_punctuations(script);

  std::memcpy(script->buffer, ptr, length);

  return script;
}

void free_script(game::script_s* script) {
  if (script->punctuationtable)
    game::FreeMemory(script->punctuationtable);

  game::FreeMemory(script);
}
} // namespace pc
