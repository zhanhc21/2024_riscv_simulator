#include "store_buffer.h"

#include <stdexcept>

#include "logger.h"

StoreBuffer::StoreBuffer() {
    pushPtr = popPtr = 0;
    for (auto &slot : buffer) {
        slot.valid = false;
    }
}

/**
 * @brief 向ROB推入新项
 *
 * @param addr store地址
 * @param value store值
 */
void StoreBuffer::push(unsigned addr, unsigned value, unsigned robIdx) {
    buffer[pushPtr].storeAddress = addr;
    buffer[pushPtr].storeData = value;
    buffer[pushPtr].robIdx = robIdx;
    buffer[pushPtr].valid = true;
    Logger::Info("Store buffer push:");
    Logger::Info("Index: %u", pushPtr);
    Logger::Info("Address: %08x, value: %u\n", addr, value);
    pushPtr++;
    if (pushPtr == ROB_SIZE) {
        pushPtr -= ROB_SIZE;
    }
}

/**
 * @brief 从StoreBuffer中弹出一项，需保证Store Buffer中有项目
 *
 * @return StoreBufferSlot 弹出的StoreBuffer项目
 */
StoreBufferSlot StoreBuffer::pop() {
    auto ret = buffer[popPtr];
    Logger::Info("Store buffer pop:");
    Logger::Info("Index: %u", popPtr);
    Logger::Info("Address: %08x, value: %u\n", ret.storeAddress, ret.storeData);
    buffer[popPtr].valid = false;
    popPtr++;
    if (popPtr == ROB_SIZE) {
        popPtr -= ROB_SIZE;
    }
    return ret;
}

/**
 * @brief 返回 Store Buffer 队列首项
 *
 * @return StoreBufferSlot
 */
StoreBufferSlot StoreBuffer::front() { return buffer[popPtr]; }

/**
 * @brief 清空Store Buffer状态
 *
 */
void StoreBuffer::flush() {
    pushPtr = popPtr = 0;
    for (auto &slot : buffer) {
        slot.valid = false;
    }
}

/**
 * @brief 查询StoreBuffer中的内容
 *
 * @param addr 地址
 * @return std::optional<unsigned> 如果在Store
 * Buffer中命中，返回对应值，否则返回std::nullopt
 */
std::optional<unsigned> StoreBuffer::query(
    [[maybe_unused]] unsigned addr,
    [[maybe_unused]] unsigned robIdx,
    [[maybe_unused]] unsigned robPopPtr) {
    // 需要倒序遍历 Store Buffer 的全部项目, 找到第一个 rob 顺序（注意不是序号）在当前指令之前, 且地址相同的条目进行返回
    auto tail = pushPtr - 1;
    if (pushPtr == 0) {
        tail = ROB_SIZE - 1;
    }
    unsigned mask = 0xFFFFFFFC;
    bool mark_current = false;
    while (tail >= popPtr) {
        if ((buffer[tail].storeAddress & mask) == (addr & mask) &&
            buffer[tail].valid &&
            mark_current) {
            return std::make_optional(buffer[tail].storeData);
        }
        if (buffer[tail].robIdx == robIdx && buffer[tail].valid) {
            mark_current = true;
        }
        --tail;
    }
    return std::nullopt;
}
