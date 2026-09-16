- **NFS TLS support (RHEL 10.0)** – Added full support for Transport Layer Security (TLS) on NFS, using kernel‑TLS (kTLS) technology preview.  
  *Why it matters:* Encrypts RPC traffic between NFS clients and servers, improving confidentiality and integrity at the cost of a modest CPU overhead for encryption/decryption.

- **IPv6 address parsing bug in `nfs://` URLs (ReaR)** – Fixed the issue where square brackets around an IPv6 address were treated as shell metacharacters, causing ReaR to abort with `ERROR: Invalid scheme ''` when using `nfs://[2001:db8:…]` in `BACKUP_URL` or `OUTPUT_URL`.  
  *Why it matters:* Enables reliable use of IPv6‑only NFS endpoints in backup/restore workflows, preventing unnecessary failures and simplifying configuration.

- **FS‑Cache support for NFS (client‑side caching)** – Introduced FS‑Cache integration for NFS mounts, using the `cachefiles` kernel module and `cachefilesd` daemon, with support for XFS, ext3, and ext4 back‑ends that provide extended attributes.  
  *Why it matters:* Caches frequently accessed NFS data locally, greatly reducing network traffic and server load; the trade‑off is a potential slight slowdown in initial read operations but overall faster subsequent access.