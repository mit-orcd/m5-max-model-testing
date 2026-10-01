- **NFS with TLS support added (RHEL 10.0)** – TLS is now fully supported for NFS, encrypting RPC traffic between clients and servers via kernel TLS (kTLS). Improves security without requiring application changes; kTLS itself remains a Technology Preview.

- **NFS client supports larger rsize/wsize (up to 1MB) (RHEL 10.0)** – Maximum read/write buffer sizes increased to 1,048,576 bytes on both client and server. Enhances NFS performance by allowing larger data transfers per operation.

- **IPv6 address support fixed in nfs:// URLs for ReaR (RHEL 10.0)** – Square brackets around IPv6 addresses in `nfs://` URLs are no longer misinterpreted as shell metacharacters. Enables correct use of IPv6 NFS URLs in backup/restore configurations.

- **FS-Cache client-side caching documentation updated (RHEL 10.0)** – Clarifies use of `cachefiles` and `cachefilesd` for NFS caching, including supported backends (XFS, ext3, ext4) and behavior with partial caching and I/O error handling. Helps optimize NFS client performance and reduce server load.