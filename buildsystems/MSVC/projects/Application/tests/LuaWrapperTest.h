#pragma once

// ---------------------- Tests ----------------------
namespace tests
{
    void test_doString_success();
    void test_doString_error();
    void test_doFile_success_and_gets();
    void test_doFile_missing_file();
    void test_setGlobals_then_read_from_lua();
    void test_getGlobal_type_errors();
    void test_registerFunction_and_call_from_lua();
    void test_callIntInt_success();
    void test_callIntInt_missing_function();
    void test_callIntInt_wrong_return_type();
    void test_callIntInt_lua_runtime_error();

    void test_runAllLuaWrapperTests();

    int tests_run();

} // namespace tests
