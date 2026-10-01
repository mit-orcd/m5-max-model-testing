- **NFS with TLS support added (RHEL 10.0)** – TLS is now fully supported for NFS, encrypting RPC traffic between clients and servers. This improves security without requiring application changes, though it relies on kTLS (provided as a Technology Preview).

- **NFS client supports `vers` as alias for `nfsvers` (RHEL 10.0)** – The `vers` mount option is now equivalent to `nfsvers`, included for compatibility. This simplifies configuration and aligns with standard NFS mount option naming.

- **Increased default and maximum `rsize`/`wsize` values for NFS (RHEL 10.0)** – The maximum transfer size for NFS read/write operations is now 1,048,576 bytes on both client and server. Larger I/O sizes can improve performance by reducing protocol overhead and increasing throughput.

- **IPv6 address support in `nfs://` URLs for ReaR (RHEL 10.0)** – Fixed parsing of IPv6 addresses enclosed in square brackets in `nfs://` URLs used in `BACKUP_URL` and `OUTPUT_URL`. This enables correct backup and restore operations over NFS when using IPv6.

- **FS-Cache integration with NFS remains supported (RHEL 10.0)** – Client-side caching via FS-Cache and `cachefilesd` continues to be available for NFS mounts. This can reduce server load and improve performance for repeated reads, especially over high-latency networks.