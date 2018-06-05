#pragma once

#include <algorithm>
#include <iostream>
#include <numeric>
// cstdint is a C++11 header. Weired.
#include <stdint.h>
#include <vector>

// Arbitrary precision integer class.
// Each cell of std::vector save 32 bits.
class BigInteger {
 public:
  BigInteger();
  BigInteger(const BigInteger &);
  BigInteger &operator=(const BigInteger &);
#if __cplusplus >= 201103L
  BigInteger(BigInteger &&);
  BigInteger &operator=(BigInteger &&);
#endif
  BigInteger(int64_t);
  // Only std::basic_{i,o}stream<char, std::char_traits<char> >
  // (a.k.a. std::istream, std::ostream) are useful in ACM-ICPC.
  friend std::ostream & operator<<(std::ostream &, const BigInteger &);
  friend std::istream &operator>>(std::istream &, BigInteger &);
  friend BigInteger &operator++(BigInteger &);
  friend BigInteger &operator--(BigInteger &);
  friend BigInteger &operator+=(BigInteger &, const BigInteger &);
  friend BigInteger &operator-=(BigInteger &, const BigInteger &);
  friend BigInteger &operator*=(BigInteger &, const BigInteger &);
  // TODO: operator/= and operator%=

  friend BigInteger &operator&=(BigInteger &, const BigInteger &);
  friend BigInteger &operator|=(BigInteger &, const BigInteger &);
  friend BigInteger &operator^=(BigInteger &, const BigInteger &);
  friend BigInteger operator~(const BigInteger &);

  friend bool operator==(const BigInteger &, const BigInteger &);
  friend bool operator<(const BigInteger &, const BigInteger &);
 private:
  std::vector<uint32_t> value;
  bool negative;
  const static uint64_t base;
  const static uint64_t mod;
  const static uint64_t root;

  // Time complexity of converting between bases is O(log²n).
  void getDec_(std::istream &);
  void putDec_(std::ostream &) const;
  void getHex_(std::istream &);
  void putHex_(std::ostream &) const;
  void getOct_(std::istream &);
  void putOct_(std::ostream &) const;
  void getBin_(std::istream &);
  void putBin_(std::ostream &) const;
  // Helper function for operator*=, /= and %=
  uint64_t modpow_(uint64_t, uint64_t);
  uint64_t modinv_(int64_t, int64_t);
  // Number Theory Transform.
  void NTT_(uint64_t, bool);

  void eliminateNegativeZero_();
  void trimLeadingZeros_();
};
const uint64_t BigInteger::base = static_cast<uint64_t>(1) << 32;
// With BigInteger::mod = (1 << 30) * 3 + 1,
// BigInteger can proccess 4GB data,
// which is fully adequate for ACM-ICPC.
const uint64_t BigInteger::mod = (1u << 30) * 3 + 1;
const uint64_t BigInteger::root = 5;

BigInteger::BigInteger()
    : value(1, 0), negative(false) {
}

BigInteger::BigInteger(const BigInteger &that)
    : value(that.value), negative(that.negative) {
}

BigInteger &BigInteger::operator=(const BigInteger &that) {
  this->value = that.value;
  this->negative = that.negative;
  return *this;
}

#if __cplusplus >= 201103L
BigInteger::BigInteger(BigInteger &&that)
    : value{std::move(that.value)}, negative(that.negative) {
}

BigInteger &BigInteger::operator=(BigInteger &&that) {
  this->value = std::move(that.value);
  this->negative = that.negative;
  return *this;
}
#endif

BigInteger::BigInteger(int64_t value)
    : negative(value < 0) {
  const int64_t base = static_cast<int64_t>(BigInteger::base);
  this->value.push_back(std::abs(value % base));
  if ((value /= base))
    this->value.push_back(std::abs(value));
}

std::istream &operator>>(std::istream &is, BigInteger &self) {
  char cursor;
  self.negative = false;
  // Discards spaces.
  is >> cursor;
  // Negative or positive sign.
  if (cursor == '-' || cursor == '+') {
    self.negative = cursor == '-';
    cursor = is.get();
  }
  is.unget();
  switch (is.flags() & std::ios::basefield) {
   case std::ios::hex:
    // Not implemented.
    break;
   case std::ios::dec:
    self.getDec_(is);
    break;
   case std::ios::oct:
    // Not implemented.
    break;
   default:  // case 0:
    // Not implemented.
    break;
  }
  return is;
}

std::ostream &operator<<(std::ostream &os, const BigInteger &self) {
  if (self.negative) os.put('-');
  switch (os.flags() & std::ios::basefield) {
   case std::ios::hex:
    // Not implemented.
    break;
   case std::ios::dec:
    self.putDec_(os);
    break;
   case std::ios::oct:
    // Not implemented.
    break;
   default:  // case 0:
    // Not implemented.
    break;
  }
  return os;
}

bool operator==(const BigInteger &lhs, const BigInteger &rhs) {
  if (lhs.negative != rhs.negative) return false;
  if (lhs.value.size() != rhs.value.size()) return false;
  return std::equal(lhs.value.begin(), lhs.value.end(), rhs.value.begin());
}

bool operator!=(const BigInteger &lhs, const BigInteger &rhs) {
  return !(lhs == rhs);
}

bool operator<(const BigInteger &lhs, const BigInteger &rhs) {
  if (lhs.negative != rhs.negative) return lhs.negative;
  if (lhs.value.size() != rhs.value.size())
    return lhs.value.size() < rhs.value.size();
  return lhs.negative ^ std::equal(lhs.value.rbegin(), lhs.value.rend(),
                                   rhs.value.rbegin(),
                                   std::less<uint32_t>());
}

bool operator>=(const BigInteger &lhs, const BigInteger &rhs) {
  return !(lhs < rhs);
}

bool operator>(const BigInteger &lhs, const BigInteger &rhs) {
  return rhs < lhs;
}

bool operator<=(const BigInteger &lhs, const BigInteger &rhs) {
  return !(rhs < lhs);
}

BigInteger &operator++(BigInteger &self) {
  typedef std::vector<uint32_t>::iterator Iter;
  Iter i = self.value.begin();
  if (!self.negative) {
    while (i != self.value.end() && !++*i++);
    if (*(i - 1) == 0) self.value.push_back(1);
  } else {
    while (i != self.value.end() && !~--*i++);
    self.trimLeadingZeros_();
  }
  return self;
}

BigInteger operator++(BigInteger &self, int) {
  BigInteger result = self; ++self; return result;
}

BigInteger &operator--(BigInteger &self) {
  typedef std::vector<uint32_t>::iterator Iter;
  Iter i = self.value.begin();
  if (!self.negative) {
    while (i != self.value.end() && !~--*i++);
    self.trimLeadingZeros_();
  } else {
    while (i != self.value.end() && !++*i++);
    if (*(i - 1) == 0) self.value.push_back(1);
  }
  return self;
}

BigInteger operator--(BigInteger &self, int) {
  BigInteger result = self; --self; return result;
}

BigInteger &operator+=(BigInteger &lhs, const BigInteger &rhs) {
  typedef std::vector<uint32_t>::iterator Iter;
  typedef std::vector<uint32_t>::const_iterator CIter;
  if (lhs.negative == rhs.negative) {
    lhs.value.reserve(rhs.value.size() + 1);
    if (lhs.value.size() < rhs.value.size())
      lhs.value.resize(rhs.value.size(), 0);
    Iter i = lhs.value.begin();
    CIter j = rhs.value.begin();
    uint64_t value = 0;
    while (j != rhs.value.end()) {
      value = value + *i + *j++;  // Promote to uint64_t.
      *i++ = value % BigInteger::base;
      value /= BigInteger::base;
    }
    while (value && i != lhs.value.end()) {
      value = value + *i;
      *i++ = value % BigInteger::base;
      value /= BigInteger::base;
    }
    if (value) lhs.value.push_back(value);
  } else {
    lhs.negative ^= 1;
    lhs -= rhs;
    lhs.negative ^= 1;
    lhs.eliminateNegativeZero_();
  }
  return lhs;
}

BigInteger operator+(const BigInteger &lhs, const BigInteger &rhs) {
  BigInteger result = lhs;
  return result += rhs;
}

BigInteger &operator-=(BigInteger &lhs, const BigInteger &rhs) {
  typedef std::vector<uint32_t>::iterator Iter;
  typedef std::vector<uint32_t>::const_iterator CIter;
  if (lhs.negative == rhs.negative) {
    lhs.value.reserve(rhs.value.size() + 1);
    if (lhs.value.size() < rhs.value.size())
      lhs.value.resize(rhs.value.size(), 0);
    Iter i = lhs.value.begin();
    CIter j = rhs.value.begin();
    int64_t value = 0;
    if ((lhs > rhs) ^ lhs.negative) {  // abs(lhs) > abs(rhs)
      while (j != rhs.value.end()) {
        value = value + *i - *j++;  // Promote to int64_t
        *i++ = value < 0 ? value + BigInteger::base : value;
        value = value < 0 ? -1 : 0;
      }
      while (value && i != lhs.value.end()) {
        value = value + *i;
        *i++ = value < 0 ? value + BigInteger::base : value;
        value = value < 0 ? -1 : 0;
      }
    } else {
      lhs.negative = !lhs.negative;
      while (j != rhs.value.end()) {
        value = value + *j++ - *i;
        *i++ = value < 0 ? value + BigInteger::base : value;
        value = value < 0 ? -1 : 0;
      }
    }
    lhs.trimLeadingZeros_();
  } else {
    lhs.negative ^= 1;
    lhs += rhs;
    lhs.negative ^= 1;
  }
  return lhs;
}

BigInteger operator-(const BigInteger &lhs, const BigInteger &rhs) {
  BigInteger result = lhs;
  return result -= rhs;
}

// TODO: run benchmark to determine the critical section where
// O(|I|²) multiply is less or as fast as O(|I|log|I|) one.
BigInteger &operator*=(BigInteger &lhs, const BigInteger &rhs) {
  BigInteger &l = lhs, r = rhs;
  uint64_t logN = 0, value;
  const size_t size = std::max(l.value.size(), r.value.size());
  while (static_cast<size_t>(1) << logN < size << 1) ++logN;
  const size_t n = static_cast<size_t>(1) << logN;
  l.value.resize(n); l.NTT_(logN, false);
  r.value.resize(n); r.NTT_(logN, false);
  for (size_t i = 0; i != n; ++i) {
    value = static_cast<uint64_t>(l.value[i]) * r.value[i];
    l.value[i] = value % BigInteger::mod;
  }
  // TODO: lazy evaluation, optimize for continually multiplying.
  l.NTT_(logN, true);
  l.trimLeadingZeros_();
  return lhs;
}

BigInteger operator*(const BigInteger &lhs, const BigInteger &rhs) {
  BigInteger result = lhs;
  return result *= rhs;
}

BigInteger &operator&=(BigInteger &lhs, const BigInteger &rhs) {
  if (lhs.value.size() > rhs.value.size())
    lhs.value.resize(rhs.value.size());
  lhs.negative &= rhs.negative;
  std::transform(lhs.value.begin(), lhs.value.end(),
                 rhs.value.begin(), lhs.value.begin(),
                 std::bit_and<uint32_t>());
  lhs.trimLeadingZeros_();
  return lhs;
}

BigInteger operator&(const BigInteger &lhs, const BigInteger &rhs) {
  BigInteger result = lhs;
  return result &= rhs;
}

BigInteger &operator|=(BigInteger &lhs, const BigInteger &rhs) {
  if (lhs.value.size() < rhs.value.size())
    lhs.value.resize(rhs.value.size());
  lhs.negative |= rhs.negative;
  std::transform(lhs.value.begin(), lhs.value.end(),
                 rhs.value.begin(), lhs.value.begin(),
                 std::bit_or<uint32_t>());
  return lhs;
}

BigInteger operator|(const BigInteger &lhs, const BigInteger &rhs) {
  BigInteger result = lhs;
  return result |= rhs;
}

BigInteger &operator^=(BigInteger &lhs, const BigInteger &rhs) {
  const size_t size = std::min(lhs.value.size(), rhs.value.size());
  if (lhs.value.size() == size)
    lhs.value.insert(lhs.value.end(),
                     rhs.value.begin() + size, rhs.value.end());
  lhs.negative ^= rhs.negative;
  std::transform(lhs.value.begin(), lhs.value.begin() + size,
                 rhs.value.begin(), lhs.value.begin(),
                 std::bit_xor<uint32_t>());
  lhs.trimLeadingZeros_();
  return lhs;
}

BigInteger operator^(const BigInteger &lhs, const BigInteger &rhs) {
  BigInteger result = lhs;
  return result ^= rhs;
}

BigInteger operator~(const BigInteger &self) {
  BigInteger result = self;
  ++result;
  result.negative ^= 1;
  return result;
}

void BigInteger::getDec_(std::istream &is) {
  typedef std::vector<uint32_t>::iterator Iter;
  typedef std::vector<uint32_t>::reverse_iterator RevIter;
  const static uint_fast32_t logBase = 9, pow10[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000,
  };
  this->value.clear();
  if (!isdigit(is.peek())) {
    this->negative = false;
    this->value.push_back(0);
    is.clear(std::ios::failbit);
    return;
  }
  std::vector<uint32_t> decimal;
  // Discard leading zeros.
  while (is.peek() == '0') is.get();
  // Read from most significant digit.
  uint64_t value = 0, logLen = 0;
  while (isdigit(is.peek())) {
    int cursor = is.get();
    value = value * 10 + cursor - '0';
    if (++logLen == logBase) {
      decimal.push_back(value);
      value = 0; logLen = 0;
    }
  }
  // Save in least significant digit first, adjust.
  if (logLen != 0) {
    decimal.push_back(value);
    const uint_fast32_t shift = pow10[logBase - logLen];
    const uint_fast32_t len = pow10[logLen];
    RevIter next = decimal.rbegin(), prev = next++;
    while (next != decimal.rend()) {
      *prev++ += *next % shift * len;
      *next++ /= shift;
    }
  }
  if (decimal.empty()) {
    this->negative = false;
    this->value.push_back(0);
    return;
  }
  this->value.reserve(decimal.size());
  // Convert from base-1000000000 to base-4294967294.
  Iter head = decimal.begin();
  while (head + 1 != decimal.end()) {
    value = *head++;
    for (Iter i = head; i != decimal.end(); ++i) {
      value = value * pow10[logBase] + *i;
      *i = value / BigInteger::base;
      value %= BigInteger::base;
    }
    this->value.push_back(value);
  }
  if (*head) this->value.push_back(*head);
}

void BigInteger::putDec_(std::ostream &os) const {
  typedef std::vector<uint32_t>::reverse_iterator RevIter;
  const static uint_fast32_t logBase = 9, pow10[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000,
  };
  if (this->value.size() == 1 && this->value.front() == 0) {
    os.put('0');
    return;
  }
  // Convert from base-4294967294 to base-1000000000.
  std::vector<uint32_t> binary = this->value, decimal;
  RevIter head = binary.rbegin();
  while (head + 1 != binary.rend()) {
    uint64_t value = 0;
    for (RevIter i = head; i != binary.rend(); ++i) {
      value = value * BigInteger::base | *i;
      *i = value / pow10[logBase];
      value %= pow10[logBase];
    }
    decimal.push_back(value);
    if (!*head) ++head;
  }
  if (*head) decimal.push_back(*head);
  os << decimal.back();
  decimal.pop_back();
  for (RevIter i = decimal.rbegin(); i != decimal.rend(); ++i) {
    for (int_fast8_t j = logBase - 1; j != 0; --j)
      if (*i < pow10[j]) os.put('0'); else break;
    os << *i;
  }
}

uint64_t BigInteger::modpow_(uint64_t base, uint64_t expo) {
  uint64_t retn = 1;
  do {
    if (expo & 1) retn = retn * base % mod;
    if (expo ^ 1) base = base * base % mod;
  } while (expo >>= 1);
  return retn;
}

uint64_t BigInteger::modinv_(int64_t m, int64_t n) {
  int64_t x = 0, y = 1, xp = 1, yp = 0, xt, yt, mod = n;
  for (int64_t r = m % n, q = m / n; r; ) {
    xt = xp - q * x, xp = x, x = xt;
    yt = yp - q * y, yp = y, y = yt;
    m = n, n = r, r = m % n, q = m / n;
  }
  return x + mod;
}

void BigInteger::NTT_(uint64_t logN, bool inverse) {
  // Initialization of unit roots w.
  const uint64_t n = this->value.size();
  const uint64_t g = modpow_(root, mod >> logN);
  uint64_t *const w = new uint64_t[n];
  for (size_t i = 0; i != n; ++i)
    w[i] = i ? w[i - 1] * g % mod : 1;
  if (inverse) {
    std::reverse(w + 1, w + n);
    const uint64_t inv = modinv_(n, mod);
    for (size_t i = 0; i != n; ++i)
      this->value[i] = this->value[i] * inv % mod;
  }
  // Main part of transforming.
  for (size_t i = 0, j = 0; i != n; ++i) {
    if (i > j) std::swap(this->value[i], this->value[j]);
    for (size_t k = n; j < k; j ^= k) k >>= 1;
  }
  for (size_t i = 1, I = n >> 1; I; i <<= 1, I >>= 1)
    for (size_t j = 0; j != n; j += i << 1)
      for (size_t k = 0; k != i; ++k) {
        const uint64_t z = this->value[i + j + k] * w[I * k] % mod;
        if (this->value[j + k] < z)
          this->value[i + j + k] = this->value[j + k] + mod - z;
        else this->value[i + j + k] = this->value[j + k] - z;
        if (this->value[j + k] + z < mod) this->value[j + k] += z;
        else this->value[j + k] = this->value[j + k] + z - mod;
      }
  delete[] w;
}

void BigInteger::eliminateNegativeZero_() {
  if (this->value.size() == 1 && !this->value.front())
    this->negative = false;
}

void BigInteger::trimLeadingZeros_() {
  size_t len = this->value.size();
  while (!this->value[len] && len) --len;
  this->value.resize(len + 1);
  this->eliminateNegativeZero_();
}
