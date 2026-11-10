#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

enum class EventType {
    RequestReceived,
    RequestAssigned,
    RequestQueued,
    StateChanged,
    Moved,
    DoorOpened,
    DoorClosed,
    PassengerBoarded,
    PassengerExited,
    OverloadRejected,
    Info,
    Error
};

struct Event {
    long tick = 0;
    EventType type = EventType::Info;
    int elevatorId = -1;
    int floor = -1;
    std::string message;
    long value = 0;
};

class EventListener {
public:
    virtual ~EventListener() = default;
    virtual void onEvent(const Event& e) = 0;
};

class EventBus {
public:
    void attach(std::shared_ptr<EventListener> listener);
    void publish(const Event& e) const;

private:
    std::vector<std::shared_ptr<EventListener>> listeners_;
};

class ConsoleLogger : public EventListener {
public:
    explicit ConsoleLogger(std::ostream& out, bool showMoves = true) : out_(out), showMoves_(showMoves) {}
    void onEvent(const Event& e) override;

private:
    std::ostream& out_;
    bool showMoves_;
};

class StatsCollector : public EventListener {
public:
    void onEvent(const Event& e) override;

    int count(EventType t) const;
    int totalFloorsMoved() const;
    int servedPassengers() const { return rides_.size(); }
    double averageWait() const;
    long maxWait() const;
    double averageRide() const;
    void report(std::ostream& out) const;

private:
    std::map<EventType, int> counts_;
    std::map<int, int> movedByElevator_;
    std::vector<long> waits_;
    std::vector<long> rides_;
};
