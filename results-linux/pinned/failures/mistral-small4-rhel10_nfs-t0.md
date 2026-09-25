- **NFS with TLS support** (Technology Preview in RHEL 10):
  - NFS now supports TLS for RPC traffic to encrypt communication between clients and servers.
  - Matters for secure and encrypted NFS operations, though performance impact may be higher due to encryption overhead.

- **RHEL 10 Managing file systems — Common NFS mount options**:
  - `rsize=num` and `wsize=num`:
    - Default and maximum values increased to 1,048,576 bytes (1 MiB).
    - Why: Larger read/write sizes reduce network overhead and improve throughput for NFS operations.
  - `sec=krb5p`:
    - Uses Kerberos V5 for authentication, integrity checking, and encryption.
    - Why: Most secure option, but may introduce performance overhead due to encryption.

- **RHEL 10 Managing file systems — Enabling client-side caching of NFS content**:
  - FS-Cache (client-side caching) now supported for NFS to cache data locally.
  - Why: Reduces server load and network bandwidth usage, improving performance for repeated reads.
  - Cache backends supported: XFS, ext3, ext4.
  - Why: These file systems support block mapping and extended attributes required for caching.
  - `cachefilesd` service introduced to manage caching.
  - Why: Provides a way to configure and manage client-side caching transparently.