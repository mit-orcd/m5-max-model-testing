- **NFS with TLS support added** – NFS traffic can now be encrypted using TLS (via kernel TLS/kTLS), improving security for RPC communication between clients and servers. This is a Technology Preview feature in RHEL 10.  
- **FS-Cache integration for NFS client-side caching** – Introduced in RHEL 10, FS-Cache allows NFS clients to cache data locally using `cachefilesd` and supported file systems (XFS, ext3, ext4), reducing network load and improving read performance.  
- **Default `rsize` and `wsize` increased to 1,048,576 bytes** – The maximum read/write buffer size for NFS mounts is now 1 MB in RHEL 10, potentially improving throughput for large I/O operations.  
- **IPv6 address support in NFS URLs** – Fixed an issue where IPv6 addresses in `nfs://` URLs (e.g., in ReaR backup configurations) were not parsed correctly due to unescaped square brackets.  
- **Improved NFS mount option handling** – The `vers` option is now explicitly supported as an alias for `nfsvers`, enhancing compatibility.  
- **FS-Cache requires specific file system support** – Caching via FS-Cache requires the cache backend (mounted at `/var/cache/fscache/`) to be on XFS, ext3, or ext4 with extended attributes enabled.  

*Note: No bug fixes directly related to NFS server or client performance were mentioned in the provided excerpts.*
</think>- **NFS with TLS support added** – NFS traffic can now be encrypted using TLS (via kernel TLS/kTLS), improving security for RPC communication between clients and servers. This is a Technology Preview feature in RHEL 10.  
- **FS-Cache integration for NFS client-side caching** – Introduced in RHEL 10, FS-Cache allows NFS clients to cache data locally using `cachefilesd` and supported file systems (XFS, ext3, ext4), reducing network load and improving read performance.  
- **Default `rsize` and `wsize` increased to 1,048,576 bytes** – The maximum read/write buffer size for NFS mounts is now 1 MB in RHEL 10, potentially improving throughput for large I/O operations.  
- **IPv6 address support in NFS URLs** – Fixed an issue where IPv6 addresses in `nfs://` URLs (e.g., in ReaR backup configurations) were not parsed correctly due to unescaped square brackets.  
- **Improved NFS mount option handling** – The `vers` option is now explicitly supported as an alias for `nfsvers`, enhancing compatibility.  
- **FS-Cache requires specific file system support** – Caching via FS-Cache requires the cache backend (mounted at `/var/cache/fscache/`) to be on XFS, ext3, or ext4 with extended attributes enabled.  

*Note: No bug fixes directly related to NFS server or client performance were mentioned in the provided excerpts.*