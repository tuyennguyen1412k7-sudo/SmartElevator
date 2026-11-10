#include "test_common.h"

void testTextUtils() {
    CHECK(trim("  abc \t") == "abc");
    CHECK(trim("   ") == "");
    CHECK(toLower("AbC") == "abc");

    const std::vector<std::string> parts = split(" a , b,c ", ',');
    CHECK(parts.size() == 3 && parts[0] == "a" && parts[1] == "b" && parts[2] == "c");

    CHECK(parseInt("42", "x") == 42);
    CHECK(parseInt("-7", "x") == -7);
    CHECK(parseDouble("2.5", "x") == 2.5);
    CHECK_THROWS(parseInt("12abc", "x"), ValidationError);
    CHECK_THROWS(parseInt("", "x"), ValidationError);
    CHECK_THROWS(parseDouble("abc", "x"), ValidationError);

    CHECK(formatFixed(3.14159, 2) == "3.14");
    CHECK(padRight("ab", 5) == "ab   ");
    CHECK(padRight("abcdef", 3) == "abcdef");
    CHECK(padLeft("7", 3, '0') == "007");
}

void testExceptionHierarchy() {
    OverloadError overload(900, 800);
    const ElevatorError& asBase = overload;
    CHECK(asBase.kind() == "OverloadError");
    CHECK(InvalidFloorError(5, 0, 3).kind() == "InvalidFloorError");
    CHECK(ElevatorUnavailableError(2).kind() == "ElevatorUnavailableError");
    CHECK(ValidationError("x").kind() == "ValidationError");
    CHECK(ParseError("f.csv", 7, "sai").kind() == "ParseError");
    CHECK(std::string(ParseError("f.csv", 7, "sai").what()) == "f.csv:7: sai");

    bool caughtAsFileError = false;
    try {
        throw ParseError("f.csv", 1, "sai");
    } catch (const FileError&) {
        caughtAsFileError = true;
    }
    CHECK(caughtAsFileError);
}

void registerUtilityTests(std::vector<TestCase>& tests) {
    tests.push_back({"Hàm xử lý chuỗi (text_utils)", testTextUtils});
    tests.push_back({"Cây ngoại lệ", testExceptionHierarchy});
}
