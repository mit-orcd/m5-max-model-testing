- **NFS with TLS support added (Technology Preview)**  
  *Changed:* RHEL 10 now supports NFS over TLS, encrypting RPC traffic between clients and servers.  
  *Why it matters:* Enhances security for NFS communications, especially in environments requiring encrypted data transfer.

- **Improved NFS client-side caching with FS-Cache**  
  *Changed:* FS-Cache integration allows client-side caching of NFS content using the `cachefiles` backend.  
  *Why it matters:* Reduces server load and network bandwidth usage by locally caching frequently accessed files.

- **Fixed IPv6 address handling in NFS URLs**  
  *Changed:* ReaR now correctly parses NFS URLs with IPv6 addresses enclosed in square brackets.  
  *Why it matters:* Enables proper use of IPv6-based NFS mounts in backup tools like ReaR without workarounds.

- **Updated NFS mount options and security settings**  
  *Changed:* Clarified behavior of `rsize`, `wsize`, `sec`, and other mount options for better performance and compatibility.  
  *Why it matters:* Provides more control over NFS performance tuning and security, especially with Kerberos-based authentication.

- **Improved NFS client retransmission and timeout behavior**  
  *Changed:* Default retry and timeout values for NFS over TCP are now configurable and more predictable.  
  *Why it matters:* Helps optimize NFS performance over unreliable or high-latency networks.

- **NFS server port handling improvements**  
  *Changed:* NFSv4 clients no longer query `rpcbind` if port is unspecified, defaulting to standard NFS port 2049.  
  *Why it matters:* Simplifies NFSv4 setup and improves connection reliability when `rpcbind` is not available.