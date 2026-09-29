*   **NFS over TLS Support**
    *   **Change:** RHEL 10 now fully supports Network File System (NFS) with Transport Layer Security (TLS) for RPC traffic, utilizing kernel TLS (kTLS).
    *   **Why it matters:** This enhances security by encrypting communication between clients and servers; note that kTLS is currently provided as a Technology Preview.

*   **IPv6 Address Support in NFS URLs**
    *   **Change:** The ReaR backup tool now correctly interprets square brackets around IPv6 addresses (e.g., `nfs://[::1]/...`) in `BACKUP_URL` or `OUTPUT_URL` configurations without requiring escape characters.
    *   **Why it matters:** This fixes a bug where using IPv6 addresses in NFS URLs caused backup operations to abort prematurely with "Invalid scheme" errors, allowing direct and reliable configuration of IPv6 NFS targets.

*   **NFS Client-Side Caching (FS-Cache)**
    *   **Change:** RHEL 10 includes FS-Cache support for NFS, allowing clients to cache NFS content locally on block-based file systems (XFS, ext3, ext4) via the `cachefilesd` service.
    *   **Why it matters:** This significantly improves performance by satisfying read requests locally, which reduces server load and network bandwidth consumption, though it may introduce slight client-side latency.

*   **NFS Read/Write Size Defaults**
    *   **Change:** The default maximum value for `rsize` and `wsize` (read/write buffer sizes) in RHEL 10 is now 1,048,576 bytes (1 MiB), which is the maximum supported by both the client and server.
    *   **Why it matters:** This ensures optimal throughput for large file transfers by default, as the system automatically negotiates the largest possible block size supported by both ends of the connection.