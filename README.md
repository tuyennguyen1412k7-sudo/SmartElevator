# Mô phỏng thang máy thông minh (Đồ án OOP C++ – Chủ đề 24)

Mô phỏng một toà nhà nhiều tầng với nhiều thang máy. Hệ thống nhận yêu cầu (gọi thang ở sảnh, chọn tầng trong cabin, gọi khẩn cấp), điều phối bằng các chính sách có thể đổi lúc chạy, và mô phỏng chuyển động theo từng tick.

- Mẫu thiết kế: **State** (trạng thái thang), **Strategy** (chính sách điều phối), **Factory** (`StrategyFactory`, `RequestFactory`), **Observer** (`EventBus`).
- Ngoại lệ tự định nghĩa: `OverloadError`, `InvalidFloorError`, `ValidationError`, `ElevatorUnavailableError`, `FileError`, `ParseError` (đều kế thừa `ElevatorError`).
- Chuẩn ngôn ngữ: **C++20** (chỉ cần cho `std::erase_if`; phần còn lại là C++17).
- Phong cách code: đơn giản, mỗi hàm làm một việc, hạn chế thủ thuật nâng cao để dễ đọc và dễ giải thích khi bảo vệ.
- Chỉ dùng thư viện và kỹ thuật đã học trong các bài giảng (xem mục "Thư viện sử dụng"); không dùng `<filesystem>`, `<cctype>`, `<cmath>`, `std::stoi`, `static_cast`, namespace tự định nghĩa...

## Build

Cần trình biên dịch hỗ trợ C++20 (GCC ≥ 10, Clang ≥ 12, MSVC 2019 16.11+).

```bash
# Cách 1: make (không cần CMake)
make            # tạo elevator_sim và elevator_tests
make test       # chạy bộ test
make demo       # chạy tất cả kịch bản demo rồi thoát
make manual     # chạy chế độ tự thao tác với dữ liệu nhập mẫu
./elevator_sim  # mở menu để tự chọn

# Cách 2: CMake
cmake -S . -B build && cmake --build build
cd build && ./elevator_sim
ctest --output-on-failure

# Cách 3: gọi thẳng g++
g++ -std=c++20 -Wall -Wextra -Wpedantic -Iinclude src/*.cpp app/main.cpp -o elevator_sim
g++ -std=c++20 -Wall -Wextra -Wpedantic -Iinclude src/*.cpp tests/*.cpp -o elevator_tests
```

Chạy chương trình và test từ thư mục có `data/` (thư mục gốc dự án, hoặc thư mục `build` khi dùng CMake).
Tuỳ chọn: `./elevator_sim --demo` (chạy tất cả demo), `--data <thư mục>` (chỉ thư mục dữ liệu).

## Cấu trúc thư mục

```
include/elevator/   header (.h)
src/                cài đặt lõi (thư viện elevator_core)
app/main.cpp        menu console + các kịch bản demo + chế độ tự thao tác
tests/              bộ kiểm thử (không phụ thuộc thư viện ngoài), chia theo module: test_elevator, test_persistence, test_dispatch, test_utils + test_main (bộ chạy) + sample_manual_input.txt
data/               cấu hình và kịch bản mẫu (CSV)
docs/               sơ đồ UML (.dot nguồn + .png) và báo cáo
```

## Nên đọc code theo thứ tự nào

| Bước | File | Nội dung |
|---|---|---|
| 1 | `value_types`, `exceptions.h` | `Direction`, `Weight` (value type), `Passenger`; cây ngoại lệ |
| 2 | `request`, `building` | cây `Request` đa hình + `RequestFactory`; `Floor`, `Building` |
| 3 | `elevator_state`, `elevator` | State pattern; cabin thang máy, thuật toán LOOK |
| 4 | `dispatch_strategy` | Strategy + Factory: chọn thang nào phục vụ |
| 5 | `controller` | bộ điều phối của cả hệ thống |
| 6 | `event` | Observer: nhật ký và thống kê |
| 7 | `config`, `text_utils`, `persistence`, `simulator` | cấu hình, đọc/ghi file CSV, chạy kịch bản |
| 8 | `app/main.cpp`, `tests/` | menu, demo, kiểm thử |

## Menu chính

| Mục | Nội dung |
|---|---|
| 1 | Luồng nghiệp vụ cơ bản (`scenario_basic.csv`) |
| 2 | So sánh 3 chiến lược điều phối trên `scenario_rush.csv` |
| 3 | Ưu tiên khẩn cấp |
| 4 | Quá tải và an toàn ngoại lệ (strong guarantee) |
| 5 | Sự cố: thang ngừng hoạt động, phân lại yêu cầu |
| 6 | Kịch bản lỗi (lỗi tầng, lỗi file, lệnh sai) |
| 7 | Lưu / đọc / khôi phục trạng thái |
| 8 | **Tự thao tác** (menu con, xem bên dưới) |
| 9 | Chạy tất cả demo |

### Menu "Tự thao tác" (mục 8)

Nhập từng bước như một chương trình quản lý thông thường: chương trình hỏi từng thông tin, nhập sai thì báo lỗi và hỏi lại, lỗi nghiệp vụ được báo rõ loại lỗi rồi cho nhập tiếp.

| Mục | Nội dung |
|---|---|
| 1 | Thêm hành khách (khối lượng, tầng đứng, tầng đến; mã tự cấp) |
| 2 | Gọi thang ở sảnh (tầng, hướng) |
| 3 | Chọn tầng trong cabin (mã thang, tầng) |
| 4 | Gọi khẩn cấp |
| 5 | Chạy tiếp một số tick |
| 6 | Xem trạng thái các thang |
| 7 | Đổi chiến lược điều phối |
| 8 | Cho thang ngừng hoạt động / hoạt động lại |
| 9 | Tra cứu, sắp xếp, thống kê: lọc lịch sử yêu cầu theo loại, sắp xếp thang theo tải, hành khách đang chờ, bảng thống kê |
| 10 | Lưu / khôi phục trạng thái |

Chạy thử không cần gõ tay: `make manual` (dùng `tests/sample_manual_input.txt`).

## Định dạng file dữ liệu

`config_default.csv` – `building,<thấp>,<cao>` · `dwell,<tick>` · `strategy,<nearest|eta|least-load>` · `elevator,<mã>,<tầng xuất phát>,<kg>`

Kịch bản – mỗi dòng `tick,lệnh,tham số…`: `passenger,<id>,<kg>,<từ>,<đến>` · `hall,<tầng>,<up|down>` · `car,<thang>,<tầng>` · `emergency,<tầng>[,lý do]` · `strategy,<tên>` · `outofservice,<thang>` · `restore,<thang>`. Dòng bắt đầu bằng `#` là chú thích.

Snapshot (`elevator_state.csv`) gồm tick, chiến lược, vị trí/hướng/điểm dừng từng thang, hành khách trong cabin và đang chờ. Nội dung được dựng đầy đủ trong bộ nhớ (`std::ostringstream`) rồi mới ghi ra file bằng `std::ofstream`; mở hoặc ghi file lỗi thì ném `FileError`.

## Thư viện sử dụng

Tất cả header đều có trong bài giảng:

| Header | Dùng cho | Bài học |
|---|---|---|
| `<iostream>`, `<string>`, `<vector>` | nhập xuất, chuỗi, mảng động | Bài 2, 3, 8 |
| `<memory>` | `unique_ptr`, `shared_ptr`, `make_unique`, `make_shared` | Bài 3.3 |
| `<stdexcept>` | `std::runtime_error` làm cha của cây ngoại lệ | Bài 7 |
| `<fstream>`, `<sstream>`, `<iomanip>` | đọc/ghi file CSV, tách và đổi chuỗi thành số, định dạng số thực | Bài 8 |
| `<algorithm>`, `<numeric>` | `sort`, `find`, `find_if`, `count_if`, `max_element`, `accumulate`, `erase_if` | Bài 9.2 |
| `<map>`, `<set>`, `<deque>` | thống kê, điểm dừng, hàng đợi | Bài 9.2 |
| `<functional>` | `std::function` (nơi đăng ký chiến lược, hàm chấm điểm) | Bài 9.2 |
| `<optional>` | tầng khẩn cấp (có hoặc không có) | Bài 2 |
| `<utility>` | `std::move` | Bài 4 |

Đổi chuỗi thành số bằng `std::istringstream` (Bài 8), tách chuỗi bằng `std::getline` với dấu phân cách (Bài 8), duyệt `map` bằng structured binding (Bài 9.2).

## Yêu cầu đồ án nằm ở đâu trong code

| Yêu cầu | Vị trí |
|---|---|
| Cây lớp, kế thừa IS-A, hàm huỷ ảo | `Request` (request.h), `ElevatorState` (elevator_state.h), `DispatchStrategy` (dispatch_strategy.h), `ElevatorError` (exceptions.h), `EventListener` (event.h) |
| Container đa hình bằng smart pointer | `requestLog_` trong controller.h: `vector<unique_ptr<Request>>` |
| Đóng gói + bất biến | `Weight`, `Building`, `HallCall` kiểm tra dữ liệu ngay trong constructor |
| RAII | `unique_ptr`/`shared_ptr` khắp nơi; `ifstream`/`ofstream` trong persistence.cpp; không có `new`/`delete` thủ công |
| Ngoại lệ ≥ 2 lớp tự định nghĩa | exceptions.h; bắt ở `mainMenu` và `manualMode` (app/main.cpp), `Simulator::run`, `Elevator::loadPassengers` |
| An toàn ngoại lệ | `Elevator::board`, `ElevatorController::restore / setStrategy / addPassenger / pressFloor` |
| STL container | `vector`, `deque`, `set`, `map` |
| STL algorithm / lambda | `find_if`, `count_if`, `any_of`, `all_of`, `sort`, `stable_sort`, `accumulate`, `max_element`, `erase_if` |
| Strategy + Factory | dispatch_strategy.h, request.h |
| State, Observer | elevator_state.h, event.h |
| Value type + operator | `Weight` (`+ - * < == <<`) |
| File I/O | persistence.h/.cpp |
| Test + demo (luồng đúng, biên, lỗi) | tests/, data/scenario_*.csv, `--demo` |

## Thêm một chiến lược điều phối mới (OCP)

1. Tạo lớp kế thừa `DispatchStrategy`, cài `name()`, `description()`, `select()`.
   Mẹo: `select()` chỉ cần trả về thang tốt nhất trong danh sách ứng viên (hoặc `nullptr` nếu danh sách rỗng); xem `LeastLoadStrategy` để biết cách chấm điểm từng thang.
2. Đăng ký: `factory.registerStrategy("tên", [] { return std::make_unique<MyStrategy>(); });` (hoặc thêm một dòng vào constructor `StrategyFactory`).
3. Dùng ngay: `strategy,tên` trong file cấu hình/kịch bản hoặc mục "Đổi chiến lược" trong menu. Không phải sửa `ElevatorController`.

## Thêm một trạng thái thang mới (State)

Ví dụ trạng thái bảo trì: tạo lớp kế thừa `ElevatorState`, cài `name()` và `onTick()` (trả về trạng thái kế tiếp hoặc `nullptr` nếu giữ nguyên), ghi đè `acceptsRequests()` nếu không nhận yêu cầu. Không phải sửa các trạng thái khác.
