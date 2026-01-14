/*
    This file is part of the Arduino_RPClite library.

    Copyright (c) 2025 Arduino SA

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    
*/

#include <Arduino_RPClite.h>
#include "DummyTransport.h"
#include "decoder_tester.h"

// Shorthand
MsgPack::Packer packer;

void print_buf() {
    Serial.print("buf size: ");
    Serial.print(packer.size());
    Serial.print(" - ");

    for (size_t i=0; i<packer.size(); i++){
        Serial.print(packer.data()[i], HEX);
        Serial.print(" ");
    }
    Serial.println(" ");
}

void runDecoderTest(const char* label) {
  Serial.println(label);

  print_buf();
  DummyTransport dummy_transport(packer.data(), packer.size());
  RpcDecoder<> decoder(dummy_transport);

  while (!decoder.packet_incoming()) {
    Serial.println("Packet not ready");
    decoder.decode();
    delay(50);
  }

  size_t pack_size = decoder.get_packet_size();
  Serial.print("1st Packet size: ");
  Serial.println(pack_size);

  Serial.println("-- Done --\n");
}

void runDecoderConsumeTest(const char* label, size_t second_packet_sz) {
  Serial.println(label);

  print_buf();
  DummyTransport dummy_transport(packer.data(), packer.size());
  RpcDecoder<> decoder(dummy_transport);

  DecoderTester dt(decoder);

  while (!decoder.packet_incoming()) {
    Serial.println("Packet not ready");
    decoder.decode();
    delay(50);
  }

  size_t pack_size = decoder.get_packet_size();
  Serial.print("1st Packet size: ");
  Serial.println(pack_size);

  Serial.print("Consuming 2nd packet of given size: ");
  Serial.println(second_packet_sz);

  dt.crop_bytes(second_packet_sz, pack_size);

  dt.print_raw_buf();

  Serial.println("-- Done --\n");
}

void testNestedArrayRequest() {
  packer.clear();
  MsgPack::arr_size_t outer_arr(3);
  MsgPack::arr_size_t inner_arr(2);
  MsgPack::arr_size_t req_sz(4);

  packer.serialize(req_sz, 0, 1, "nested_array", outer_arr, 1, inner_arr, 2, 3, 4);
  runDecoderTest("== Test: Nested Array in Request ==");
}

void testMapRequest() {
  packer.clear();
  MsgPack::map_size_t main_map(3);
  MsgPack::arr_size_t arr_b(2);
  MsgPack::map_size_t inner_map(1);
  MsgPack::arr_size_t req_sz(4);

  packer.serialize(req_sz, 0, 1, "map_param", main_map,
                   "a", 1,
                   "b", arr_b, 10, 20,
                   "c", inner_map, "d", 4);
  runDecoderTest("== Test: Map in Request ==");
}

void testNestedReturn() {
  packer.clear();
  MsgPack::arr_size_t ret_arr(2);
  MsgPack::map_size_t map_ret(2);
  MsgPack::arr_size_t arr_x(2);
  MsgPack::arr_size_t resp_sz(4);
  MsgPack::object::nil_t nil;

  packer.serialize(resp_sz, 1, 1, nil,
                   ret_arr, 1,
                   map_ret, "x", arr_x, 100, 200, "y", true);
  runDecoderTest("== Test: Nested Return in Response ==");
}

void testDeepNestedStructure() {
  packer.clear();
  MsgPack::arr_size_t top_arr(1);
  MsgPack::map_size_t map_a(1);
  MsgPack::arr_size_t arr_a(1);
  MsgPack::map_size_t map_b(1);
  MsgPack::arr_size_t arr_b(1);
  MsgPack::arr_size_t req_sz(4);

  packer.serialize(req_sz, 0, 1, "deep_nested", top_arr,
                   map_a, "a", arr_a,
                   map_b, "b", arr_b, 42);
  runDecoderTest("== Test: Deep Nested Request ==");
}

void testArrayOfMapsResponse() {
  packer.clear();
  MsgPack::map_size_t map1(1), map2(1), map3(1);
  MsgPack::arr_size_t arr_maps(3);
  MsgPack::arr_size_t resp_sz(4);
  MsgPack::object::nil_t nil;

  packer.serialize(resp_sz, 1, 1, nil,
                   arr_maps,
                     map1, "k", 1,
                     map2, "k", 2,
                     map3, "k", 3);
  runDecoderTest("== Test: Array of Maps in Response ==");
}

// Multiple RPCs in one buffer
void testMultipleRpcPackets() {
  packer.clear();
  MsgPack::arr_size_t req_sz(4);
  MsgPack::arr_size_t par_sz(2);
  MsgPack::arr_size_t resp_sz(4);
  MsgPack::object::nil_t nil;

  // 1st request
  packer.serialize(req_sz, 0, 1, "sum", par_sz, 10, 20);
  // 2nd response
  packer.serialize(resp_sz, 1, 1, nil, 42);
  // 3rd request
  packer.serialize(req_sz, 0, 2, "echo", par_sz, "Hello", true);

  runDecoderTest("== Test: Multiple RPCs in Buffer ==");

  runDecoderConsumeTest("== Test: Mid-buffer consume ==", 5);

}

// Binary parameter (e.g., binary blob)
void testBinaryParam() {
  packer.clear();
  MsgPack::arr_size_t req_sz(4);

  const MsgPack::bin_t<uint8_t> bin_data {0x01, 0x23, 0x45, 0x67, 0x89};
  packer.serialize(req_sz, 0, 3, "binary", bin_data);

  runDecoderTest("== Test: Binary Parameter ==");
}

// Extension type param (e.g., tagged binary format)
void testExtensionParam() {
  packer.clear();
  MsgPack::arr_size_t req_sz(4);

  const int8_t ext_type = 42;
  const uint8_t ext_payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
  packer.serialize(req_sz, 0, 4, "extension", MsgPack::object::ext(ext_type, ext_payload, sizeof(ext_payload)));

  runDecoderTest("== Test: Extension Parameter ==");
}

// Mix binary + ext + multiple RPCs
void testCombinedComplexBuffer() {
  packer.clear();
  MsgPack::arr_size_t req_sz(4);
  MsgPack::arr_size_t par_sz(3);
  MsgPack::object::nil_t nil;
  MsgPack::arr_size_t resp_sz(4);

  const MsgPack::bin_t<uint8_t> bin_blob {0xAA, 0xBB, 0xCC};
  const uint8_t ext_blob[] = {0xFE, 0xED};

  // Request with bin + ext + normal
  packer.serialize(req_sz, 0, 5, "combo", par_sz,
                   bin_blob,
                   MsgPack::object::ext(7, ext_blob, sizeof(ext_blob)),
                   999);

  // Response after that
  packer.serialize(resp_sz, 1, 5, nil, true);

  runDecoderTest("== Test: Mixed Bin/Ext + Multiple RPCs ==");
}

void setup() {
  Serial.begin(115200);
  while(!Serial);
  
  delay(1000);
  Serial.println("=== RPC Decoder Nested Tests ===");

  testNestedArrayRequest();
  testMapRequest();
  testNestedReturn();
  testDeepNestedStructure();
  testArrayOfMapsResponse();
  testMultipleRpcPackets();
  testBinaryParam();
  testExtensionParam();
  testCombinedComplexBuffer();

  Serial.println("=== All Tests Completed ===");
}

void loop() {
  // No repeated logic needed
}
