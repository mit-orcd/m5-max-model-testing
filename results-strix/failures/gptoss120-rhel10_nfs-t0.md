- **NFS with TLS (kTLS) support added** – NFS traffic can now be encrypted via kernel TLS. This improves security for client‑server communication; because it relies on the kTLS Technology Preview, administrators should evaluate any performance impact of the added encryption overhead.

- **IPv6 URL parsing bug fixed for NFS (ReaR)** – The backup tool ReaR now correctly handles `nfs://[IPv6‑address]/…` URLs. Users can reliably use NFS as a backup destination over IPv6 without needing work‑arounds, eliminating failures that could interrupt automated backup jobs.

- **Read/Write size defaults increased** – In RHEL 10 the NFS client and server negotiate a maximum `rsize`/`wsize` of **1 048 576 bytes** (up from previous limits). Larger read/write chunks boost data‑transfer throughput and overall NFS performance when both ends support the size.

- **Client‑side FS‑Cache for NFS enabled** – FS‑Cache (via the `cachefiles` backend) can be turned on for NFS mounts, providing local caching of read data. This reduces network traffic and server load, improving read latency on the client, though it may add some local overhead in cache‑heavy workloads.

- **Kerberos security option `sec=krb5p` highlighted** – The most secure NFS security mode (`krb5p`) now encrypts NFS traffic. Documentation notes that this mode incurs the highest performance cost, helping administrators weigh security versus throughput when configuring NFS mounts.