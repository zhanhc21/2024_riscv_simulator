#pragma once
#include <algorithm>
#include <memory>
#include <sstream>

#include "instructions.h"
#include "issue_slot.h"
#include "logger.h"
#include "register_file.h"
#include "rob.h"

template <unsigned size>
class ReservationStation {
    IssueSlot buffer[size];

public:
    ReservationStation();
    [[nodiscard]] bool hasEmptySlot() const;
    void insertInstruction(const Instruction &inst,
                           unsigned robIdx,
                           RegisterFile *regFile,
                           const ReorderBuffer &reorderBuffer);
    void wakeup(const ROBStatusWritePort &x);
    [[nodiscard]] bool canIssue() const;
    IssueSlot issue();
    void flush();
    void zip();
};

template <unsigned size>
ReservationStation<size>::ReservationStation() {
    for (auto &slot : buffer) {
        slot.busy = false;
    }
}

template <unsigned size>
bool ReservationStation<size>::hasEmptySlot() const {
    return std::any_of(buffer, buffer + size, [](const IssueSlot &slot) {
        return !slot.busy;
    });
}

template <unsigned size>
void ReservationStation<size>::insertInstruction(
    [[maybe_unused]] const Instruction &inst,
    [[maybe_unused]] unsigned robIdx,
    [[maybe_unused]] RegisterFile *const regFile,
    [[maybe_unused]] const ReorderBuffer &reorderBuffer) {
    // 若有 slot 空闲，执行插入
    // 插入时，设置两个寄存器读取端口是否已经唤醒，以及对应值，必要时从 ROB 中读取。
    // 寄存器设置 busy
    // slot busy，返回
    for (auto &slot : buffer) {
        if (slot.busy) {
            continue;
        }

        auto rs1 = inst.getRs1();

        if (regFile->isBusy(rs1)) {
            auto busyIndex = regFile->getBusyIndex(rs1);

            slot.readPort1.robIdx = busyIndex;
            if (reorderBuffer.checkReady(busyIndex)) {
                slot.readPort1.value = reorderBuffer.read(busyIndex);
                slot.readPort1.waitForWakeup = false;
            } else {
                slot.readPort1.waitForWakeup = true;
            }
        } else {
            slot.readPort1.value = regFile->read(rs1);
            slot.readPort1.waitForWakeup = false;
        }

        auto rs2 = inst.getRs2();
        if (regFile->isBusy(rs2)) {
            auto busyIndex = regFile->getBusyIndex(rs2);

            slot.readPort2.robIdx = busyIndex;
            if (reorderBuffer.checkReady(busyIndex)) {
                slot.readPort2.value = reorderBuffer.read(busyIndex);
                slot.readPort2.waitForWakeup = false;
            } else {
                slot.readPort2.waitForWakeup = true;
            }
        } else {
            slot.readPort2.value = regFile->read(rs2);
            slot.readPort2.waitForWakeup = false;
        }

        slot.inst = inst;
        slot.busy = true;
        slot.robIdx = robIdx;
        return;
    }
}

template <unsigned size>
void ReservationStation<size>::wakeup(
    [[maybe_unused]] const ROBStatusWritePort &x) {
    // Wakeup instructions according to ROB Write
    // 将 ROB 编号和寄存器数据写入总线, 与保留站中的有效项目进行比对
    // 如果有指令需要对应 rob 编号的数据作为自己的操作数的话, 则可以提供操作数, 唤醒指令
    for (auto &slot : buffer) {
        if (!slot.busy) {
            continue;
        }
        if (slot.readPort1.waitForWakeup && slot.readPort1.robIdx == x.robIdx) {
            slot.readPort1.value = x.result;
            slot.readPort1.waitForWakeup = false;
        }
        if (slot.readPort2.waitForWakeup && slot.readPort2.robIdx == x.robIdx) {
            slot.readPort2.value = x.result;
            slot.readPort2.waitForWakeup = false;
        }
    }
}

template <unsigned size>
bool ReservationStation<size>::canIssue() const {
    // Decide whether an issueSlot is ready to issue.
    // Warning: Store instructions must be issued in order!!
    for (auto &slot : buffer) {
        // Store
        if (slot.busy && (slot.inst == RV32I::SB || slot.inst == RV32I::SH || slot.inst == RV32I::SW)) {
            if (!slot.readPort1.waitForWakeup && !slot.readPort2.waitForWakeup) {
                return true;
            } else {
                return false;
            }
        }
        // Not Store
        else if (slot.busy) {
            if (!slot.readPort1.waitForWakeup && !slot.readPort2.waitForWakeup) {
                return true;
            }
        }
    }
    return false;
}

template <unsigned size>
IssueSlot ReservationStation<size>::issue() {
    // Issue a ready issue slot and remove it from reservation station.
    // Warning: Store instructions must be issued in order!!
    for (auto &slot : buffer) {
        // Store
        if (slot.busy && (slot.inst == RV32I::SB || slot.inst == RV32I::SH || slot.inst == RV32I::SW)) {
            if (!slot.readPort1.waitForWakeup && !slot.readPort2.waitForWakeup) {
                IssueSlot ret = slot;
                slot.busy = false;
                zip();
                return ret;
            } else {
                Logger::Error("No available slots for issuing");
                std::__throw_runtime_error("No available slots for issuing");
            }
        }
        // Not Store
        else if (slot.busy) {
            if (!slot.readPort1.waitForWakeup && !slot.readPort2.waitForWakeup) {
                IssueSlot ret = slot;
                slot.busy = false;
                zip();
                return ret;
            }
        }
    }
    Logger::Error("No available slots for issuing");
    std::__throw_runtime_error("No available slots for issuing");
}

template <unsigned size>
void ReservationStation<size>::flush() {
    for (auto &slot : buffer) {
        slot.busy = false;
    }
}

template <unsigned size>
void ReservationStation<size>::zip() {
    auto index = 0;
    IssueSlot zipped_buffer[size] = {};
    for (auto slot : buffer) {
        if (slot.busy) {
            zipped_buffer[index] = slot;
            ++index;
        }
    }
    for (unsigned i=0; i<size; ++i) {
        buffer[i] = zipped_buffer[i];
    }
}