#pragma once
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

inline std::string kgText(double kg) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(1) << kg;
    return os.str();
}

class ElevatorError : public std::runtime_error {
public:
    explicit ElevatorError(const std::string& message) : std::runtime_error(message) {}

    virtual std::string kind() const { return "ElevatorError"; }
};

class ValidationError : public ElevatorError {
public:
    explicit ValidationError(const std::string& message) : ElevatorError(message) {}
    std::string kind() const override { return "ValidationError"; }
};

class InvalidFloorError : public ElevatorError {
public:
    InvalidFloorError(int floor, int lowest, int highest)
        : ElevatorError("Tầng " + std::to_string(floor) + " không hợp lệ (cho phép " +
                        std::to_string(lowest) + ".." + std::to_string(highest) + ")"),
          floor_(floor) {}

    int floor() const { return floor_; }
    std::string kind() const override { return "InvalidFloorError"; }

private:
    int floor_;
};

class OverloadError : public ElevatorError {
public:
    OverloadError(double attemptedKg, double limitKg)
        : ElevatorError("Quá tải: " + kgText(attemptedKg) + " kg > giới hạn " + kgText(limitKg) + " kg"),
          attemptedKg_(attemptedKg),
          limitKg_(limitKg) {}

    double attemptedKg() const { return attemptedKg_; }
    double limitKg() const { return limitKg_; }
    std::string kind() const override { return "OverloadError"; }

private:
    double attemptedKg_;
    double limitKg_;
};

class ElevatorUnavailableError : public ElevatorError {
public:
    explicit ElevatorUnavailableError(int elevatorId)
        : ElevatorError("Thang máy " + std::to_string(elevatorId) + " không khả dụng"),
          elevatorId_(elevatorId) {}

    int elevatorId() const { return elevatorId_; }
    std::string kind() const override { return "ElevatorUnavailableError"; }

private:
    int elevatorId_;
};

class FileError : public ElevatorError {
public:
    explicit FileError(const std::string& message) : ElevatorError(message) {}
    std::string kind() const override { return "FileError"; }
};

class ParseError : public FileError {
public:
    ParseError(const std::string& file, int line, const std::string& what)
        : FileError(file + ":" + std::to_string(line) + ": " + what), line_(line) {}

    int line() const { return line_; }
    std::string kind() const override { return "ParseError"; }

private:
    int line_;
};
