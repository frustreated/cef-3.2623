// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_RAPPOR_BLOOM_FILTER_H_
#define COMPONENTS_RAPPOR_BLOOM_FILTER_H_

#include <stdint.h>

#include <string>

#include "base/macros.h"
#include "components/rappor/byte_vector_utils.h"

namespace rappor {

// BloomFilter is a simple Bloom filter for keeping track of a set of strings.
class BloomFilter {
 public:
  // Constructs a BloomFilter using |bytes_size| bytes of Bloom filter bits,
  // and |hash_function_count| hash functions to set bits in the filter. The
  // hash functions will be generated by using seeds in the range
  // |hash_seed_offset| to (|hash_seed_offset| + |hash_function_count|).
  BloomFilter(uint32_t bytes_size,
              uint32_t hash_function_count,
              uint32_t hash_seed_offset);
  ~BloomFilter();

  // Sets the Bloom filter bits to contain a single string.
  void SetString(const std::string& str);

  // Returns the current value of the Bloom filter's bit array.
  const ByteVector& bytes() const { return bytes_; };

  // Sets bytes for testing purposes.
  void SetBytesForTesting(const ByteVector& bytes);

 private:
  // Stores the byte array of the Bloom filter.
  ByteVector bytes_;

  // The number of bits to set for each string added.
  uint32_t hash_function_count_;

  // A number add to a hash function index to get a seed for that hash function.
  uint32_t hash_seed_offset_;

  DISALLOW_COPY_AND_ASSIGN(BloomFilter);
};


namespace internal {

// A function for getting bloom filters less than or equal to 64 bits.
uint64_t GetBloomBits(uint32_t bytes_size,
                      uint32_t hash_function_count,
                      uint32_t hash_seed_offset,
                      const std::string& str);

}  // namespace internal

}  // namespace rappor

#endif  // COMPONENTS_RAPPOR_BLOOM_FILTER_H_
