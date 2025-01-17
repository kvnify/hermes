/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_UNITTESTS_VMRUNTIME_ADDITIONALSLOTSTEST_H
#define HERMES_UNITTESTS_VMRUNTIME_ADDITIONALSLOTSTEST_H

#include "gtest/gtest.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/PrimitiveBox.h"

namespace hermes {
namespace vm {

template <typename T>
size_t numAdditionalSlotsForTest() {
  // Allocate the maximum possible number of anonymous property slots, this
  // ensures that we use both direct and indirect storage.
  static_assert(
      InternalProperty::NumInternalProperties > JSObject::DIRECT_PROPERTY_SLOTS,
      "Must use both direct and indirect prop storage.");
  constexpr size_t numAdditionalSlots =
      InternalProperty::NumInternalProperties - T::ANONYMOUS_PROPERTY_SLOTS -
      JSObject::numOverlapSlots<T>();
  static_assert(
      numAdditionalSlots > 1, "At least 2 properties needed for this test");
  return numAdditionalSlots;
}

template <typename T>
void testAdditionalSlots(Runtime *runtime, Handle<T> handle) {
  size_t numAdditionalSlots = numAdditionalSlotsForTest<T>();
  PseudoHandle<JSNumber> boxedNum = JSNumber::create(
      runtime, 3.14, Handle<JSObject>::vmcast(&runtime->numberPrototype));
  T::setAdditionalSlotValue(*handle, runtime, 0, boxedNum.getHermesValue());

  for (size_t i = 1; i < numAdditionalSlots; i++)
    T::setAdditionalSlotValue(
        *handle, runtime, i, HermesValue::encodeNumberValue(i));

  // Verify slot values survive GC.
  runtime->collect("test");
  JSNumber *n =
      vmcast<JSNumber>(T::getAdditionalSlotValue(*handle, runtime, 0));
  EXPECT_EQ(JSNumber::getPrimitiveValue(n).getNumber(), 3.14);

  for (size_t i = 1; i < numAdditionalSlots; i++)
    EXPECT_EQ(T::getAdditionalSlotValue(*handle, runtime, i).getNumber(), i);
}

} // namespace vm
} // namespace hermes

#endif // HERMES_UNITTESTS_VMRUNTIME_ADDITIONALSLOTSTEST_H
