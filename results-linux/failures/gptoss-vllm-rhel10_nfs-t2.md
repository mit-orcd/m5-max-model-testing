- **NFS TLS support (RHEL 10 0)**
  - NFS now supports Transport Layer Security for RPC traffic, enabled via the kernel TLS (kTLS) technology preview.  
  - **Why it matters:** Encrypts all NFS traffic, protecting data in transit and meeting compliance requirements; may add a small CPU overhead but gives end‑to‑end confidentiality.

- **ReaR IPv6 URL parsing bug fix (RHEL 10 0)**
  - ReaR no longer misinterprets square brackets in `sshfs://` or `nfs://` URLs containing IPv6 addresses; the URLs are parsed correctly and the backup/restore process proceeds.  
  - **Why it matters:** Enables seamless use of IPv6 NFS mounts in backup scripts and restores, eliminating a critical failure point.

- **FS‑Cache client‑side caching for NFS (RHEL 10 0)**
  - The `cachefilesd` daemon and `cachefiles` kernel module are now supported as a cache backend for NFS mounts.  
  - **Why it matters:** Local caching can drastically reduce network traffic and server load for read‑heavy workloads, improving overall throughput at the cost of potential client‑side latency.

- **Default `rsize`/`wsize` values for NFS (RHEL 10 0)**
  - The maximum transfer size for NFS reads and writes defaults to 1,048,576 bytes (1 MiB), the largest value supported by the client and server.  
  - **Why it matters:** Larger transfer units increase throughput for large I/O operations, reducing per‑request overhead and improving performance on high‑bandwidth links.

- **Alias `vers` for `nfsvers` (RHEL 10 0)**
  - The `vers` mount option is now treated as an alias for `nfsvers` for backward compatibility.  
  - **Why it matters:** Simplifies migration from older RHEL releases and ensures consistent behavior across systems, though it does not directly affect performance.