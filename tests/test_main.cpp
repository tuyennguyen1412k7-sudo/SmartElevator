#include "test_common.h"

int g_total = 0;
int g_fail = 0;

int main() {
    std::vector<TestCase> tests;
    registerUtilityTests(tests);
    registerDispatchTests(tests);
    registerElevatorTests(tests);
    registerPersistenceTests(tests);

    for (const TestCase& t : tests) {
        const int failBefore = g_fail;
        try {
            t.run();
        } catch (const std::exception& e) {
            ++g_fail;
            std::cerr << "  NGOẠI LỆ KHÔNG MONG ĐỢI: " << e.what() << '\n';
        }
        std::cout << (g_fail == failBefore ? "[ OK ] " : "[FAIL] ") << t.name << '\n';
    }
    std::cout << "\nTổng: " << g_total << " kiểm tra, " << g_fail << " lỗi\n";
    return g_fail == 0 ? 0 : 1;
}
