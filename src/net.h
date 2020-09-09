#ifndef SNAKE__NET_H_
#define SNAKE__NET_H_

#include <SDL2/SDL_net.h>

#define LAN_LISTEN_PORT 21739
#define LAN_SEED_MASK 0x00ffffff

// Disable padding
#pragma pack(push,1)

// Play on lan general packet should be 4 byte ( 32 bit )

typedef enum {
  HEADER_HANDSHAKE,
  HEADER_PLAYERMOVE,
  HEADER_GAMEOVER
} HeaderType;

// Packet header, 8 bit
typedef struct {
  unsigned version: 2;
  unsigned type: 6;
} LanPacketHeader;

typedef struct {
  LanPacketHeader header;
  unsigned payload: 24;
} LanPacket;

typedef struct {
  LanPacketHeader header; // type 0
  unsigned seed: 24; // random seed to be used in the turn
} HandShakePacket;

typedef struct {
  LanPacketHeader header; // type 1
  unsigned playerId: 3;
  unsigned direction: 2;
  unsigned padding: 19;
} PlayerMovePacket;

typedef struct {
  LanPacketHeader header; // type 2
  unsigned playerId: 3;
  unsigned padding: 21;
} GameOverPacket;

#pragma pack(pop)

extern TCPsocket lanServerSocket;
extern TCPsocket lanClientSocket;

void hostGame();
void joinGame(const char* hostname, Uint16 port);
void sendPlayerMovePacket(unsigned playerId, unsigned direction);
void sendGameOverPacket(unsigned playerId);
unsigned recvLanPacket(LanPacket* dest);
#endif
