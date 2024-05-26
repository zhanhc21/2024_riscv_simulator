#include <stdexcept>
#include "logger.h"
#include "processor.h"

/**
 * @brief 处理前端流出的指令
 *
 * @param inst 前端将要流出的指令（在流出buffer里面）
 * @return true 后端接受该指令
 * @return false 后端拒绝该指令
 */
bool Backend::dispatchInstruction([[maybe_unused]] const Instruction &inst) {
    // NOTE: use getFUType to get instruction's target FU
    // NOTE: FUType::NONE only goes into ROB but not Reservation Stations
    // Check rob
    if (!rob.canPush()) {
        // Logger::Error("ROB can't push");
        return false;
    }

    unsigned int robIdx = 0;
    switch (getFUType(inst)) {
    case FUType::ALU:
        // Check reservation station
        if (!rsALU.hasEmptySlot()) {
            return false;
        }
        // Insert ROB
        robIdx = rob.push(inst, false);
        // Insert reservation station
        rsALU.insertInstruction(inst, robIdx, regFile, rob);
        // Update register status
        regFile->markBusy(inst.getRd(), robIdx);
        break;
    case FUType::BRU:
        if (!rsBRU.hasEmptySlot()) {
            return false;
        }
        robIdx = rob.push(inst, false);
        rsBRU.insertInstruction(inst, robIdx, regFile, rob);
        regFile->markBusy(inst.getRd(), robIdx);
        break;
    case FUType::LSU:
        if (!rsLSU.hasEmptySlot()) {
            return false;
        }
        robIdx = rob.push(inst, false);
        rsLSU.insertInstruction(inst, robIdx, regFile, rob);
        regFile->markBusy(inst.getRd(), robIdx);
        break;
    case FUType::MUL:
        if (!rsMUL.hasEmptySlot()) {
            return false;
        }
        robIdx = rob.push(inst, false);
        rsMUL.insertInstruction(inst, robIdx, regFile, rob);
        regFile->markBusy(inst.getRd(), robIdx);
        break;
    case FUType::DIV:
        if (!rsDIV.hasEmptySlot()) {
            return false;
        }
        robIdx = rob.push(inst, false);
        rsDIV.insertInstruction(inst, robIdx, regFile, rob);
        regFile->markBusy(inst.getRd(), robIdx);
        break;
    case FUType::NONE:
        rob.push(inst, true);
        break;
    }

    std::stringstream ss;
    ss << inst;
    Logger::Info("Dispatch %s in ROB %d", ss.str().c_str(), robIdx);
    return true;
}

/**
 * @brief 后端完成指令提交
 *
 * @param entry 被提交的 ROBEntry
 * @param frontend 前端，用于调用前端接口
 * @return true 提交了 EXTRA::EXIT
 * @return false 其他情况
 */
//bool Backend::commitInstruction([[maybe_unused]] const ROBEntry &entry,
//                                [[maybe_unused]] Frontend &frontend) {
//    // NOTE: Be careful about Store Buffer!
//    // NOTE: Re-executing load instructions when it is invalidated in load
//    // buffer. NOTE: Be careful about flush!
//
//    // Optional TODO: Update your BTB when necessary
//
//    // Set executeExit when committing EXTRA::EXIT
//    if (entry.inst == EXTRA::EXIT) {
//        Logger::Info("Commit Exit\n");
//        rob.pop();
//        return true;
//    }
//    // S type
//    // 查询store buffer, 传入 指令地址 / rob id / rob pop ptr, 写入内存
//    // 注意内存可能不能在 1 周期之内完成写入. 如果写入未完成, 请不要弹出 rob
//    // 弹出 store buffer & rob, 提交
//    if (entry.inst == RV32I::SB || entry.inst == RV32I::SH ||
//        entry.inst == RV32I::SW) {
//        StoreBufferSlot slot = storeBuffer.front();
//        bool status =
//            writeMemoryHierarchy(slot.storeAddress, slot.storeData, 0xF);
//        if (status) {
//            storeBuffer.pop();
//            rob.pop();
//        } else {
//            Logger::Warn("Write mem not finish");
//        }
//    }
//    // L type
//    // 弹出 Load Buffer 项，如果失效，转 2，否则转 3
//    // 跳转到当前pc, 重新执行 Load
//    // 写入寄存器, 清除 Load Buffer, 弹出 rob. 提交
//    else if (entry.inst == RV32I::LB || entry.inst == RV32I::LH ||
//             entry.inst == RV32I::LW || entry.inst == RV32I::LBU ||
//             entry.inst == RV32I::LHU) {
//        auto robIdx = rob.getPopPtr();
//        auto slot = loadBuffer.pop(robIdx);
//        if (slot.invalidate) {
//            Logger::Info("Invalidate");
//            frontend.jump(entry.inst.pc);
//            flush();
//        }
//        else {
//            regFile->write(entry.inst.getRd(), entry.state.result, slot.robIdx);
//            Logger::Info("Load write reg %u, value %08x, rob %u", entry.inst.getRd(),
//                         entry.state.result, robIdx);
//            loadBuffer.flush();
//        }
//        rob.pop();
//    }
//    // B type
//    // If mis predict, jump & flush
//    else if (entry.inst == RV32I::BEQ || entry.inst == RV32I::BNE ||
//             entry.inst == RV32I::BLT || entry.inst == RV32I::BGE ||
//             entry.inst == RV32I::BLTU || entry.inst == RV32I::BGEU) {
//        if (entry.state.mispredict && entry.state.actualTaken) {
//            frontend.jump(entry.state.jumpTarget);
//            flush();
//        } else {
//            Logger::Info("Do not jump");
//            rob.pop();
//        }
//    }
//    // Else type
//    else {
//        auto robIdx = rob.getPopPtr();
//        Logger::Info("Alu write reg %u, value %08x, rob %u", entry.inst.getRd(),
//                     entry.state.result, robIdx);
//        regFile->write(entry.inst.getRd(), entry.state.result, robIdx);
//        rob.pop();
//    }
//    std::stringstream ss;
//    ss << entry.inst;
//    Logger::Info("Commit inst %s\n", ss.str().c_str());
//    return false;
//}

bool Backend::commitInstruction([[maybe_unused]] const ROBEntry &entry,
                                [[maybe_unused]] Frontend &frontend) {
    bool executeExit = false;
    using namespace RV32I;

    StoreBufferSlot stSlot{};
    LoadBufferSlot ldSlot{};

    ldSlot.valid = false;
    ldSlot.invalidate = false;

    if (entry.inst == SB || entry.inst == SH || entry.inst == SW) {
        stSlot = storeBuffer.front();
        bool status =
            writeMemoryHierarchy(stSlot.storeAddress, stSlot.storeData, 0xF);
        if (!status) {
            return false;
        } else {
            storeBuffer.pop();
        }
    } else if (entry.inst == LB || entry.inst == LBU || entry.inst == LH ||
               entry.inst == LHU || entry.inst == LW) {
        ldSlot = loadBuffer.pop(rob.getPopPtr());
        if (ldSlot.invalidate) {
            frontend.jump(entry.inst.pc);
            flush();
            return false;
        }
    }

    if (entry.inst == EXTRA::EXIT) {
        executeExit = true;
    }

    regFile->write(entry.inst.getRd(), entry.state.result, rob.getPopPtr());
    rob.pop();
    if (entry.state.mispredict) {
        frontend.jump(entry.state.actualTaken ? entry.state.jumpTarget : entry.inst.pc + 4);
        flush();
    }
    if (entry.inst == BEQ || entry.inst == BNE || entry.inst == BLT ||
        entry.inst == BGE || entry.inst == BLTU || entry.inst == BGEU) {
        BpuUpdateData data{};
        data.pc = entry.inst.pc;
        data.jumpTarget = entry.state.jumpTarget;
        data.isBranch = true;
        data.branchTaken = entry.state.actualTaken;
        frontend.bpuBackendUpdate(data);
    }

    return executeExit;
}