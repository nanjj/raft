#include <assert.h>
#include <stdio.h>

#include <uv.h>

#include "../include/raft.h"

static void __stop_cb(void *data)
{
    struct uv_loop_s *loop = data;

    uv_stop(loop);
}

/**
 * Handler triggered by SIGINT. It will stop the raft engine.
 */
static void __sigint_cb(uv_signal_t *handle, int signum)
{
    struct raft *raft = handle->data;
    int rv;

    assert(signum == SIGINT);

    uv_signal_stop(handle);
    uv_close((uv_handle_t *)handle, NULL);

    rv = raft_stop(raft, handle->loop, __stop_cb);
    if (rv != 0) {
        printf("error: stop instance: %s\n", raft_strerror(rv));
    }
}

int main(int argc, char *argv[])
{
    struct uv_loop_s loop;
    struct uv_signal_s sigint;
    struct raft_logger logger;
    struct raft_io_uv_transport transport;
    struct raft_io io;
    struct raft raft;
    const char *dir;
    int rv;

    if (argc != 2) {
        printf("usage: example-server <dir>\n");
        return 1;
    }

    dir = argv[1];

    /* Initialize the libuv loop. */
    rv = uv_loop_init(&loop);
    if (rv != 0) {
        printf("error: loop init: %s\n", uv_strerror(rv));
        return rv;
    }
    loop.data = &raft;

    /* Add a signal handler to stop the Raft engine upon SIGINT */
    rv = uv_signal_init(&loop, &sigint);
    if (rv != 0) {
        printf("error: sigint init: %s\n", uv_strerror(rv));
        return rv;
    }

    sigint.data = &raft;

    rv = uv_signal_start(&sigint, __sigint_cb, SIGINT);
    if (rv != 0) {
        printf("error: sigint start: %s\n", uv_strerror(rv));
        return rv;
    }

    /* Initialize the TCP-based RPC transport */
    rv = raft_io_uv_tcp_init(&transport, &logger, &loop);
    if (rv != 0) {
        printf("error: init TCP transport: %s\n", raft_strerror(rv));
        return rv;
    }

    /* Initialize the libuv-based I/O backend */
    rv = raft_io_uv_init(&io, &logger, &loop, dir, &transport);
    if (rv != 0) {
        printf("error: enable uv integration: %s\n", raft_strerror(rv));
        return rv;
    }

    /* Initialize and start the Raft engine, using libuv-based I/O backend. */
    raft_init(&raft, &raft_default_logger, &io,
              NULL /* TODO: fsm implementation */, NULL, 1, "127.0.0.1:9000");

    rv = raft_start(&raft);
    if (rv != 0) {
        printf("error: start engine: %s\n", raft_strerror(rv));
        return rv;
    }

    /* Run the event loop until we receive SIGINT. */
    rv = uv_run(&loop, UV_RUN_DEFAULT);
    if (rv != 0) {
        printf("error: loop run: %s\n", uv_strerror(rv));
        return rv;
    }

    /* Clean up */
    raft_close(&raft);
    raft_io_uv_close(&io);
    raft_io_uv_tcp_close(&transport);

    rv = uv_loop_close(&loop);
    if (rv != 0) {
        printf("error: loop close: %s\n", uv_strerror(rv));
        return rv;
    }

    return 0;
}
