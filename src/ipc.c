#include "i3/ipc.h"


ssize_t writeall(int fd, const void* buf, size_t count) {
	size_t written = 0;

	while (written < count) {
		const ssize_t n = write(fd, ((char*) buf) + written, count - written);
		if (n == -1) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return n;
		}
		written += (size_t) n;
	}

	return written;
}

int i3_ipc_send_message(int sockfd, const uint32_t msg_size, const uint32_t message_type, const uint8_t* payload) {
	const i3_ipc_header_t header = {
			.magic = {'i', '3', '-', 'i', 'p', 'c'},
			.size = msg_size,
			.type = message_type};

	if (writeall(sockfd, ((void*) &header), sizeof(i3_ipc_header_t)) == -1)
		return -1;

	if (writeall(sockfd, payload, msg_size) == -1)
		return -1;

	return 0;
}

int i3_ipc_connect(const char* socket_path) {
	char* path = NULL;

	if (socket_path != NULL) path = strdup(socket_path);

	if (path == NULL) {
		if ((path = getenv("I3SOCK")) != NULL) {
			path = strdup(path);
		}
	}

	if (path == NULL) path = strdup("/tmp/i3-ipc.sock");

	int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd == -1)
		return -1;

	(void) fcntl(sockfd, F_SETFD, FD_CLOEXEC);

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(struct sockaddr_un));

	addr.sun_family = AF_LOCAL;

	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

	if (connect(sockfd, (const struct sockaddr*) &addr, sizeof(struct sockaddr_un)) < 0)
		return -1;

	free(path);
	return sockfd;
}

int i3_ipc_recv_message(int sockfd, uint32_t* message_type,
						uint32_t* reply_length, uint8_t** reply) {
	/* Read the message header first */
	const uint32_t to_read = strlen(I3_IPC_MAGIC) + sizeof(uint32_t) + sizeof(uint32_t);
	char msg[to_read];
	char* walk = msg;

	uint32_t read_bytes = 0;
	while (read_bytes < to_read) {
		int n = read(sockfd, msg + read_bytes, to_read - read_bytes);
		if (n == -1) return -1;
		if (n == 0) {
			if (read_bytes == 0) {
				return -2;
			} else {
				return -3;
			}
		}

		read_bytes += n;
	}

	if (memcmp(walk, I3_IPC_MAGIC, strlen(I3_IPC_MAGIC)) != 0) {
		return -3;
	}

	walk += strlen(I3_IPC_MAGIC);
	memcpy(reply_length, walk, sizeof(uint32_t));
	walk += sizeof(uint32_t);
	if (message_type != NULL)
		memcpy(message_type, walk, sizeof(uint32_t));

	*reply = malloc(*reply_length);

	read_bytes = 0;
	while (read_bytes < *reply_length) {
		const int n = read(sockfd, *reply + read_bytes, *reply_length - read_bytes);
		if (n == -1) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			return -1;
		}
		if (n == 0) {
			return -3;
		}

		read_bytes += n;
	}

	return 0;
}
