/**
 * Stub implementation of the @raft_io interface, meant for unit tests.
 */
#ifndef RAFT_IO_STUB_H
#define RAFT_IO_STUB_H

#include <stdbool.h>

struct raft_io;
struct raft_logger;
struct raft_message;
struct raft_entry;

/**
 * Configure the given @raft_io instance to use a stub in-memory I/O
 * implementation.
 */
int raft_io_stub_init(struct raft_io *io, struct raft_logger *logger);

void raft_io_stub_close(struct raft_io *io);

/**
 * Advance the stub time by the given number of milliseconds, and invoke the
 * tick callback accordingly.
 */
void raft_io_stub_advance(struct raft_io *io, unsigned msecs);

/**
 * Dispatch a message, invoking the recv callback.
 */
void raft_io_stub_dispatch(struct raft_io *io, struct raft_message *message);

/**
 * Flush all pending I/O requests, invoking the @notify callback as appropriate.
 */
void raft_io_stub_flush(struct raft_io *io);

/**
 * Return a copy of the pending messages that where flushed upon the last call
 * to
 * @raft_io_stub_flush.
 */
void raft_io_stub_sent(struct raft_io *io,
                       struct raft_message **messages,
                       unsigned *n);

/**
 * Return a copy of the pending log entries that where flushed upon the last
 * call to @raft_io_stub_flush.
 */
void raft_io_stub_appended(struct raft_io *io,
                           struct raft_entry **entries,
                           unsigned *n);

/**
 * Inject a failure that will be triggered after @delay I/O requests and occur
 * @repeat times.
 */
void raft_io_stub_fault(struct raft_io *io, int delay, int repeat);

/**
 * Convenience for getting the current term stored in the stub.
 */
unsigned raft_io_stub_term(struct raft_io *io);

/**
 * Convenience for getting the current vote stored in the stub.
 */
unsigned raft_io_stub_vote(struct raft_io *io);

#endif /* RAFT_IO_STUB_H */
