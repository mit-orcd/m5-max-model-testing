*   **NFS with TLS support**
    *   **What changed:** RHEL 10 fully supports NFS with Transport Layer Security (TLS) for RPC traffic, utilizing kernel TLS (kTLS), which is available as a Technology Preview.
    *   **Why it matters:** This enhances security by ensuring encrypted communication between NFS clients and servers, protecting data in transit.

*   **ReaR NFS/SSHFS URL parsing for IPv6**
    *   **What changed:** The ReaR backup and restore tool now correctly parses `nfs://` (and `sshfs://`) URLs containing IPv6 addresses enclosed in square brackets (e.g., `nfs://[2001:db8...]/...`) without interpreting the brackets as shell metacharacters.
    *   **Why it matters:** Fixes a bug that caused ReaR to abort prematurely when using IPv6 addresses in NFS backup/output URLs, removing the need for manual workaround escaping.

*   **Increased maximum NFS read/write buffer sizes**
    *   **What changed:** The default maximum values for `rsize` (read) and `wsize` (write) mount options are now 1,048,576 bytes (1 MB) for both client and server.
    *   **Why it matters:** Allows for larger data transfers per operation, which can improve NFS throughput and performance by reducing the number of round-trips required for large file operations.

*   **FS-Cache integration for NFS client-side caching**
    *   **What changed:** The documentation details the use of FS-Cache with the `cachefiles` backend to enable transparent client-side caching for NFS mounts.
    *   **Why it matters:** Improves NFS performance by satisfying read requests locally from the cache, reducing network traffic and lowering server load, though it requires specific filesystem backends (XFS, ext3, ext4) and may introduce latency trade-offs.