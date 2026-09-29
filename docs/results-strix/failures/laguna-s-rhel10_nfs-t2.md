- **NFS with TLS support added (RHEL 10.0)**: TLS is now fully supported for NFS, encrypting RPC traffic between clients and servers for improved security. Relies on kernel TLS (kTLS), which is available as a Technology Preview.

- **NFSv4 client no longer checks rpcbind when port is not specified (NFS mount options)**: For NFSv4, if the `port` option is not specified, the client now defaults to TCP port 2049 without querying `rpcbind`, reducing connection overhead and avoiding potential rpcbind-related issues.

- **Increased default NFS rsize/wsize to 1MB (NFS mount options)**: The maximum supported `rsize` and `wsize` values are now 1,048,576 bytes on both client and server in RHEL 10, improving NFS performance by allowing larger data transfers per operation.

- **IPv6 address support fixed in NFS URLs for ReaR (Bug fix)**: ReaR no longer misinterprets square brackets around IPv6 addresses in `nfs://` URLs, allowing proper use of IPv6 NFS servers in backup configurations without workarounds.

- **FS-Cache integration for NFS client-side caching (Managing file systems)**: NFS clients can now use FS-Cache with the `cachefiles` backend to locally cache remote content, reducing server load and network usage. Supports partial caching and transparent integration with XFS, ext3, and ext4.