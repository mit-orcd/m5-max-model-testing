- **NFS with TLS support (RHEL 10.0)**  
  *What changed:* NFS now supports Transport Layer Security (TLS) for RPC traffic via kernel TLS (kTLS).  
  *Why it matters:* Encrypts all NFS client‑server communication, protecting data confidentiality and integrity over the network. While the feature is primarily security‑oriented, it also enables compliance with environments that require encrypted NFS traffic, potentially reducing the risk of man‑in‑the‑middle attacks that could otherwise degrade performance.

- **ReaR IPv6 URL parsing bug fix (RHEL 10.0)**  
  *What changed:* ReaR now correctly ignores square brackets in `sshfs://` and `nfs://` URLs containing IPv6 addresses, eliminating the “Invalid scheme ''” error.  
  *Why it matters:* Users can now specify IPv6 NFS endpoints in backup or output URLs without workarounds, preventing abrupt aborts and ensuring reliable use of NFS mounts in backup workflows.

- **Client‑side caching of NFS content via FS‑Cache (RHEL 10.0)**  
  *What changed:* RHEL 10 introduces FS‑Cache integration for NFS, enabling the `cachefiles` back‑end (XFS, ext3, ext4) and the `cachefilesd` daemon to cache NFS data locally.  
  *Why it matters:* Local caching reduces round‑trips to the NFS server, significantly improving read performance and decreasing network bandwidth usage. It also lowers server load, leading to better scalability for workloads with repeated file accesses.