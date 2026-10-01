- **NFS with TLS support added (Technology Preview)**  
  *Changed:* RHEL 10 now supports NFS over TLS for encrypted RPC traffic.  
  *Why it matters:* Enhances security by enabling encrypted communication between NFS clients and servers, though kTLS is still a preview feature.

- **Improved NFS client-side caching with FS-Cache**  
  *Changed:* FS-Cache integration with NFS allows client-side caching using the `cachefilesd` service.  
  *Why it matters:* Reduces server load and network bandwidth usage by caching frequently accessed files locally.

- **Fixed IPv6 address handling in NFS URLs**  
  *Changed:* ReaR now correctly parses NFS URLs with IPv6 addresses enclosed in square brackets.  
  *Why it matters:* Enables proper use of IPv6-based NFS mounts in backup tools like ReaR without workarounds.

- **New mount options for NFS performance tuning**  
  *Changed:* Options like `rsize`, `wsize`, `retrans`, `timeo`, and `sec=` are documented for fine-tuning NFS performance and security.  
  *Why it matters:* Allows administrators to optimize NFS performance and security based on network conditions and use cases.

- **NFS client-side caching back end support (ext3, ext4, XFS)**  
  *Changed:* FS-Cache supports specific file systems for caching back ends.  
  *Why it matters:* Provides reliable local caching infrastructure for NFS traffic, improving read performance and reducing server load.