# Tests

## LuaWrapperTest

Tests the functionality of the `LuaWrapper`class.

- `test_doString_success()`: Call `DoString` with a valid string.
- `test_doString_error()`: Call `DoString` with a string that causes a Lua error.
- `test_doFile_success_and_gets()`: Call `DoFile` with a valid file and retrieve values.
- `test_doFile_missing_file()`: Call `DoFile` with a missing file.
- `test_setGlobals_then_read_from_lua()`: Call `SetGlobal` to set global variables and read them from Lua.
- `test_getGlobal_type_errors()`: Call `GetGlobal` with type errors.
- `test_registerFunction_and_call_from_lua()`: Call `registerFunction` to register a function and call it from Lua.
- `test_callIntInt_success()`: Call a Lua function that takes an int and returns an int successfully.
- `test_callIntInt_missing_function()`: Call a Lua function that does not exist.
- `test_callIntInt_wrong_return_type()`: Call a Lua function that returns the wrong type.
- `test_callIntInt_lua_runtime_error()`: Try calling a Lua function that causes a runtime error.