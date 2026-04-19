// Сборка:
//   gcc -O2 -pthread distributed_editor.c -o distributed_editor
//
// Запуск:
//   ./distributed_editor 0
//   ./distributed_editor 1
//   ./distributed_editor 2

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_NODES 3
#define MAX_TEXT  1024
#define MAX_DOC   4096

typedef struct {
    uint32_t sender_id;
    uint32_t lamport_ts;
    char payload[MAX_TEXT];
} Message;

typedef struct {
    const char *ip;
    uint16_t port;
} Peer;

static const Peer peers[MAX_NODES] = {
    { "127.0.0.1", 5000 },
    { "127.0.0.1", 5001 },
    { "127.0.0.1", 5002 }
};

static int my_id = -1;
static int sockfd = -1;
static pthread_mutex_t state_mu = PTHREAD_MUTEX_INITIALIZER;

static uint32_t lamport_clock = 0;
static char document[MAX_DOC] = "";

// timestamp последней применённой операции append
static bool has_last_applied = false;
static uint32_t last_applied_ts = 0;
static uint32_t last_applied_sender = 0;

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void chomp(char *s) {
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') {
        s[n - 1] = '\0';
    }
}

static void print_state_locked(const char *prefix) {
    printf("%s [node=%d clock=%u] document=\"%s\"",
           prefix, my_id, lamport_clock, document);

    if (has_last_applied) {
        printf(" last=(ts=%u sender=%u)", last_applied_ts, last_applied_sender);
    } else {
        printf(" last=(none)");
    }
    printf("\n");
    fflush(stdout);
}

static void apply_append_locked(const Message *msg, bool local_apply) {
    const char *src = local_apply ? "LOCAL" : "REMOTE";

    char before[MAX_DOC];
    snprintf(before, sizeof(before), "%s", document);

    strncat(document, msg->payload, sizeof(document) - strlen(document) - 1);

    has_last_applied = true;
    last_applied_ts = msg->lamport_ts;
    last_applied_sender = msg->sender_id;

    printf("[%s APPLY] node=%d append \"%s\" (ts=%u sender=%u) before=\"%s\" after=\"%s\"\n",
           src, my_id, msg->payload, msg->lamport_ts, msg->sender_id, before, document);
    fflush(stdout);
}

static void process_incoming_locked(const Message *msg) {
    if (!has_last_applied) {
        apply_append_locked(msg, false);
        return;
    }

    if (msg->lamport_ts < last_applied_ts) {
        printf("[IGNORE] node=%d incoming append \"%s\" (ts=%u sender=%u) ignored as older than "
               "last applied (ts=%u sender=%u)\n",
               my_id,
               msg->payload, msg->lamport_ts, msg->sender_id,
               last_applied_ts, last_applied_sender);
        fflush(stdout);
        return;
    }

    if (msg->lamport_ts == last_applied_ts) {
        printf("[CONFLICT] node=%d incoming append \"%s\" (ts=%u sender=%u) conflicts with "
               "last applied (ts=%u sender=%u)\n",
               my_id,
               msg->payload, msg->lamport_ts, msg->sender_id,
               last_applied_ts, last_applied_sender);
        fflush(stdout);
        return;
        exit(1); // экстренное завершение программы
    }

    // msg->lamport_ts > last_applied_ts
    apply_append_locked(msg, false);
}

static void send_to_peer(int peer_id, const Message *msg) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(peers[peer_id].port);

    if (inet_pton(AF_INET, peers[peer_id].ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "inet_pton failed for %s\n", peers[peer_id].ip);
        return;
    }

    ssize_t sent = sendto(sockfd, msg, sizeof(*msg), 0,
                          (struct sockaddr *)&addr, sizeof(addr));
    if (sent != sizeof(*msg)) {
        fprintf(stderr, "[node=%d] sendto to node %d failed: %s\n",
                my_id, peer_id, strerror(errno));
        return;
    }

    printf("[SEND] node=%d -> node=%d append \"%s\" ts=%u\n",
           my_id, peer_id, msg->payload, msg->lamport_ts);
    fflush(stdout);
}

static void broadcast_message(const Message *msg) {
    for (int i = 0; i < MAX_NODES; ++i) {
        if (i == my_id) {
            continue;
        }
        send_to_peer(i, msg);
    }
}

static void local_append_and_broadcast(const char *payload) {
    Message msg;
    memset(&msg, 0, sizeof(msg));

    pthread_mutex_lock(&state_mu);

    // пользовательский ввод — отдельное событие
    lamport_clock++;

    // перед отправкой увеличиваем clock ещё раз, потому что отправка - тоже событие
    lamport_clock++;

    msg.sender_id = (uint32_t)my_id;
    msg.lamport_ts = lamport_clock;
    snprintf(msg.payload, sizeof(msg.payload), "%s", payload);

    printf("[LOCAL EVENT] node=%d append \"%s\" ts=%u\n",
           my_id, payload, msg.lamport_ts);

    apply_append_locked(&msg, true);
    print_state_locked("[STATE AFTER LOCAL]");

    pthread_mutex_unlock(&state_mu);

    broadcast_message(&msg);
}

static void *receiver_thread(void *arg) {
    (void)arg;

    while (1) {
        Message msg;
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);

        ssize_t n = recvfrom(sockfd, &msg, sizeof(msg), 0,
                             (struct sockaddr *)&from_addr, &from_len);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "[node=%d] recvfrom failed: %s\n", my_id, strerror(errno));
            continue;
        }

        if ((size_t)n != sizeof(msg)) {
            fprintf(stderr, "[node=%d] short message: %zd bytes\n", my_id, n);
            continue;
        }

        pthread_mutex_lock(&state_mu);

        uint32_t old_clock = lamport_clock;
        lamport_clock = (lamport_clock > msg.lamport_ts ? lamport_clock : msg.lamport_ts) + 1;

        printf("[RECV] node=%d <- node=%u append \"%s\" remote_ts=%u local_clock: %u -> %u\n",
               my_id, msg.sender_id, msg.payload, msg.lamport_ts, old_clock, lamport_clock);

        process_incoming_locked(&msg);
        print_state_locked("[STATE AFTER RECV]");

        pthread_mutex_unlock(&state_mu);
    }

    return NULL;
}

static void repl_loop(void) {
    char line[2048];

    printf("Commands:\n");
    printf("  append <text>\n");
    printf("  state\n");
    printf("  help\n\n");

    while (1) {
        printf("node %d> ", my_id);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\nEOF, exiting.\n");
            break;
        }

        chomp(line);

        if (strncmp(line, "append ", 7) == 0) {
            const char *arg = line + 7;
            if (*arg == '\0') {
                printf("empty payload\n");
                continue;
            }
            local_append_and_broadcast(arg);
        } else if (strcmp(line, "state") == 0) {
            pthread_mutex_lock(&state_mu);
            print_state_locked("[STATE]");
            pthread_mutex_unlock(&state_mu);
        } else if (strcmp(line, "help") == 0) {
            printf("Commands:\n");
            printf("  append <text>\n");
            printf("  state\n");
            printf("  help\n");
        } else if (*line == '\0') {
            continue;
        } else {
            printf("unknown command: %s\n", line);
        }
    }
}

static void bind_socket_for_me(void) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        die("socket");
    }

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        die("setsockopt(SO_REUSEADDR)");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(peers[my_id].port);

    if (inet_pton(AF_INET, peers[my_id].ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "inet_pton failed for %s\n", peers[my_id].ip);
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        die("bind");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <node_id>\n", argv[0]);
        fprintf(stderr, "  node_id must be in [0..%d]\n", MAX_NODES - 1);
        return 1;
    }

    my_id = atoi(argv[1]);
    if (my_id < 0 || my_id >= MAX_NODES) {
        fprintf(stderr, "invalid node_id: %d\n", my_id);
        return 1;
    }

    printf("Starting worker %d at %s:%u\n",
           my_id, peers[my_id].ip, peers[my_id].port);

    bind_socket_for_me();

    pthread_t th;
    if (pthread_create(&th, NULL, receiver_thread, NULL) != 0) {
        die("pthread_create");
    }

    repl_loop();

    close(sockfd);
    return 0;
}