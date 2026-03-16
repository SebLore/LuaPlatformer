#include "LuaWrapper.h"

namespace Lua
{
    LuaWrapper::LuaWrapper()
    {
        L = luaL_newstate();
        if (!L)
            throw std::runtime_error("Failed to create lua_State");
        luaL_openlibs(L);
    }

    LuaWrapper::~LuaWrapper()
    {
        if (L)
            lua_close(L);
        L = nullptr;
    }

    LuaWrapper::LuaWrapper(LuaWrapper&& other) noexcept
        : L(other.L), lastError(std::move(other.lastError))
    {
        other.L = nullptr;
    }

    LuaWrapper& LuaWrapper::operator=(LuaWrapper&& other) noexcept
    {
        if (this != &other)
        {
            if (L)
                lua_close(L);
            L         = other.L;
            lastError = std::move(other.lastError);
            other.L   = nullptr;
        }
        return *this;
    }

    // --- Run Lua code ---
    bool LuaWrapper::DoFile(const std::string& file)
    {
        // 1) Try Lua's configured search path (package.path)
        // This enables: lua.doFile("player.lua") when scripts dir is in package.path.
        try
        {
            std::string resolved = ResolveWithPackageSearchPath(file);
            int         rc       = luaL_dofile(L, resolved.c_str());
            return HandleStatus(rc, "luaL_dofile");
        }
        catch (...)
        {
            // ignore and fallback
        }

        // 2) Fallback: base scripts
        const auto full = ResolveScriptPath(file);
        int        rc   = luaL_dofile(L, full.string().c_str());
        return HandleStatus(rc, "luaL_dofile");
    }

    bool LuaWrapper::RequireModule(const std::string& module)
    {
        // Lua's require expects module names like "player" not "player.lua"
        std::string mod = module;
        if (mod.size() >= 4 && mod.compare(mod.size() - 4, 4, ".lua") == 0)
            mod.resize(mod.size() - 4);

        lua_getglobal(L, "require");
        lua_pushlstring(L, mod.data(), mod.size());

        // require returns the module (usually a table); we don't need it here
        int rc = lua_pcall(L, 1, 1, 0);
        if (!HandleStatus(rc, "require"))
            return false;

        lua_pop(L, 1); // pop returned module
        return true;
    }

    /// Safely doString with a length-aware load
    bool LuaWrapper::DoString(const std::string& code)
    {
        int rc = luaL_loadbuffer(L, code.data(), code.size(), "chunk");
        if (!HandleStatus(rc, "luaL_loadbuffer"))
            return false;
        rc = lua_pcall(L, 0, LUA_MULTRET, 0);
        return HandleStatus(rc, "lua_pcall");
    }

    bool LuaWrapper::PCall(int nargs, int nresults)
    {
        int rc = lua_pcall(L, nargs, nresults, 0);
        return HandleStatus(rc, "lua_pcall");
    }

    // --- Get/Set globals (basic types) ---
    void LuaWrapper::SetGlobal(const std::string& name, int v) const
    {
        lua_pushinteger(L, v);
        lua_setglobal(L, name.c_str());
    }

    void LuaWrapper::SetGlobal(const std::string& name, double v) const
    {
        lua_pushnumber(L, v);
        lua_setglobal(L, name.c_str());
    }

    void LuaWrapper::SetGlobal(const std::string& name, bool v) const
    {
        lua_pushboolean(L, v);
        lua_setglobal(L, name.c_str());
    }

    void
    LuaWrapper::SetGlobal(const std::string& name, const std::string& v) const
    {
        lua_pushlstring(L, v.data(), v.size());
        lua_setglobal(L, name.c_str());
    }

    int LuaWrapper::GetGlobalInt(const std::string& name) const
    {
        lua_getglobal(L, name.c_str());

        if (lua_type(L, -1) != LUA_TNUMBER || !lua_isinteger(L, -1))
        {
            lua_pop(L, 1);
            throw std::runtime_error("Global '" + name + "' is not an integer");
        }

        int v = static_cast<int>(lua_tointeger(L, -1));
        lua_pop(L, 1);
        return v;
    }

    double LuaWrapper::GetGlobalNumber(const std::string& name) const
    {
        lua_getglobal(L, name.c_str());

        if (lua_type(L, -1) != LUA_TNUMBER)
        {
            lua_pop(L, 1);
            throw std::runtime_error("Global '" + name + "' is not a number");
        }

        double v = lua_tonumber(L, -1);
        lua_pop(L, 1);
        return v;
    }

    bool LuaWrapper::GetGlobalBool(const std::string& name) const
    {
        lua_getglobal(L, name.c_str());
        if (!lua_isboolean(L, -1))
        {
            lua_pop(L, 1);
            throw std::runtime_error("Global '" + name + "' is not a boolean");
        }
        bool v = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);
        return v;
    }

    std::string LuaWrapper::GetGlobalString(const std::string& name) const
    {
        lua_getglobal(L, name.c_str());

        if (lua_type(L, -1) != LUA_TSTRING)
        {
            lua_pop(L, 1);
            throw std::runtime_error("Global '" + name + "' is not a string");
        }

        size_t      len = 0;
        const char* s   = lua_tolstring(L, -1, &len);
        std::string out(s ? s : "", len);
        lua_pop(L, 1);
        return out;
    }

    // --- Register C function(s) ---
    void LuaWrapper::RegisterFunction(const std::string& name, lua_CFunction fn)
        const
    {
        lua_pushcfunction(L, fn);
        lua_setglobal(L, name.c_str());
    }

    // --- Call a Lua function (demo: int -> int) ---
    // Call: result = funcName(arg)
    int LuaWrapper::CallIntInt(const std::string& funcName, int arg)
    {
        lua_getglobal(L, funcName.c_str());
        if (!lua_isfunction(L, -1))
        {
            lua_pop(L, 1);
            throw std::runtime_error("'" + funcName + "' is not a function");
        }

        lua_pushinteger(L, arg);

        // protected call: 1 arg, 1 return
        int rc = lua_pcall(L, 1, 1, 0);
        if (!HandleStatus(rc, "lua_pcall"))
            throw std::runtime_error("Lua function call failed: " + lastError);

        if (!lua_isinteger(L, -1))
        {
            lua_pop(L, 1);
            throw std::runtime_error(
                "Lua function '" + funcName + "' did not return an integer");
        }

        int result = static_cast<int>(lua_tointeger(L, -1));
        lua_pop(L, 1);
        return result;
    }

    bool LuaWrapper::HandleStatus(int status, const char* where)
    {
        if (status == LUA_OK)
        {
            lastError.clear();
            return true;
        }

        // Lua error message is on stack top
        const char* msg = lua_tostring(L, -1);
        lastError = std::string(where) + ": " + (msg ? msg : "(unknown error)");
        lua_pop(L, 1);
        return false;
    }

    fs::path LuaWrapper::ResolveScriptPath(const std::string& file) const
    {
        fs::path p(file);

        // If already absolute, or no scriptsPath configured, use as-is.
        if (p.is_absolute() || m_ScriptsPath.empty())
            return p;

        // If caller already included directories (e.g. "ai/player.lua"),
        // this still works: scriptsPath / "ai/player.lua"
        return m_ScriptsPath / p;
    }

    void LuaWrapper::AddToPackagePath(const std::string& dir) const
    {
        lua_getglobal(L, "package"); // stack: package
        lua_getfield(L, -1, "path"); // stack: package, package.path

        const char* cur  = lua_tostring(L, -1);
        std::string path = cur ? cur : "";
        lua_pop(L, 1); // pop package.path

        path += ";" + dir + "/?.lua;" + dir + "/?/init.lua";

        lua_pushlstring(L, path.data(), path.size());
        lua_setfield(L, -2, "path"); // package.path = newPath
        lua_pop(L, 1);               // pop package
    }

    std::string LuaWrapper::ResolveWithPackageSearchPath(
        const std::string& fileOrModule) const
    {
        // Normalize "player.lua" -> "player"
        std::string mod = fileOrModule;
        if (mod.size() >= 4 && mod.compare(mod.size() - 4, 4, ".lua") == 0)
            mod.resize(mod.size() - 4);

        // Get package.searchpath
        lua_getglobal(L, "package");       // stack: package
        lua_getfield(L, -1, "searchpath"); // stack: package, searchpath
        lua_remove(L, -2);                 // stack: searchpath

        // Arg1: module name
        lua_pushlstring(L, mod.data(), mod.size());

        // Arg2: package.path
        lua_getglobal(L, "package"); // stack: searchpath, mod, package
        lua_getfield(L, -1, "path"); // stack: searchpath, mod, package, path
        lua_remove(L, -2);           // stack: searchpath, mod, path

        // pcall(searchpath, mod, path) -> (pathOrNil, errOrNil)
        int rc = lua_pcall(L, 2, 2, 0);
        if (rc != LUA_OK)
        {
            const char* e   = lua_tostring(L, -1);
            std::string err = e ? e : "package.searchpath failed";
            lua_pop(L, 1);
            throw std::runtime_error(err);
        }

        if (lua_isnil(L, -2))
        {
            const char* e   = lua_tostring(L, -1);
            std::string err = e ? e : "module not found";
            lua_pop(L, 2);
            throw std::runtime_error(err);
        }

        const char* resolved = lua_tostring(L, -2);
        std::string out      = resolved ? resolved : "";
        lua_pop(L, 2);
        return out;
    }
} // namespace Lua
