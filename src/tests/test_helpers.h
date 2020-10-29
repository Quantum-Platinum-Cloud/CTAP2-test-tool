// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TESTS_TEST_HELPERS_H_
#define TESTS_TEST_HELPERS_H_

#include "src/cbor_builders.h"
#include "src/command_state.h"
#include "src/device_interface.h"
#include "src/device_tracker.h"
#include "third_party/chromium_components_cbor/values.h"

namespace fido2_tests {

namespace test_helpers {

// Returns a PIN that is different from the PIN set on the device. This is
// enforced in SetPin() by making sure the chosen PIN is different.
cbor::Value::BinaryValue BadPin();

// Extracts the credential ID from an authenticator data structure[1].
// [1] https://www.w3.org/TR/webauthn/#sec-authenticator-data
cbor::Value::BinaryValue ExtractCredentialId(const cbor::Value& response);

// Sends a ClientPin command to get the PIN retries.
absl::variant<cbor::Value, Status> GetPinRetriesResponse(
    DeviceInterface* device, DeviceTracker* device_tracker);

// Extracts the PIN retries from an authenticator client PIN response.
int ExtractPinRetries(const cbor::Value& response);

void PrintNoTouchPrompt();

// TODO(#16) replace version string with FIDO_2_1 when specification is final
bool IsFido2Point1Complicant(DeviceTracker* device_tracker);

// Makes a credential for all tests that require one, for example assertions.
cbor::Value MakeTestCredential(DeviceTracker* device_tracker,
                               CommandState* command_state,
                               const std::string& rp_id,
                               bool use_residential_key);

// The following helper functions are used to test input parameters. All return
// an error message, if a test fails.

// Tries to insert types other than the correct one into the CBOR builder.
// Make sure to pass the appropriate CborBuilder for your command. The correct
// types are inferred through the currently present builder entries. The tests
// include other types than maps for the command and inner types of maps and
// the first element of an inner array (assuming all array elements have the
// same type). If that first element happens to be a map, its entries are also
// checked. Even though this seems like an arbitrary choice, it covers most of
// the CTAP input.
std::optional<std::string> TestBadParameterTypes(DeviceInterface* device,
                                                 DeviceTracker* device_tracker,
                                                 Command command,
                                                 CborBuilder* builder);

// Tries to remove each parameter once. Make sure to pass the appropriate
// CborBuilder for your command. The necessary parameters are inferred through
// the currently present builder entries.
std::optional<std::string> TestMissingParameters(DeviceInterface* device,
                                                 DeviceTracker* device_tracker,
                                                 Command command,
                                                 CborBuilder* builder);

// Tries to insert types other than the correct one into map entries. Those
// maps themselves are values of the command parameter map. If
// has_wrapping_array is true, the inner map is used as an array element
// instead. To sum it up, the data structure tested can look like this:
// command:outer_map_key->inner_map[key]->wrongly_typed_value or
// command:outer_map_key->[inner_map[key]->wrongly_typed_value].
std::optional<std::string> TestBadParametersInInnerMap(
    DeviceInterface* device, DeviceTracker* device_tracker, Command command,
    CborBuilder* builder, int outer_map_key,
    const cbor::Value::MapValue& inner_map, bool has_wrapping_array);

// Tries to insert types other than the correct one into array elements. Those
// arrays themselves are values of the command parameter map.
std::optional<std::string> TestBadParametersInInnerArray(
    DeviceInterface* device, DeviceTracker* device_tracker, Command command,
    CborBuilder* builder, int outer_map_key, const cbor::Value& array_element);

// Tries to insert a map or an array as a transport in an array of public key
// credential descriptors. Both excludeList in MakeCredential and allowList in
// GetAssertion expect this kind of value and share this test. Authenticators
// must ignore unknown items in the transports list, so unexpected types are
// untested. For arrays and maps though, the maximum nesting depth is reached.
std::optional<std::string> TestCredentialDescriptorsArrayForCborDepth(
    DeviceInterface* device, DeviceTracker* device_tracker, Command command,
    CborBuilder* builder, int map_key, const std::string& rp_id);

// The following helper functions are used to test command behaviour.

// Gets and checks the PIN retry counter response from the authenticator.
int GetPinRetries(DeviceInterface* device, DeviceTracker* device_tracker);

// Checks if the PIN we currently assume is set works for getting an auth
// token. This way, we don't have to trust only the returned status code
// after a SetPin or ChangePin command. It does not actually return an auth
// token, use GetAuthToken() in that case.
void CheckPinByGetAuthToken(DeviceTracker* device_tracker,
                            CommandState* command_state);

// Checks if the PIN is not currently set by trying to make a credential.
// The MakeCredential command should fail when the authenticator is PIN
// protected. Even though this test could fail in case of a bad implementation
// of Make Credential, this kind of misbehavior would be caught in another
// test.
void CheckPinAbsenceByMakeCredential(DeviceInterface* device,
                                     DeviceTracker* device_tracker);

}  // namespace test_helpers
}  // namespace fido2_tests

#endif  // TESTS_TEST_HELPERS_H_

