#pragma once

#include <lua.hpp>

// STL
#include <filesystem>
#include <string>

namespace Lua
{
    class LuaWrapper
    {
      public:
        LuaWrapper();

        ~LuaWrapper();

        // no copying
        LuaWrapper(const LuaWrapper&)            = delete;
        LuaWrapper& operator=(const LuaWrapper&) = delete;

        LuaWrapper(LuaWrapper&& other) noexcept;

        LuaWrapper& operator=(LuaWrapper&& other) noexcept;

        // --- Run Lua code ---
        bool DoFile(const std::string& file);
        bool RequireModule(const std::string& module);

        /// Safely doString with a length-aware load
        bool DoString(const std::string& code);

        bool PCall(int nargs, int nresults);

        // --- Get/Set globals (basic types) ---
        void SetGlobal(const std::string& name, int v) const;
        void SetGlobal(const std::string& name, double v) const;
        void SetGlobal(const std::string& name, bool v) const;
        void SetGlobal(const std::string& name, const std::string& v) const;

        int         GetGlobalInt(const std::string& name) const;
        double      GetGlobalNumber(const std::string& name) const;
        bool        GetGlobalBool(const std::string& name) const;
        std::string GetGlobalString(const std::string& name) const;

        // --- Register C function(s) ---
        void RegisterFunction(const std::string& name, lua_CFunction fn) const;

        // --- Call a Lua function (demo: int -> int) ---
        // Call: result = funcName(arg)
        int CallIntInt(const std::string& funcName, int arg);
        // getters
        const std::string& Error() const { return lastError; }
        lua_State*         State() const { return L; }

        // -- functions to manipulate the script paths for dofile and require respectively
        void SetScriptsPath(const std::string& path)
        {
            m_ScriptsPath = std::filesystem::path(path);

            // Make Lua's require/searchpath look here too
            AddToPackagePath(m_ScriptsPath.string());
        }

        const std::filesystem::path& GetScriptsPath() const
        {
            return m_ScriptsPath;
        }

      private:
        bool HandleStatus(int status, const char* where);

        std::filesystem::path ResolveScriptPath(const std::string& file) const;
        void                  AddToPackagePath(const std::string& dir) const;
        std::string
        ResolveWithPackageSearchPath(const std::string& fileOrModule) const;

      private:
        lua_State*            L = nullptr;
        std::string           lastError;
        std::filesystem::path m_ScriptsPath;
    };

    // function to call function with arg
    inline bool CallLuaVoid1Number(LuaWrapper& lua, const char* fn, float arg)
    {
        lua_State* L = lua.State();

        lua_getglobal(L, fn);
        if (!lua_isfunction(L, -1))
        {
            lua_pop(L, 1); // pop non-function
            return true;   // treat missing as "ok"
        }

        lua_pushnumber(L, (lua_Number)arg);

        return lua.PCall(1, 0);
    }

    // function to call function with no args
    inline bool CallLuaVoid0(LuaWrapper& lua, const char* fn)
    {
        lua_State* L = lua.State();

        lua_getglobal(L, fn);
        if (!lua_isfunction(L, -1))
        {
            lua_pop(L, 1);
            return true;
        }

        return lua.PCall(0, 0);
    }

} // namespace Lua
