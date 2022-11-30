#pragma once

#include "common/common.hpp"

template<typename IdType, typename TargetType>
struct ArrayIndex {
    using Type = TargetType;

    bool valid() { return this->value != -1; }

    i32 value = -1;
};

template<typename IdType>
struct ArrayIndex<IdType, void> {
    using Type = void;

    ArrayIndex() = default;

    template<typename U>
    ArrayIndex(const ArrayIndex<IdType, U> &other)
        : value(other.value)
        , type(U::TYPE) {
    }

    bool IsValid() const { return this->value != -1; }

    i32 value = -1;
    IdType type{};
};

template<typename IdType, typename TargetType>
struct IndexArray {
    ArrayIndex<IdType, TargetType> Push() {
        ArrayIndex<IdType, TargetType> res;
        res.value = static_cast<i32>(this->values.size());
        this->values.emplace_back();
        return res;
    }

    ArrayIndex<IdType, TargetType> Push(TargetType &&value) {
        ArrayIndex<IdType, TargetType> res;
        res.value = static_cast<i32>(this->values.size());
        this->values.emplace_back(ToRvalue(value));
        return res;
    }

    template<typename U>
    TargetType &Get(ArrayIndex<IdType, U> index) {
        if constexpr (std::is_same_v<U, void>) {
            if (index.type != TargetType::TYPE) {
                FAIL("Index type mismatch");
            }
        }

        return this->values.at(index.value);
    }

    template<typename U>
    const TargetType &Get(ArrayIndex<IdType, U> index) const {
        if constexpr (std::is_same_v<U, void>) {
            if (index.type != TargetType::TYPE) {
                FAIL("Index type mismatch");
            }
        }

        return this->values.at(index.value);
    }

    Array<TargetType> values;
};

