#include "LuaWrapperTest.h"

#include <cassert>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <string>

#include "LuaWrapper.h"

namespace tests
{
    using namespace Lua;

#define TEST_CASE(name)                                                        \
    do                                                                         \
    {                                                                          \
        ++g_testsRun;                                                          \
        std::cout << "[TEST] " << (name) << "\n";                              \
    } while (0)

    // ---------------------- Tiny test helpers ----------------------
    namespace
    {
        int g_testsRun = 0;

        void
        assert_contains(const std::string& haystack, const std::string& needle)
        {
            assert(haystack.find(needle) != std::string::npos);
        }

        void assert_contains_any(
            const std::string&                 haystack,
            std::initializer_list<const char*> needles)
        {
            for (const char* n : needles)
                if (haystack.find(n) != std::string::npos)
                    return;
            // If we reach here, none matched
            std::cerr << "Expected error to contain one of: ";
            for (const char* n : needles)
                std::cerr << "'" << n << "' ";
            std::cerr << "\nActual error: " << haystack << "\n";
            assert(!R"(Haystack did not contain any expected substrings)");
        }

        template <class Fn>
        void expect_throw_contains(Fn&& fn, const std::string& contains)
        {
            try
            {
                fn();
                assert(!R"(Expected exception, but none thrown)");
            }
            catch (const std::exception& e)
            {
                assert_contains(e.what(), contains);
            }
        }

        // ---------------------- C function for registerFunction test ----------------------
        int cpp_add(lua_State* L)
        {
            int a = static_cast<int>(luaL_checkinteger(L, 1));
            int b = static_cast<int>(luaL_checkinteger(L, 2));
            lua_pushinteger(L, (a + b));
            return 1;
        }

    } // namespace

    // ---------------------- Tests ----------------------
    void test_doString_success()
    {
        TEST_CASE("doString executes chunk and sets global");
        LuaWrapper lua;
        bool       ok = lua.DoString("a = 123");
        assert(ok);
        assert(lua.Error().empty());
        assert(lua.GetGlobalInt("a") == 123);
    }

    void test_doString_error()
    {
        TEST_CASE("doString returns false and sets error on syntax error");
        LuaWrapper lua;

        bool ok = lua.DoString("this is not lua code!!!");
        assert(!ok);
        assert(!lua.Error().empty());

        // Your implementation currently uses luaL_loadbuffer, so expect that,
        // but keep tolerant in case you switch again later.
        assert_contains_any(
            lua.Error(),
            { "luaL_loadbuffer", "luaL_loadbufferx", "luaL_dostring" });

        // Also validate it’s actually a Lua parse error (stable across implementations)
        assert_contains(lua.Error(), "syntax error");
    }

    void test_doFile_success_and_gets()
    {
        TEST_CASE("doFile executes file and getGlobal* works");
        LuaWrapper lua;

        const std::string path = "tmp_lua_test_script.lua";
        {
            std::ofstream f(path);
            f << "i = 7\n";
            f << "n = 3.25\n";
            f << "b = true\n";
            f << "s = 'hello'\n";
            f << "function times2(x) return x*2 end\n";
        }

        bool ok = lua.DoFile(path);
        assert(ok);
        assert(lua.GetGlobalInt("i") == 7);
        assert(lua.GetGlobalNumber("n") == 3.25);
        assert(lua.GetGlobalBool("b") == true);
        assert(lua.GetGlobalString("s") == "hello");

        std::remove(path.c_str());
    }

    void test_doFile_missing_file()
    {
        TEST_CASE("doFile returns false and sets error if file missing");
        LuaWrapper lua;

        bool ok = lua.DoFile("definitely_missing_file_12345.lua");
        assert(!ok);
        assert(!lua.Error().empty());

        // If your doFile still uses luaL_dofile internally, this will match.
        // If you later refactor doFile similarly (loadfile+pcall), you can make this tolerant too.
        assert_contains_any(
            lua.Error(),
            { "luaL_dofile", "luaL_loadfile", "luaL_loadfilex" });
    }

    void test_setGlobals_then_read_from_lua()
    {
        TEST_CASE("setGlobal pushes values Lua can read and compute with");
        LuaWrapper lua;

        lua.SetGlobal("x", 10);
        lua.SetGlobal("pi", 3.1415);
        lua.SetGlobal("flag", true);
        lua.SetGlobal("name", std::string("abc"));

        bool ok = lua.DoString(R"(
        z = x + 5
        ok_flag = flag and (name == "abc") and (pi > 3.0)
    )");
        assert(ok);

        assert(lua.GetGlobalInt("z") == 15);
        assert(lua.GetGlobalBool("ok_flag") == true);
    }

    void test_getGlobal_type_errors()
    {
        TEST_CASE("getGlobal* throws on wrong type");
        LuaWrapper lua;

        lua.DoString("val = 'not an int'");
        expect_throw_contains(
            [&] { (void)lua.GetGlobalInt("val"); },
            "not an integer");

        lua.DoString("val2 = 123");
        expect_throw_contains(
            [&] { (void)lua.GetGlobalString("val2"); },
            "not a string");

        lua.DoString("val3 = {}");
        expect_throw_contains(
            [&] { (void)lua.GetGlobalNumber("val3"); },
            "not a number");

        lua.DoString("val4 = 1"); // number, not boolean
        expect_throw_contains(
            [&] { (void)lua.GetGlobalBool("val4"); },
            "not a boolean");
    }

    void test_registerFunction_and_call_from_lua()
    {
        TEST_CASE("registerFunction exposes C function and Lua can call it");
        LuaWrapper lua;
        lua.RegisterFunction("cpp_add", cpp_add);

        bool ok = lua.DoString("sum = cpp_add(9, 33)");
        assert(ok);
        assert(lua.GetGlobalInt("sum") == 42);
    }

    void test_callIntInt_success()
    {
        TEST_CASE("callIntInt calls Lua function and returns int");
        LuaWrapper lua;
        lua.DoString("function times2(x) return x * 2 end");
        int r = lua.CallIntInt("times2", 21);
        assert(r == 42);
    }

    void test_callIntInt_missing_function()
    {
        TEST_CASE("callIntInt throws if function does not exist");
        LuaWrapper lua;
        expect_throw_contains(
            [&] { (void)lua.CallIntInt("nope", 1); },
            "not a function");
    }

    void test_callIntInt_wrong_return_type()
    {
        TEST_CASE("callIntInt throws if Lua returns wrong type");
        LuaWrapper lua;
        lua.DoString("function badret(x) return 'hello' end");
        expect_throw_contains(
            [&] { (void)lua.CallIntInt("badret", 1); },
            "did not return an integer");
    }

    void test_callIntInt_lua_runtime_error()
    {
        TEST_CASE("callIntInt throws if Lua errors during execution");
        LuaWrapper lua;
        lua.DoString("function boom(x) error('kaboom') end");
        expect_throw_contains(
            [&] { (void)lua.CallIntInt("boom", 1); },
            "Lua function call failed");
    }

    void test_runAllLuaWrapperTests()
    {
        test_doString_success();
        test_doString_error();
        test_doFile_success_and_gets();
        test_doFile_missing_file();
        test_setGlobals_then_read_from_lua();
        test_getGlobal_type_errors();
        test_registerFunction_and_call_from_lua();
        test_callIntInt_success();
        test_callIntInt_missing_function();
        test_callIntInt_wrong_return_type();
        test_callIntInt_lua_runtime_error();

        std::cout << "\nAll tests passed! (" << tests_run() << " tests)\n";
    }

    int tests_run() { return g_testsRun; }
} // namespace tests
