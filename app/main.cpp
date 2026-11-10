#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <string>

#include "elevator/controller.h"
#include "elevator/exceptions.h"
#include "elevator/persistence.h"
#include "elevator/simulator.h"
#include "elevator/text_utils.h"

struct InputClosed {};

struct Session {
    std::unique_ptr<ElevatorController> controller;
    std::shared_ptr<StatsCollector> stats;
};

Session makeSession(const SystemConfig& cfg, bool verbose, bool showMoves = false) {
    Session s;
    s.controller = std::make_unique<ElevatorController>(cfg);
    s.stats = std::make_shared<StatsCollector>();
    s.controller->attach(s.stats);
    if (verbose) s.controller->attach(std::make_shared<ConsoleLogger>(std::cout, showMoves));
    return s;
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) throw InputClosed{};
    return trim(line);
}

int readInt(const std::string& prompt) {
    for (;;) {
        const std::string text = readLine(prompt);
        try {
            return parseInt(text, "Giá trị");
        } catch (const ElevatorError& ex) {
            std::cout << "  Nhập sai: " << ex.what() << ". Hãy nhập lại.\n";
        }
    }
}

double readDouble(const std::string& prompt) {
    for (;;) {
        const std::string text = readLine(prompt);
        try {
            return parseDouble(text, "Giá trị");
        } catch (const ElevatorError& ex) {
            std::cout << "  Nhập sai: " << ex.what() << ". Hãy nhập lại.\n";
        }
    }
}

void banner(const std::string& title) { std::cout << "\n########## " << title << " ##########\n"; }

std::string joinStops(const std::set<int>& stops) {
    std::string text;
    for (int s : stops) {
        if (!text.empty()) text += ",";
        text += std::to_string(s);
    }
    return text.empty() ? "-" : text;
}

void printStatus(const ElevatorController& c) {
    std::cout << "=== Trạng thái t=" << c.now() << " | chiến lược: " << c.strategy().name() << " ===\n";
    for (const auto& e : c.elevators()) std::cout << "  " << e->describe() << '\n';
    for (const Floor& f : c.building().floors())
        if (!f.waiting().empty()) std::cout << "  Tầng " << f.number() << ": " << f.waiting().size() << " người đang chờ\n";
}

void expectError(const std::string& label, const std::function<void()>& action) {
    try {
        action();
        std::cout << "  [KHÔNG LỖI] " << label << '\n';
    } catch (const ElevatorError& ex) {
        std::cout << "  [" << ex.kind() << "] " << label << "\n      -> " << ex.what() << '\n';
    }
}

std::string resolveDataDir(const std::string& dir) {
    const std::string candidates[]{dir, "../" + dir, "../../" + dir};
    for (const std::string& c : candidates) {
        std::ifstream probe{c + "/config_default.csv"};
        if (probe.good()) return c;
    }
    return dir;
}

void runScenarioFile(const SystemConfig& cfg, const std::string& path, bool showMoves = false) {
    Session s = makeSession(cfg, true, showMoves);
    Simulator simulator(*s.controller);
    const SimulationResult result = simulator.run(loadScenario(path));

    std::cout << "Kết thúc sau " << result.ticks << " tick; lệnh OK/lỗi: " << result.commandsOk << '/'
              << result.commandsFailed << (result.timedOut ? " (HẾT THỜI GIAN)" : "") << '\n';
    s.stats->report(std::cout);
}

void compareStrategies(const SystemConfig& base, const std::string& dir) {
    banner("So sánh chiến lược điều phối (scenario_rush.csv)");
    const auto events = loadScenario(dir + "/scenario_rush.csv");
    StrategyFactory factory;

    std::cout << padRight("Strategy", 12) << padRight("Ticks", 14) << padRight("AvgWait", 14)
              << padRight("MaxWait", 14) << padRight("FloorsMoved", 14) << '\n';
    for (const std::string& name : factory.names()) {
        SystemConfig cfg = base;
        cfg.strategy = name;

        Session s = makeSession(cfg, false);
        const SimulationResult r = Simulator(*s.controller).run(events);

        std::cout << padRight(name, 12) << padRight(std::to_string(r.ticks), 14)
                  << padRight(formatFixed(s.stats->averageWait(), 2), 14)
                  << padRight(std::to_string(s.stats->maxWait()), 14)
                  << padRight(std::to_string(s.stats->totalFloorsMoved()), 14) << '\n';
    }
}

void demoEmergency(const SystemConfig& cfg, const std::string& dir) {
    banner("Ưu tiên khẩn cấp (scenario_emergency.csv)");
    runScenarioFile(cfg, dir + "/scenario_emergency.csv");

    banner("Ưu tiên khẩn cấp: thang bỏ qua điểm dừng thường để chạy thẳng");
    std::cout << "Chỉ còn thang 1 hoạt động; thang 1 có điểm dừng tại tầng 3 và 7, rồi có cuộc gọi khẩn cấp tầng 8\n";
    Session s = makeSession(cfg, true, true);
    ElevatorController& c = *s.controller;
    c.setOutOfService(2);
    c.setOutOfService(3);
    c.pressFloor(1, 3);
    c.pressFloor(1, 7);
    c.tick();
    c.tick();
    c.emergencyCall(8, "y tế");
    for (int i = 0; i < 40 && !c.idle(); ++i) c.tick();
}

Passenger makePassenger(int id, double kg, int destination) {
    Passenger p;
    p.id = id;
    p.weight = Weight{kg};
    p.origin = 0;
    p.destination = destination;
    return p;
}

void demoOverload(const SystemConfig& cfg, const std::string& dir) {
    banner("Quá tải: hành khách nặng phải chờ thang khác");
    runScenarioFile(cfg, dir + "/scenario_overload.csv");

    banner("An toàn ngoại lệ: board() thất bại thì cabin không đổi (strong guarantee)");
    Session s = makeSession(cfg, false);
    Elevator& e = s.controller->elevator(1);
    e.board(makePassenger(1, 300, 5));
    e.board(makePassenger(2, 300, 5));
    std::cout << "  Trước: tải " << e.load() << ", " << e.passengerCount() << " khách, điểm dừng "
              << joinStops(e.stops()) << '\n';
    try {
        e.board(makePassenger(3, 300, 5));
    } catch (const OverloadError& ex) {
        std::cout << "  Bắt OverloadError: " << ex.what() << '\n';
    }
    std::cout << "  Sau  : tải " << e.load() << ", " << e.passengerCount() << " khách, điểm dừng "
              << joinStops(e.stops()) << '\n';
}

void demoErrors(const SystemConfig& cfg, const std::string& dir) {
    banner("Kịch bản lỗi: mỗi lỗi được bắt đúng tầng, hệ thống không hỏng");
    Session s = makeSession(cfg, false);
    ElevatorController& c = *s.controller;

    expectError("pressFloor(1, 99)", [&] { c.pressFloor(1, 99); });
    expectError("pressFloor(7, 3)  // không có thang 7", [&] { c.pressFloor(7, 3); });
    expectError("callElevator(9, Up)  // tầng cao nhất không đi lên được", [&] { c.callElevator(9, Direction::Up); });
    expectError("addPassenger khối lượng âm", [&] { c.addPassenger(1, -5, 0, 3); });

    const std::string before = c.strategy().name();
    expectError("setStrategy(\"xyz\")", [&] { c.setStrategy("xyz"); });
    std::cout << "  Chiến lược trước/sau lỗi: " << before << " / " << c.strategy().name() << " (không đổi)\n";

    expectError("loadConfig(file không tồn tại)", [&] { loadConfig(dir + "/khong_co.csv"); });
    expectError("loadScenario(scenario_bad.csv)", [&] { loadScenario(dir + "/scenario_bad.csv"); });

    std::cout << "\n  Chạy scenario_errors.csv (lệnh lỗi được ghi nhận, mô phỏng vẫn tiếp tục):\n";
    runScenarioFile(cfg, dir + "/scenario_errors.csv");
}

void demoPersistence(const SystemConfig& cfg) {
    banner("Lưu / đọc trạng thái hệ thống");
    Session s = makeSession(cfg, false);
    ElevatorController& c = *s.controller;
    c.addPassenger(1, 70, 0, 8);
    c.addPassenger(2, 60, 5, 1);
    for (int i = 0; i < 4; ++i) c.tick();
    printStatus(c);

    const std::string file = "elevator_state.csv";
    saveSnapshot(c.snapshot(), file);
    std::cout << "-> đã lưu " << file << "\n";

    for (int i = 0; i < 6; ++i) c.tick();
    std::cout << "(chạy tiếp 6 tick)\n";
    printStatus(c);

    c.restore(loadSnapshot(file));
    std::cout << "-> đã khôi phục từ file\n";
    printStatus(c);

    Snapshot bad = c.snapshot();
    bad.elevators[0].floor = 99;
    expectError("restore(snapshot có tầng 99)", [&] { c.restore(bad); });
    std::cout << "Sau lỗi, trạng thái vẫn nguyên vẹn:\n";
    printStatus(c);
}

void runAllDemos(const SystemConfig& cfg, const std::string& dir) {
    banner("Luồng nghiệp vụ cơ bản (scenario_basic.csv)");
    runScenarioFile(cfg, dir + "/scenario_basic.csv");
    compareStrategies(cfg, dir);
    demoEmergency(cfg, dir);
    demoOverload(cfg, dir);
    banner("Sự cố thang máy (scenario_outage.csv)");
    runScenarioFile(cfg, dir + "/scenario_outage.csv");
    demoErrors(cfg, dir);
    demoPersistence(cfg);
}

void actionAddPassenger(ElevatorController& c) {
    const double kg = readDouble("  Khối lượng (kg): ");
    const int from = readInt("  Tầng đang đứng: ");
    const int to = readInt("  Tầng muốn đến: ");
    const int id = c.addPassenger(kg, from, to);
    std::cout << "  -> Đã thêm hành khách #" << id << '\n';
}

void actionCallElevator(ElevatorController& c) {
    const int floor = readInt("  Tầng gọi thang: ");
    const int dir = readInt("  Hướng (1 = lên, 2 = xuống): ");
    if (dir != 1 && dir != 2) throw ValidationError("Hướng chỉ nhận 1 (lên) hoặc 2 (xuống)");
    c.callElevator(floor, dir == 1 ? Direction::Up : Direction::Down);
}

void actionPressFloor(ElevatorController& c) {
    const int elevatorId = readInt("  Mã thang: ");
    const int floor = readInt("  Tầng muốn đến: ");
    c.pressFloor(elevatorId, floor);
}

void actionEmergency(ElevatorController& c) {
    const int floor = readInt("  Tầng xảy ra sự việc: ");
    std::string reason = readLine("  Lý do (Enter = y tế): ");
    if (reason.empty()) reason = "y tế";
    c.emergencyCall(floor, reason);
}

void actionRunTicks(ElevatorController& c) {
    const int n = readInt("  Chạy bao nhiêu tick (1..1000): ");
    if (n < 1 || n > 1000) throw ValidationError("Số tick phải nằm trong khoảng 1..1000");
    for (int i = 0; i < n; ++i) c.tick();
}

void actionChangeStrategy(ElevatorController& c) {
    std::cout << "  Các chiến lược hiện có:\n";
    for (const std::string& name : c.strategies().names())
        std::cout << "    - " << name << ": " << c.strategies().create(name)->description() << '\n';
    c.setStrategy(readLine("  Nhập tên chiến lược: "));
}

void actionOutOfService(ElevatorController& c) {
    const int elevatorId = readInt("  Mã thang: ");
    const int mode = readInt("  1 = cho ngừng hoạt động, 2 = cho hoạt động lại: ");
    if (mode == 1)
        c.setOutOfService(elevatorId);
    else if (mode == 2)
        c.restoreService(elevatorId);
    else
        throw ValidationError("Chỉ nhận 1 hoặc 2");
}

void actionSearch(const ElevatorController& c, const StatsCollector& stats) {
    std::cout << "  1. Lọc lịch sử yêu cầu theo loại\n"
                 "  2. Sắp xếp các thang theo tải (nặng nhất trước)\n"
                 "  3. Hành khách đang chờ (chờ lâu nhất trước)\n"
                 "  4. Thống kê tổng hợp\n";
    const int choice = readInt("  Chọn: ");

    if (choice == 1) {
        const std::string kinds[] = {"hall", "car", "emergency"};
        const int k = readInt("  Loại (1 = hall, 2 = car, 3 = emergency): ");
        if (k < 1 || k > 3) throw ValidationError("Loại yêu cầu chỉ nhận 1, 2 hoặc 3");
        const auto found = c.findRequests(kinds[k - 1]);
        std::cout << "  Có " << found.size() << " yêu cầu loại '" << kinds[k - 1] << "':\n";
        for (const Request* r : found)
            std::cout << "    #" << r->id() << " [t=" << r->createdTick() << "] " << r->describe() << '\n';
    } else if (choice == 2) {
        for (const Elevator* e : c.elevatorsByLoad()) std::cout << "    " << e->describe() << '\n';
    } else if (choice == 3) {
        const auto waiting = c.waitingList();
        if (waiting.empty()) std::cout << "    Không có ai đang chờ.\n";
        for (const Passenger& p : waiting)
            std::cout << "    " << p.describe() << " (đến lúc t=" << p.arrivalTick << ")\n";
    } else if (choice == 4) {
        stats.report(std::cout);
    } else {
        std::cout << "  Lựa chọn không hợp lệ.\n";
    }
}

void actionSaveLoad(ElevatorController& c) {
    const std::string file = "elevator_state.csv";
    const int choice = readInt("  1 = lưu trạng thái, 2 = khôi phục từ file: ");
    if (choice == 1) {
        saveSnapshot(c.snapshot(), file);
        std::cout << "  -> Đã lưu vào " << file << '\n';
    } else if (choice == 2) {
        c.restore(loadSnapshot(file));
        std::cout << "  -> Đã khôi phục từ " << file << '\n';
    } else {
        std::cout << "  Lựa chọn không hợp lệ.\n";
    }
}

void manualMode(const SystemConfig& cfg) {
    Session s = makeSession(cfg, true, true);
    ElevatorController& c = *s.controller;

    for (;;) {
        std::cout << "\n----- TỰ THAO TÁC (t=" << c.now() << ") -----\n"
                     " 1. Thêm hành khách\n"
                     " 2. Gọi thang ở sảnh\n"
                     " 3. Chọn tầng trong cabin\n"
                     " 4. Gọi khẩn cấp\n"
                     " 5. Chạy tiếp một số tick\n"
                     " 6. Xem trạng thái các thang\n"
                     " 7. Đổi chiến lược điều phối\n"
                     " 8. Cho thang ngừng hoạt động / hoạt động lại\n"
                     " 9. Tra cứu, sắp xếp, thống kê\n"
                     "10. Lưu / khôi phục trạng thái\n"
                     " 0. Quay lại\n";
        const int choice = readInt("Chọn: ");
        if (choice == 0) return;

        try {
            switch (choice) {
                case 1: actionAddPassenger(c); break;
                case 2: actionCallElevator(c); break;
                case 3: actionPressFloor(c); break;
                case 4: actionEmergency(c); break;
                case 5: actionRunTicks(c); break;
                case 6: printStatus(c); break;
                case 7: actionChangeStrategy(c); break;
                case 8: actionOutOfService(c); break;
                case 9: actionSearch(c, *s.stats); break;
                case 10: actionSaveLoad(c); break;
                default: std::cout << "Lựa chọn không hợp lệ.\n";
            }
        } catch (const ElevatorError& ex) {
            std::cout << "LỖI (" << ex.kind() << "): " << ex.what() << '\n';
        }
    }
}

void mainMenu(const SystemConfig& cfg, const std::string& dir) {
    for (;;) {
        std::cout << "\n===== MÔ PHỎNG THANG MÁY THÔNG MINH =====\n"
                     " 1. Luồng nghiệp vụ cơ bản\n"
                     " 2. So sánh chiến lược điều phối (đổi scheduler)\n"
                     " 3. Ưu tiên khẩn cấp\n"
                     " 4. Quá tải & an toàn ngoại lệ\n"
                     " 5. Sự cố thang máy (ngừng hoạt động / phục hồi)\n"
                     " 6. Kịch bản lỗi\n"
                     " 7. Lưu / đọc trạng thái\n"
                     " 8. Tự thao tác (thêm khách, gọi thang, xem trạng thái...)\n"
                     " 9. Chạy tất cả demo\n"
                     " 0. Thoát\n";
        const int choice = readInt("Chọn: ");
        if (choice == 0) return;

        try {
            switch (choice) {
                case 1:
                    banner("Cơ bản");
                    runScenarioFile(cfg, dir + "/scenario_basic.csv", true);
                    break;
                case 2: compareStrategies(cfg, dir); break;
                case 3: demoEmergency(cfg, dir); break;
                case 4: demoOverload(cfg, dir); break;
                case 5:
                    banner("Sự cố");
                    runScenarioFile(cfg, dir + "/scenario_outage.csv");
                    break;
                case 6: demoErrors(cfg, dir); break;
                case 7: demoPersistence(cfg); break;
                case 8: manualMode(cfg); break;
                case 9: runAllDemos(cfg, dir); break;
                default: std::cout << "Lựa chọn không hợp lệ.\n";
            }
        } catch (const ElevatorError& ex) {
            std::cout << "LỖI (" << ex.kind() << "): " << ex.what() << '\n';
        }
    }
}

int main(int argc, char** argv) {
    std::string dir = "data";
    bool demoOnly = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--demo") {
            demoOnly = true;
        } else if (arg == "--data" && i + 1 < argc) {
            dir = argv[++i];
        } else {
            std::cerr << "Cách dùng: " << argv[0] << " [--demo] [--data <thư mục dữ liệu>]\n";
            return 2;
        }
    }
    dir = resolveDataDir(dir);

    try {
        const SystemConfig cfg = loadConfig(dir + "/config_default.csv");
        if (demoOnly)
            runAllDemos(cfg, dir);
        else
            mainMenu(cfg, dir);
    } catch (const InputClosed&) {
        std::cout << "\nHết dữ liệu nhập, thoát chương trình.\n";
    } catch (const std::exception& ex) {
        std::cerr << "Lỗi nghiêm trọng: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
