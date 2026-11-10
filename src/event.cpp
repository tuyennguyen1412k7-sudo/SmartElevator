#include "elevator/event.h"

#include <algorithm>
#include <numeric>

#include "elevator/text_utils.h"

void EventBus::attach(std::shared_ptr<EventListener> listener) {
    if (listener) listeners_.push_back(listener);
}

void EventBus::publish(const Event& e) const {
    for (const auto& listener : listeners_) listener->onEvent(e);
}

void ConsoleLogger::onEvent(const Event& e) {
    if (!showMoves_ && e.type == EventType::Moved) return;

    out_ << "[t=" << padLeft(std::to_string(e.tick), 3, '0') << "] ";
    if (e.elevatorId >= 0)
        out_ << "Thang máy " << e.elevatorId << ": ";
    else
        out_ << "Hệ thống: ";
    out_ << e.message << '\n';
}

void StatsCollector::onEvent(const Event& e) {
    ++counts_[e.type];

    if (e.type == EventType::Moved) {
        ++movedByElevator_[e.elevatorId];
    } else if (e.type == EventType::PassengerBoarded) {
        waits_.push_back(e.value);
    } else if (e.type == EventType::PassengerExited) {
        rides_.push_back(e.value);
    }
}

int StatsCollector::count(EventType t) const {
    if (counts_.find(t) == counts_.end()) return 0;
    return counts_.at(t);
}

int StatsCollector::totalFloorsMoved() const {
    int total = 0;
    for (const auto& [elevatorId, floors] : movedByElevator_) total += floors;
    return total;
}

double StatsCollector::averageWait() const {
    if (waits_.empty()) return 0.0;
    double sum = std::accumulate(waits_.begin(), waits_.end(), 0.0);
    return sum / waits_.size();
}

long StatsCollector::maxWait() const {
    if (waits_.empty()) return 0L;
    return *std::max_element(waits_.begin(), waits_.end());
}

double StatsCollector::averageRide() const {
    if (rides_.empty()) return 0.0;
    double sum = std::accumulate(rides_.begin(), rides_.end(), 0.0);
    return sum / rides_.size();
}

void StatsCollector::report(std::ostream& out) const {
    out << "--- Thống kê ---\n";
    out << "Hành khách đã phục vụ : " << servedPassengers() << '\n';
    out << "Chờ trung bình / tối đa: " << formatFixed(averageWait(), 2) << " / " << maxWait() << " tick\n";
    out << "Thời gian đi trung bình: " << formatFixed(averageRide(), 2) << " tick\n";
    out << "Tổng số tầng đã di chuyển: " << totalFloorsMoved();
    for (const auto& [elevatorId, floors] : movedByElevator_) out << "  (T" << elevatorId << ": " << floors << ")";
    out << '\n';
    out << "Số lần từ chối do quá tải: " << count(EventType::OverloadRejected) << '\n';
}
