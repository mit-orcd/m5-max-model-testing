- **NFS TLS Support Added**  
  RHEL 10 introduces full support for NFS over TLS (RPC over TLS), encrypting communication between clients and servers. This enhances security by leveraging kernel TLS (kTLS), though kTLS itself is a Technology Preview. Critical for environments requiring encrypted NFS traffic.

- **FS-Cache Documentation Enhanced**  
  Updated documentation details FS-Cache for NFS client-side caching, improving performance by reducing server load and enabling local data retrieval. Requires `cachefilesd` service and compatible filesystems (XFS, ext3/4). Trade-off: may slow clients but reduces network usage.

- **Default `rsize`/`wsize` Increased to 1,048,576 bytes**  
  RHEL 10 sets the maximum read/write buffer size for NFS clients and servers to 1MB, optimizing throughput for large data transfers. Previously, defaults varied; now aligns with modern network capabilities.

- **IPv6 Support in NFS/SSHFS URLs Fixed**  
  Resolved parsing issues where square brackets in IPv6 addresses (e.g., `nfs://[2001:db8::1]/path`) caused errors. Critical for users relying on IPv6 connectivity with NFS or SSHFS-based tools like ReaR.

- **FS-Cache Prerequisites Clarified**  
  Documentation now explicitly states requirements for FS-Cache: a block-based filesystem (XFS/ext3/4) mounted at `/var/cache/fscache/` with extended attributes. Ensures proper caching backend setup for NFS clients.