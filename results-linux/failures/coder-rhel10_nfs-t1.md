- **NFS TLS Support Introduced (Technology Preview)**  
  *Changed:* RHEL 10 now supports NFS with Transport Layer Security (TLS) for encrypted RPC traffic.  
  *Why it matters:* Enhances security for NFS communications, especially in environments requiring encrypted data transfer between clients and servers.

- **Improved NFS Client-Side Caching with FS-Cache**  
  *Changed:* FS-Cache support is added for NFS client-side caching using the `cachefiles` backend.  
  *Why it matters:* Reduces server load and improves performance by caching frequently accessed NFS data locally, enabling efficient local data retrieval.

- **IPv6 Address Handling in NFS URLs Fixed**  
  *Changed:* ReaR now correctly parses IPv6 addresses in NFS URLs without requiring escaping.  
  *Why it matters:* Fixes an issue that previously prevented correct handling of IPv6-based NFS mount URLs in backup tools like ReaR.