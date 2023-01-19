#include <unity.h>

#include <stdint.h>
#include <stdio.h>
#include <packetize.h>


static_assert(sizeof(float) == sizeof(uint32_t), "float must be 32 bits wide");
struct test_msg {
  struct {
    uint8_t one;
    uint8_t two;
    uint8_t three;
  } data;
  uint8_t crc8;
};

void test_packetize() {
  // canaries for detecting buffer overruns
  uint8_t msg_buf[sizeof(test_msg)+3];
  msg_buf[sizeof(test_msg)] = 0xfa;
  msg_buf[sizeof(test_msg) + 1] = 0xfb;
  msg_buf[sizeof(test_msg) + 2] = 0xfc;

  test_msg* msg = (test_msg*) msg_buf;
  msg->data.one = 1;
  msg->data.two = 0;
  msg->data.three = 2;

  // canaries
  uint8_t packet_buf[sizeof(test_msg) + 2 + 3];
  packet_buf[sizeof(test_msg) + 2] = 0xfa;
  packet_buf[sizeof(test_msg) + 2 + 1] = 0xfb;
  packet_buf[sizeof(test_msg) + 2 + 2] = 0xfc;

  packetize(msg, sizeof(test_msg), packet_buf);

  uint8_t expected_packet[] = { 0x00, 0x02, 0x01, 0x03, 0x02, 0x35 };
  TEST_ASSERT_EQUAL_MEMORY(expected_packet, packet_buf, sizeof(expected_packet));

  printf("Packet:\n");
  for (size_t i = 0; i < sizeof(packet_buf) - 3; i++) {
    printf("%02x ", (unsigned char) packet_buf[i]);
  }
  printf("\n");

  TEST_ASSERT_EQUAL_INT(0xfa, msg_buf[sizeof(test_msg)]);
  TEST_ASSERT_EQUAL_INT(0xfb, msg_buf[sizeof(test_msg) + 1]);
  TEST_ASSERT_EQUAL_INT(0xfc, msg_buf[sizeof(test_msg) + 2]);
  TEST_ASSERT_EQUAL_INT(0xfa, packet_buf[sizeof(test_msg) + 2]);
  TEST_ASSERT_EQUAL_INT(0xfb, packet_buf[sizeof(test_msg) + 2 + 1]);
  TEST_ASSERT_EQUAL_INT(0xfc, packet_buf[sizeof(test_msg) + 2 + 2]);

  printf("Canaries:\n");
  printf("%02x\n", msg_buf[sizeof(test_msg)]);
  printf("%02x\n", msg_buf[sizeof(test_msg) + 1]);
  printf("%02x\n", msg_buf[sizeof(test_msg) + 2]);

  printf("%02x\n", packet_buf[sizeof(test_msg) + 2]);
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2 + 1]);
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2 + 2]);
}

void test_depacketize() {
  // including canaries
  uint8_t packet_buf[] = { 0x00, 0x02, 0x01, 0x03, 0x02, 0x35, 0xfa, 0xfb, 0xfc };

  // canaries
  uint8_t new_msg_buf[sizeof(test_msg)+3];
  new_msg_buf[sizeof(test_msg)] = 0xfa;
  new_msg_buf[sizeof(test_msg) + 1] = 0xfb;
  new_msg_buf[sizeof(test_msg) + 2] = 0xfc;
  test_msg* new_msg = (test_msg*) new_msg_buf;
  bool error = depacketize(packet_buf, sizeof(test_msg), new_msg);

  printf("\nNew Msg Struct:\n");
  for (size_t i = 0; i < sizeof(test_msg); i++) {
    printf("%02x ", ((unsigned char*) new_msg)[i]);
  }
  printf("\n");

  TEST_ASSERT_EQUAL_INT(0, error);

  printf("\nChecksum Error: %d\n", error);

  TEST_ASSERT_EQUAL_INT(1, new_msg->data.one);
  TEST_ASSERT_EQUAL_INT(0, new_msg->data.two);
  TEST_ASSERT_EQUAL_INT(2, new_msg->data.three);

  printf("\nMessage:\n");
  printf("one=%d, two=%d, three=%d\n", new_msg->data.one, new_msg->data.two, new_msg->data.three);

  TEST_ASSERT_EQUAL_INT(0xfa, packet_buf[sizeof(test_msg) + 2]);
  TEST_ASSERT_EQUAL_INT(0xfb, packet_buf[sizeof(test_msg) + 2 + 1]);
  TEST_ASSERT_EQUAL_INT(0xfc, packet_buf[sizeof(test_msg) + 2 + 2]);
  TEST_ASSERT_EQUAL_INT(0xfa, new_msg_buf[sizeof(test_msg)]);
  TEST_ASSERT_EQUAL_INT(0xfb, new_msg_buf[sizeof(test_msg) + 1]);
  TEST_ASSERT_EQUAL_INT(0xfc, new_msg_buf[sizeof(test_msg) + 2]);

  printf("\nCanaries:\n");
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2]);
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2 + 1]);
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2 + 2]);

  printf("%02x\n", new_msg_buf[sizeof(test_msg)]);
  printf("%02x\n", new_msg_buf[sizeof(test_msg) + 1]);
  printf("%02x\n", new_msg_buf[sizeof(test_msg) + 2]);
}

void test_depacketize_bad_checksum() {
  // including canaries
  uint8_t packet_buf[] = { 0x00, 0x02, 0x01, 0x03, 0x02, 0x38, 0xfa, 0xfb, 0xfc };

  // canaries
  uint8_t new_msg_buf[sizeof(test_msg)+3];
  new_msg_buf[sizeof(test_msg)] = 0xfa;
  new_msg_buf[sizeof(test_msg) + 1] = 0xfb;
  new_msg_buf[sizeof(test_msg) + 2] = 0xfc;
  test_msg* new_msg = (test_msg*) new_msg_buf;
  bool error = depacketize(packet_buf, sizeof(test_msg), new_msg);

  printf("\nNew Msg Struct:\n");
  for (size_t i = 0; i < sizeof(test_msg); i++) {
    printf("%02x ", ((unsigned char*) new_msg)[i]);
  }
  printf("\n");

  TEST_ASSERT_EQUAL_INT(1, error);

  printf("\nChecksum Error: %d\n", error);

  TEST_ASSERT_EQUAL_INT(1, new_msg->data.one);
  TEST_ASSERT_EQUAL_INT(0, new_msg->data.two);
  TEST_ASSERT_EQUAL_INT(2, new_msg->data.three);

  printf("\nMessage:\n");
  printf("one=%d, two=%d, three=%d\n", new_msg->data.one, new_msg->data.two, new_msg->data.three);

  TEST_ASSERT_EQUAL_INT(0xfa, packet_buf[sizeof(test_msg) + 2]);
  TEST_ASSERT_EQUAL_INT(0xfb, packet_buf[sizeof(test_msg) + 2 + 1]);
  TEST_ASSERT_EQUAL_INT(0xfc, packet_buf[sizeof(test_msg) + 2 + 2]);
  TEST_ASSERT_EQUAL_INT(0xfa, new_msg_buf[sizeof(test_msg)]);
  TEST_ASSERT_EQUAL_INT(0xfb, new_msg_buf[sizeof(test_msg) + 1]);
  TEST_ASSERT_EQUAL_INT(0xfc, new_msg_buf[sizeof(test_msg) + 2]);

  printf("\nCanaries:\n");
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2]);
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2 + 1]);
  printf("%02x\n", packet_buf[sizeof(test_msg) + 2 + 2]);

  printf("%02x\n", new_msg_buf[sizeof(test_msg)]);
  printf("%02x\n", new_msg_buf[sizeof(test_msg) + 1]);
  printf("%02x\n", new_msg_buf[sizeof(test_msg) + 2]);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_packetize);
    RUN_TEST(test_depacketize);
    RUN_TEST(test_depacketize_bad_checksum);
    return UNITY_END();
}
