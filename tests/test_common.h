#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "elevator/controller.h"
#include "elevator/exceptions.h"
#include "elevator/persistence.h"
#include "elevator/simulator.h"
#include "elevator/text_utils.h"

extern int g_total;
extern int g_fail;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        ++g_total;                                                                    \
        if (!(cond)) {                                                                \
            ++g_fail;                                                                 \
            std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n"; \
        }                                                                             \
    } while (0)

#define CHECK_THROWS(expr, Type)                                                              \
    do {                                                                                      \
        ++g_total;                                                                            \
        try {                                                                                 \
            expr;                                                                             \
            ++g_fail;                                                                         \
            std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ << "  không ném " #Type "\n"; \
        } catch (const Type&) {                                                               \
        } catch (...) {                                                                       \
            ++g_fail;                                                                         \
            std::cerr << "  FAIL " << __FILE__ << ":" << __LINE__ << "  sai loại ngoại lệ (cần " #Type ")\n"; \
        }                                                                                     \
    } while (0)

struct TestCase {
    const char* name;
    void (*run)();
};

const std::string kData = "data";

inline SystemConfig defaultConfig() {
    SystemConfig c;
    c.lowestFloor = -1;
    c.highestFloor = 9;
    c.dwellTicks = 2;
    c.strategy = "eta";
    c.elevators = {{1, 0, 800.0}, {2, 4, 800.0}, {3, 9, 600.0}};
    return c;
}

struct Recorder : EventListener {
    std::vector<Event> events;
    void onEvent(const Event& e) override { events.push_back(e); }

    std::vector<int> doorOpenFloors(int elevatorId) const {
        std::vector<int> floors;
        for (const Event& e : events)
            if (e.type == EventType::DoorOpened && e.elevatorId == elevatorId) floors.push_back(e.floor);
        return floors;
    }
};

struct Rig {
    Building building{0, 9};
    EventBus bus;
    long clock = 0;
    std::vector<std::unique_ptr<Elevator>> elevators;

    Elevator& add(int id, int floor, double capacityKg = 800.0) {
        ElevatorSettings settings{Weight{capacityKg}, 2};
        elevators.push_back(std::make_unique<Elevator>(id, building, bus, clock, settings, floor));
        return *elevators.back();
    }

    std::vector<const Elevator*> all() const {
        std::vector<const Elevator*> list;
        for (const auto& e : elevators) list.push_back(e.get());
        return list;
    }
};

inline Passenger makePassenger(int id, double kg, int from, int to) {
    Passenger p;
    p.id = id;
    p.weight = Weight{kg};
    p.origin = from;
    p.destination = to;
    return p;
}

inline void runUntilIdle(ElevatorController& c, int maxTicks = 300) {
    for (int i = 0; i < maxTicks && !c.idle(); ++i) c.tick();
}

void registerUtilityTests(std::vector<TestCase>& tests);
void registerDispatchTests(std::vector<TestCase>& tests);
void registerElevatorTests(std::vector<TestCase>& tests);
void registerPersistenceTests(std::vector<TestCase>& tests);
