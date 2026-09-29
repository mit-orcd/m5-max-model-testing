- **NFS with TLS support added (RHEL 10.0)** – TLS is now fully supported for NFS, encrypting RPC traffic between clients and servers via kernel TLS (kTLS). Improves security without changing core NFS functionality; kTLS itself remains a Technology Preview.

- **NFS client supports larger rsize/wsize values (up to 1MB)** – Default and maximum read/write buffer sizes for NFS mounts increased to 1,048,576 bytes on both client and server. Enhances throughput for large data transfers.

- **NFSv4 mount behavior change for port specification** – When `port` is not specified for NFSv4, the client uses the standard port (TCP 2049) directly instead of querying `rpcbind`. Reduces connection latency and dependency on rpcbind for NFSv4 mounts.

- **ReaR fixed to support IPv6 addresses in nfs:// URLs** – Square brackets around IPv6 addresses in `nfs://` URLs are no longer misinterpreted as shell metacharacters. Enables correct parsing and use of IPv6-based NFS backup locations in ReaR.

- **FS-Cache integration with NFS remains supported** – Client-side caching via FS-Cache and `cachefilesd` continues to be available for NFS, allowing local caching of remote content to reduce server load and network usage. No new changes noted, but confirms ongoing support.