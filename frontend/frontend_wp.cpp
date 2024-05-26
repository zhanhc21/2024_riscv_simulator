#include "processor.h"
#include "with_predict.h"

FrontendWithPredict::FrontendWithPredict(const std::vector<unsigned> &inst)
    : Frontend(inst) {
    for (auto &entry : btb) {
        entry.valid = false;
    }
}

/**
 * @brief 获取指令的分支预测结果，分支预测时需要
 *
 * @param pc 指令的pc
 * @return BranchPredictBundle 分支预测的结构
 */
BranchPredictBundle FrontendWithPredict::bpuFrontendUpdate(unsigned int pc) {
    // branch predictions
    // 取低8位作为index
    auto index = pc & 0xFF;
    BranchPredictBundle ret;
    ret.predictJump = false;
    if (btb[index].valid && btb[index].pc == pc) {
        if (btb[index].state == STRONG_TAKEN || btb[index].state == WEAK_TAKEN) {
            ret.predictJump = true;
            ret.predictTarget = calculateNextPC(pc);
        }
    }
    return ret;
}

/**
 * @brief 用于计算NextPC，分支预测时需要
 *
 * @param pc
 * @return unsigned
 */
unsigned FrontendWithPredict::calculateNextPC(unsigned pc) const {
    // branch predictions
    auto index = pc & 0xFF;
    unsigned next_pc = pc + 4;
    if (btb[index].valid && btb[index].pc == pc) {
        if (btb[index].state == STRONG_TAKEN || btb[index].state == WEAK_TAKEN) {
            next_pc = btb[index].target;
        }
    }
    return next_pc;
}

/**
 * @brief 用于后端提交指令时更新分支预测器状态，分支预测时需要
 *
 * @param x
 */
void FrontendWithPredict::bpuBackendUpdate(const BpuUpdateData &x) {
    // Optional branch predictions
        auto index = x.pc & 0xFF;
        if (btb[index].valid && btb[index].pc == x.pc) {
            if (x.branchTaken) {
                switch (btb[index].state) {
                    case STRONG_NOT_TAKEN:
                        btb[index].state = WEAK_NOT_TAKEN;
                        break;
                    case WEAK_NOT_TAKEN:
                        btb[index].state = WEAK_TAKEN;
                        break;
                    case WEAK_TAKEN:
                        btb[index].state = STRONG_TAKEN;
                        break;
                    case STRONG_TAKEN:
                        break;
                }
            }
            else {
                switch (btb[index].state) {
                    case STRONG_NOT_TAKEN:
                        break;
                    case WEAK_NOT_TAKEN:
                        btb[index].state = STRONG_NOT_TAKEN;
                        break;
                    case WEAK_TAKEN:
                        btb[index].state = WEAK_NOT_TAKEN;
                        break;
                    case STRONG_TAKEN:
                        btb[index].state = WEAK_TAKEN;
                        break;
                }
            }
            btb[index].target = x.jumpTarget;
        }
        else {
            btb[index].state = x.branchTaken ? WEAK_TAKEN : WEAK_NOT_TAKEN;
            btb[index].valid = true;
            btb[index].pc = x.pc;
            btb[index].target = x.jumpTarget;
        }
}

/**
 * @brief 重置前端状态
 *
 * @param inst
 * @param entry
 */
void FrontendWithPredict::reset(const std::vector<unsigned int> &inst,
                                unsigned int entry) {
    Frontend::reset(inst, entry);
    // Optional TODO: Do your reset here
}
