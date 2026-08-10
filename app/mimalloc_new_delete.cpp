/*
 * Copyright (C) 2026 EloqData Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <mimalloc.h>

#include <cstdint>
#include <limits>
#include <new>

#include "keylane/memory.h"

namespace {

void AccountAllocation(void* pointer, bool allocated) noexcept {
  if (pointer == nullptr) {
    return;
  }
  const std::size_t usable = mi_usable_size(pointer);
  const std::int64_t bytes =
      usable >
              static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())
          ? std::numeric_limits<std::int64_t>::max()
          : static_cast<std::int64_t>(usable);
  keylane::AccountMemoryAllocation(allocated ? bytes : -bytes);
}

void* New(std::size_t size) {
  void* pointer = mi_new(size);
  AccountAllocation(pointer, true);
  return pointer;
}

void* NewNoThrow(std::size_t size) noexcept {
  void* pointer = mi_new_nothrow(size);
  AccountAllocation(pointer, true);
  return pointer;
}

void* NewAligned(std::size_t size, std::size_t alignment) {
  void* pointer = mi_new_aligned(size, alignment);
  AccountAllocation(pointer, true);
  return pointer;
}

void* NewAlignedNoThrow(std::size_t size, std::size_t alignment) noexcept {
  void* pointer = mi_new_aligned_nothrow(size, alignment);
  AccountAllocation(pointer, true);
  return pointer;
}

void Delete(void* pointer) noexcept {
  AccountAllocation(pointer, false);
  mi_free(pointer);
}

void DeleteAligned(void* pointer, std::size_t alignment) noexcept {
  AccountAllocation(pointer, false);
  mi_free_aligned(pointer, alignment);
}

}  // namespace

void* operator new(std::size_t size) { return New(size); }
void* operator new[](std::size_t size) { return New(size); }
void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
  return NewNoThrow(size);
}
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
  return NewNoThrow(size);
}

void operator delete(void* pointer) noexcept { Delete(pointer); }
void operator delete[](void* pointer) noexcept { Delete(pointer); }
void operator delete(void* pointer, const std::nothrow_t&) noexcept {
  Delete(pointer);
}
void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
  Delete(pointer);
}
void operator delete(void* pointer, std::size_t) noexcept { Delete(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { Delete(pointer); }

void* operator new(std::size_t size, std::align_val_t alignment) {
  return NewAligned(size, static_cast<std::size_t>(alignment));
}
void* operator new[](std::size_t size, std::align_val_t alignment) {
  return NewAligned(size, static_cast<std::size_t>(alignment));
}
void* operator new(std::size_t size, std::align_val_t alignment,
                   const std::nothrow_t&) noexcept {
  return NewAlignedNoThrow(size, static_cast<std::size_t>(alignment));
}
void* operator new[](std::size_t size, std::align_val_t alignment,
                     const std::nothrow_t&) noexcept {
  return NewAlignedNoThrow(size, static_cast<std::size_t>(alignment));
}

void operator delete(void* pointer, std::align_val_t alignment) noexcept {
  DeleteAligned(pointer, static_cast<std::size_t>(alignment));
}
void operator delete[](void* pointer, std::align_val_t alignment) noexcept {
  DeleteAligned(pointer, static_cast<std::size_t>(alignment));
}
void operator delete(void* pointer, std::size_t,
                     std::align_val_t alignment) noexcept {
  DeleteAligned(pointer, static_cast<std::size_t>(alignment));
}
void operator delete[](void* pointer, std::size_t,
                       std::align_val_t alignment) noexcept {
  DeleteAligned(pointer, static_cast<std::size_t>(alignment));
}
void operator delete(void* pointer, std::align_val_t alignment,
                     const std::nothrow_t&) noexcept {
  DeleteAligned(pointer, static_cast<std::size_t>(alignment));
}
void operator delete[](void* pointer, std::align_val_t alignment,
                       const std::nothrow_t&) noexcept {
  DeleteAligned(pointer, static_cast<std::size_t>(alignment));
}
