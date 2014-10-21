
NOTE: below is the original unmodified 2002 README, it does not fully
reflect the current status... ;-) After reading this file, also see
the COMPILING.txt and USAGE.txt files for newer instructions.

========================================================================

NOTE:

This file is *not* intended to be comprehensive documentation for the
Tsunami protocol or the programs in the Tsunami suite.  We are working
on formal documentation, but it is not yet ready for public release.
Please bear with us -- Tsunami is a young and rapidly evolving
protocol, and we're documenting a moving target.

Tsunami is built using the standard GNU autoconf/automake system.  To
install, use the standard './configure', 'make', 'make install'
sequence.  (Thanks are due to Jeff Squyres <jsquyres@osl.iu.edu>
of Indiana University's Open Systems Lab for bringing us into the
modern age of automated building and configuration.)

Building Tsunami will create the Tsunami client (tsunami), the Tsunami
server (tsunamid), and two utilities for benchmarking disk subsystem
performance (readtest and writetest).

Later in this file, you'll find details on how Tsunami currently
performs authentication.

Please share with us any Tsunami performance data you can offer!
Ideally, we'd like to have hardware profiles of the client and server
systems (CPU, disk controller, memory size, kernel version, bdflush
settings, and so forth), the output of tsunami and tsunamid during
file transmission, the output of vmstat on both the client and server,
and the protocol parameters used.  This data will help us to tune
the protocol and make the next release more robust.

And finally, please read the license agreement found in LICENSE.TXT.

If you have any technical questions about the Tsunami protocol, please
subscribe to the Tsunami LISTSERV.  Instructions can be found on
the mailing list home page at:

  http://listserv.indiana.edu/archives/tsunami-l.html

========================================================================

The Tsunami protocol
--------------------

A basic Tsunami conversation works like this:

(1) The client connects to the Tsunamid TCP port (46224 by default).
    The server forks off a child process to deal with the connection.

(2) The client and server exchange protocol revision numbers to make
    sure that they're talking the same language.  (The revision number
    is defined in "tsunami.h".)

(3) The client authenticates to the server.  This process is described
    later in this file.

(4) The server is now waiting for the name of a file to transfer to
    the client.

(5) Once the file name is received, the server makes sure that it
    can open and read the file.  If it can, a positive result byte
    is sent to the client.  If it can't, the server reports failure.

(6) The client and server exchange protocol parameter information.

(7) The client sends the server the number of the UDP port on which
    the client will listen for the file data.

(8) The server and client both enter their file transmission loops.

========================================================================

The server file transmission loop
---------------------------------

while the whole file hasn't been sent yet:
    see if the client has sent a request over the TCP pipe (*)
    if it has:
        service that request
    otherwise:
	send the next block in the file
    delay for the next packet

(*) There are three kinds of request:
      (1) error rate notification
      (2) retransfer block [nn]
      (3) restart transfer at block [nn]

========================================================================

The client file transmission loop
---------------------------------

while the whole file hasn't been received yet:
    try to receive another block
    if it's the last block:
        break out of the loop and notify the server
    otherwise:
        on every 50th iteration, see if it's been [update_period] since
          our last statistics update
        if it has:
            display updated statistics
            notify the server of our current error rate
            transmit our queue of retransmission requests
        save the block
        if the block is later than the one we were expecting:
	    put intervening blocks in the retransmission queue
        if the block is earlier than the one we were expecting:
            remove the block from the retransmission queue

========================================================================

The retransmission queue
------------------------

This is a (potentially) sparse array of block numbers that we may need
to have retransmitted.  Each entry is either 0 or a block number.  The
size of the array is doubled if it runs out of space.  We keep track
of the lowest index used and the highest index used and rehome the
data to the base of the array occasionally.

If the queue is extremely large (over [threshold] entries), instead of
asking for each entry in the queue, we ask to restart the transfer at
the first block in the queue.

========================================================================

How Tsunami does authentication
-------------------------------

The Tsunami server and Tsunami client both know a shared secret.
(Right now it's coded into the Tsunami server as "kitten", but this
can be overridden with the '--secret' option.)  The client learns the
shared secret by giving the user a 'password' prompt and reading it in
with echo turned off.

The following sequence allows the client to prove its knowledge of the
shared secret to the server:

(1) The server reads 512 bits of random data from /dev/random and
    sends this data to the client.

(2) The client XORs copies of the shared secret over the random data.

(3) The client sends an MD5 hash of the resulting buffer back to the
    server.

(4) The server performs the same XOR/MD5 operation on the random data
    and checks to make sure that they match.  If they do, a positive
    result byte is sent to the client.  If they don't, the connection
    is closed.

========================================================================

Other notes
-----------

(1) Everything is endian-independent except for the MD5 code.

(2) Everything does work okay with 64-bit file sizes, using the
    fopen64() / fseeko64() API.

(3) Porting from Linux shouldn't be hard.  The OS-dependent bits are
    the use of /dev/random and the fixed-size data types defined in
    <sys/types.h>.  Linux uses "u_int32_t", Solaris uses "uint32_t".
    That sort of thing.  Solaris also lacks getopt_long() found in
    glibc.

(4) This probably does require gcc to build.  I use the GNU "long long"
    datatype quite a bit for manipulating 64-bit values.

(5) The tuning in response to the current error rate is still under
    active research and development.  Future releases may change this
    code significantly.

(6) Disk-to-disk on the same box is a bad test platform.  The
    scheduling daemon and the behavior of the loopback device make
    everything go to hell.

(7) The client has a limited amount of online help.  Use 'help' to
    see it.

(8) The server has a limited amount of usage information.  Run it
    with the '--help' option to see it.
