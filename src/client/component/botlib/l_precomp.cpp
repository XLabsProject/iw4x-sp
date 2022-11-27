#include <std_include.hpp>

#include "l_precomp.hpp"
#include "l_script.hpp"

namespace pc {
constexpr auto DEFINEHASHSIZE = 1024;

constexpr auto MAX_SOURCEFILES = 64;

game::define_s* globaldefines;

void free_token(game::token_s* token) {
  game::FreeMemory(token);
  --*game::numtokens;
}

game::define_s* define_from_string(const char* string) {
  game::source_s src;

  auto* script = load_script_memory(
      string, static_cast<int>(std::strlen(string)), "*extern");
  // create a new source
  std::memset(&src, 0, sizeof(game::source_s));
  strncpy_s(src.filename, "*extern", _TRUNCATE);

  src.scriptstack = script;
  src.definehash = static_cast<game::define_s**>(
      game::GetClearedMemory(DEFINEHASHSIZE * sizeof(game::define_s*)));

  // create a define from the source
  auto res = game::PC_Directive_define(&src);

  // free any tokens if left
  for (auto* t = src.tokens; t; t = src.tokens) {
    src.tokens = src.tokens->next;
    free_token(t);
  }

  game::define_s* def = nullptr;
  for (auto i = 0; i < DEFINEHASHSIZE; ++i) {
    if (src.definehash[i]) {
      def = src.definehash[i];
      break;
    }
  }

  game::FreeMemory(src.definehash);
  free_script(script);

  // if the define was created successfully
  if (res > 0) {
    return def;
  }

  // free the define is created
  if (src.defines) {
    game::PC_FreeDefine(def);
  }

  return nullptr;
}

int name_hash(char* name) {
  auto hash = 0;
  for (auto i = 0; name[i] != '\0'; ++i) {
    hash += name[i] * (119 + i);
  }

  hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (DEFINEHASHSIZE - 1);
  return hash;
}

void add_define_to_hash(game::define_s* define, game::define_s** definehash) {
  auto hash = name_hash(define->name);
  define->hashnext = definehash[hash];
  definehash[hash] = define;
}

int add_define(game::source_s* source, const char* string) {

  auto* define = define_from_string(string);
  if (!define) {
    return 0;
  }

  add_define_to_hash(define, source->definehash);

  return true;
}

game::define_s* copy_define([[maybe_unused]] game::source_s* source,
                            game::define_s* define) {
  game::token_s *token, *newtoken, *lasttoken;

  auto* newdefine = static_cast<game::define_s*>(
      game::GetMemory(sizeof(game::define_s) + strlen(define->name) + 1));

  // copy the define name
  newdefine->name = (char*)newdefine + sizeof(game::define_s);
  std::memcpy(newdefine->name, define->name, std::strlen(define->name) + 1);

  newdefine->flags = define->flags;
  newdefine->builtin = define->builtin;
  newdefine->numparms = define->numparms;

  // the define is not linked
  newdefine->next = nullptr;
  newdefine->hashnext = nullptr;

  // copy the define tokens
  newdefine->tokens = nullptr;
  for (lasttoken = nullptr, token = define->tokens; token;
       token = token->next) {
    newtoken = game::PC_CopyToken(token);
    newtoken->next = nullptr;
    if (lasttoken) {
      lasttoken->next = newtoken;
    } else {
      newdefine->tokens = newtoken;
    }

    lasttoken = newtoken;
  }

  // copy the define parameters
  newdefine->parms = nullptr;
  for (lasttoken = nullptr, token = define->parms; token; token = token->next) {
    newtoken = game::PC_CopyToken(token);
    newtoken->next = nullptr;
    if (lasttoken) {
      lasttoken->next = newtoken;
    } else {
      newdefine->parms = newtoken;
    }

    lasttoken = newtoken;
  }

  return newdefine;
}

void add_global_defines_to_source(game::source_s* source) {
  for (auto* define = globaldefines; define; define = define->next) {
    auto* newdefine = copy_define(source, define);

    add_define_to_hash(newdefine, source->definehash);
  }
}

game::source_s* load_source_file(const char* filename) {
  auto* script = load_script_file(filename);
  if (!script) {
    return nullptr;
  }

  script->next = nullptr;

  auto* source =
      static_cast<game::source_s*>(game::GetMemory(sizeof(game::source_s)));
  std::memset(source, 0, sizeof(game::source_s));

  strncpy_s(source->filename, filename, game::MAX_QPATH);
  source->scriptstack = script;
  source->tokens = nullptr;
  source->defines = nullptr;
  source->indentstack = nullptr;
  source->skip = 0;

  source->definehash = static_cast<game::define_s**>(
      game::GetClearedMemory(DEFINEHASHSIZE * sizeof(game::define_s*)));

  add_global_defines_to_source(source);
  return source;
}

int load_source_handle(const char* filename, const char** builtin_defines) {
  int i;

  for (i = 1; i < MAX_SOURCEFILES; ++i) {
    if (!game::sourceFiles[i]) {
      break;
    }
  }

  if (i >= MAX_SOURCEFILES) {
    return 0;
  }

  auto* source = load_source_file(filename);
  if (!source) {
    return 0;
  }

  if (builtin_defines) {
    for (auto define_iter = 0; builtin_defines[define_iter]; ++define_iter) {
      add_define(source, builtin_defines[define_iter]);
    }
  }

  game::sourceFiles[i] = source;
  return i;
}

void free_source(game::source_s* source) {
  // free all the scripts
  while (source->scriptstack) {
    auto* script = source->scriptstack;
    source->scriptstack = source->scriptstack->next;
    free_script(script);
  }

  // free all the tokens
  while (source->tokens) {
    auto* token = source->tokens;
    source->tokens = source->tokens->next;
    free_token(token);
  }

  for (auto i = 0; i < DEFINEHASHSIZE; ++i) {
    while (source->definehash[i]) {
      auto* define = source->definehash[i];
      source->definehash[i] = source->definehash[i]->hashnext;
      game::PC_FreeDefine(define);
    }
  }

  // free all indents
  while (source->indentstack) {
    auto* indent = source->indentstack;
    source->indentstack = source->indentstack->next;
    game::FreeMemory(indent);
  }

  if (source->definehash) {
    game::FreeMemory(source->definehash);
  }

  // free the source itself
  game::FreeMemory(source);
}

int free_source_handle(int handle) {
  if (handle < 1 || handle >= MAX_SOURCEFILES)
    return 0;

  if (!game::sourceFiles[handle])
    return 0;

  free_source(game::sourceFiles[handle]);
  game::sourceFiles[handle] = nullptr;
  return 1;
}
} // namespace pc
