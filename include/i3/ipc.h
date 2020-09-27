#ifndef __7ASKE_XINP_I3_IPC_H
#define __7ASKE_XINP_I3_IPC_H

#pragma once

#include <stdint.h>
#include <zconf.h>
#include <errno.h>
#include <memory.h>
#include <sys/un.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>

#define I3_IPC_MAGIC "i3-ipc"
#define I3_MAGIC {'i', '3', '-', 'i', 'p', 'c'}

typedef enum _i3_ipc_message_type {
	MESSAGE_RUN_COMMAND = 0,
	MESSAGE_GET_WORKSPACES = 1,
	MESSAGE_SUBSCRIBE = 2,
	MESSAGE_GET_OUTPUTS = 3,
	MESSAGE_GET_TREE = 4,
	MESSAGE_GET_MARKS = 5,
	MESSAGE_GET_BAR_CONFIG = 6,
	MESSAGE_GET_VERSION = 7,
	MESSAGE_GET_BINDING_MODES = 8,
	MESSAGE_GET_CONFIG = 9,
	MESSAGE_SEND_TICK = 10,
	MESSAGE_SYNC = 11,
	MESSAGE_GET_BINDING_STATE = 12,
} i3_ipc_message_type;

typedef enum _i3_ipc_reply_type {
	REPLY_COMMAND = 0,
	REPLY_WORKSPACES = 1,
	REPLY_SUBSCRIBE = 2,
	REPLY_OUTPUTS = 3,
	REPLY_TREE = 4,
	REPLY_MARKS = 5,
	REPLY_BAR_CONFIG = 6,
	REPLY_VERSION = 7,
	REPLY_BINDING_MODES = 8,
	REPLY_CONFIG = 9,
	REPLY_TICK = 10,
	REPLY_SYNC = 11,
	REPLY_GET_BINDING_STATE = 12,
} i3_ipc_reply_type;

typedef struct _i3_ipc_header_t {
	char magic[6];
	uint32_t size;
	uint32_t type;
} __attribute__((packed)) i3_ipc_header_t;

int i3_ipc_connect(const char* socket_path);

int i3_ipc_send_message(int sockfd, const uint32_t message_size, const uint32_t message_type, const uint8_t* payload);
int i3_ipc_recv_message(int sockfd, uint32_t *message_type, uint32_t *reply_length, uint8_t **reply);

#endif