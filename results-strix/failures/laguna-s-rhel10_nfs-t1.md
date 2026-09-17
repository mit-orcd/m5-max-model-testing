- **NFS with TLS support is now fully supported in RHEL 10**  
  - What changed: NFS can now use Transport Layer Security (TLS) to encrypt RPC traffic between clients and servers.  
  - Why it matters: Enhances security by protecting data in transit, but relies on kernel TLS (kTLS), which is provided as a Technology Preview.

- **IPv6 address handling fixed in NFS URLs for ReaR**  
  - What changed: ReaR no longer misinterprets square brackets around IPv6 addresses in `nfs://` or `sshfs://` URLs.  
  - Why it matters: Enables proper use of IPv6 addresses in backup/output URLs without workarounds.

- **Increased default and maximum values for `rsize` and `wsize` in NFS mounts**  
  - What changed: The maximum supported `rsize` and `wsize` values are now 1,048,576 bytes on both NFS clients and servers.  
  - Why it matters: Improves NFS performance by allowing larger data transfers per operation.

- **Client-side caching via FS-Cache is documented for NFS performance optimization**  
  - What changed: RHEL 10 supports enabling NFS client-side caching using FS-Cache with the `cachefiles` backend and `cachefilesd` daemon.  
  - Why it matters: Reduces network load and improves response times by caching remote NFS content locally.