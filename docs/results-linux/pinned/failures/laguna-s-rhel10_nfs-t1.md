- **NFS with TLS support added (RHEL 10.0)** – TLS is now fully supported for NFS, encrypting RPC traffic between clients and servers via kernel TLS (kTLS). Improves security; kTLS itself is a Technology Preview.

- **NFS client supports `vers` as alias for `nfsvers` (RHEL 10.0)** – The `vers` mount option is now equivalent to `nfsvers`, included for compatibility. No functional change, but clarifies mount option usage.

- **Default `rsize` and `wsize` increased to 1MB (RHEL 10.0)** – Maximum supported NFS read/write transfer size is now 1,048,576 bytes on both client and server. Can improve NFS throughput for large I/O workloads.

- **FS-Cache (`cachefilesd`) supported for NFS client-side caching (RHEL 10.0)** – Enables local caching of NFS content using the `cachefiles` backend, reducing server load and network usage. Requires ext3/ext4/XFS with extended attributes.

- **IPv6 address support fixed in `nfs://` URLs for ReaR (RHEL 10.0)** – Square brackets around IPv6 addresses in `nfs://` URLs are no longer misinterpreted as shell metacharacters. Fixes backup/restore failures when using IPv6 NFS URLs.