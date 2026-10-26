/*
** Leapfrog mission tester: a small Lua host that runs run_tests.lua.
**
** The whole Lua library is compiled in from the Lua sources next to the
** repo (../../../lua from this folder, the same checkout the game uses),
** through onelua.c, so there is nothing to link.
**
** It finds run_tests.lua (the tester's folder) in the current directory,
** next to the executable or up to three folders above it; --root <dir>
** says where it is. All other arguments go to run_tests.lua. The exit code
** is run_tests.lua's: 0 all tests passed, 1 a test failed, 2 setup error.
*/

#define MAKE_LIB
#include "onelua.c"

#include <stdio.h>
#include <string.h>

static int fileExists(const char* path)
{
   FILE* f = fopen(path, "rb");
   if (f)
   {
      fclose(f);
      return 1;
   }
   return 0;
}

/* Looks for <dir>/run_tests.lua; root receives dir on success */
static int tryRoot(const char* dir, char* root, size_t size)
{
   char path[4096];
   snprintf(path, sizeof(path), "%s/run_tests.lua", dir);
   if (fileExists(path))
   {
      snprintf(root, size, "%s", dir);
      return 1;
   }
   return 0;
}

static int findRoot(const char* argv0, char* root, size_t size)
{
   char exeDir[4096];
   char candidate[4096];
   const char* slash1 = strrchr(argv0, '/');
   const char* slash2 = strrchr(argv0, '\\');
   const char* slash = (slash1 > slash2) ? slash1 : slash2;
   const char* ups[] = { "", "/..", "/../..", "/../../.." };
   int i;

   if (tryRoot(".", root, size))
   {
      return 1;
   }

   if (slash == NULL)
   {
      return 0;
   }

   snprintf(exeDir, sizeof(exeDir), "%.*s", (int)(slash - argv0), argv0);

   for (i = 0; i < 4; i++)
   {
      snprintf(candidate, sizeof(candidate), "%s%s", exeDir, ups[i]);
      if (tryRoot(candidate, root, size))
      {
         return 1;
      }
   }

   return 0;
}

int main(int argc, char** argv)
{
   char root[4096] = "";
   char script[4096];
   lua_State* L;
   int first = 1;
   int i;
   int n;
   int code;

   if ((argc >= 3) && (strcmp(argv[1], "--root") == 0))
   {
      snprintf(root, sizeof(root), "%s", argv[2]);
      first = 3;
   }
   else if (!findRoot(argv[0], root, sizeof(root)))
   {
      fprintf(stderr, "mission_tester: cannot find run_tests.lua; run it from the "
         "mission_tester folder or pass --root <folder>\n");
      return 2;
   }

   snprintf(script, sizeof(script), "%s/run_tests.lua", root);

   L = luaL_newstate();
   if (L == NULL)
   {
      fprintf(stderr, "mission_tester: cannot create a Lua state\n");
      return 2;
   }

   luaL_openlibs(L);

   lua_pushstring(L, root);
   lua_setglobal(L, "TESTER_ROOT");

   /* arg table: arg[0] is the script, then the remaining arguments */
   lua_createtable(L, argc, 0);
   lua_pushstring(L, script);
   lua_rawseti(L, -2, 0);
   for (i = first, n = 1; i < argc; i++, n++)
   {
      lua_pushstring(L, argv[i]);
      lua_rawseti(L, -2, n);
   }
   lua_setglobal(L, "arg");

   if (luaL_loadfile(L, script) != LUA_OK)
   {
      fprintf(stderr, "mission_tester: %s\n", lua_tostring(L, -1));
      lua_close(L);
      return 2;
   }

   if (lua_pcall(L, 0, 1, 0) != LUA_OK)
   {
      fprintf(stderr, "mission_tester: %s\n", lua_tostring(L, -1));
      lua_close(L);
      return 2;
   }

   code = lua_isinteger(L, -1) ? (int)lua_tointeger(L, -1) : 2;
   lua_close(L);

   fflush(stdout);
   return code;
}
