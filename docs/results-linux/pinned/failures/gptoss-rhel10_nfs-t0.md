- **NFS with TLS support (new feature)** – RPC traffic is now encrypted, giving clients and servers end‑to‑end confidentiality.  The added cryptographic overhead can slightly reduce throughput, but the feature is essential for secure deployments.  

- **Fixed IPv6 address parsing in `nfs://` URLs (bug fix)** – ReaR and other tools that use `nfs://[IPv6]` URLs no longer abort; IPv6 addresses can now be specified in backup/restore URLs, improving reliability of NFS mounts over IPv6.  

- **Default `rsize`/`wsize` increased to 1 048 576 bytes (performance improvement)** – The client and server now negotiate a larger transfer block size by default, reducing the number of round‑trips for large reads/writes and boosting overall NFS throughput.  

- **FS‑Cache support for NFS (new feature)** – Clients can enable the `cachefiles` back‑end (`cachefilesd` daemon) to cache NFS data locally.  This reduces network traffic and server load, improves read performance, and provides persistence across reboots (requires a block‑based FS with extended attributes such as XFS, ext3, or ext4).