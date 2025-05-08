/*
    typed bit flags
    Copyright (C) 2025 zlc-dev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
*/

#pragma once

template <typename Derived, typename BitsType>
struct BitFlagsBase {
private:
    BitsType bits;
protected:
    inline constexpr BitFlagsBase(BitsType t) :bits(t) {}
    inline static constexpr BitFlagsBase crate(BitsType t) { return t; }
    inline BitsType& get_bits() {
        return bits;
    }
    inline const BitsType& get_bits() const {
        return bits;
    }
public:
    inline bool constains(const BitFlagsBase& oth) {
        return (get_bits() & oth.get_bits()) != 0;
    }
    inline Derived& add(const BitFlagsBase& oth) {
        get_bits() |= oth.get_bits();
        return *static_cast<BitFlagsBase*>(this);
    }
    inline Derived& remove(const BitFlagsBase& oth) {
        get_bits() &= ~oth.get_bits();
        return *static_cast<Derived*>(this);
    }
    inline Derived operator&(const BitFlagsBase& oth) const {
        return Derived { get_bits() & oth.get_bits() };
    }
    inline Derived operator|(const BitFlagsBase& oth) const {
        return Derived { get_bits() | oth.get_bits() };
    }
    inline Derived operator^(const BitFlagsBase& oth) const {
        return Derived { get_bits() ^ oth.get_bits() };
    }
    inline Derived operator~() {
        return Derived { ~get_bits()};
    }
    inline Derived& operator&=(const BitFlagsBase& oth) {
        get_bits() &= oth.get_bits();
        return *static_cast<Derived*>(this);
    }
    inline Derived& operator|=(const BitFlagsBase& oth) {
        get_bits() |= oth.get_bits();
        return *static_cast<Derived*>(this);
    }
    inline Derived& operator^=(const BitFlagsBase& oth) {
        get_bits() ^= oth.get_bits();
        return *static_cast<Derived*>(this);
    }
    inline bool operator==(const BitFlagsBase& oth) const {
        return get_bits() == oth.get_bits();
    }
    inline bool is_empty() const {
        return get_bits() == 0;
    }
};

#define BEGIN_BIT_TAGS(NAME, TYPE, EMPTY) class NAME: public BitFlagsBase<NAME, TYPE> {\
private:\
    friend struct BitFlagsBase<NAME, TYPE>;\
    using Base = BitFlagsBase<NAME, TYPE>;\
    using BitsType = TYPE;\
public:\
    inline NAME() : BitFlagsBase<NAME, TYPE>{(EMPTY)} {}\
    inline NAME(BitFlagsBase<NAME, TYPE> t) : BitFlagsBase<NAME, TYPE>{t} {}\
    inline static NAME empty() { return {}; }

#define DEF_BIT_TAG(NAME, BIT) public: \
    inline static constexpr Base NAME = Base::crate(BitsType {1 << (BIT)})\

#define DEF_TAG(NAME, TAG) public: \
    inline static constexpr Base NAME = Base::crate(BitsType { (TAG) })\

#define END_BIT_TAGS }
