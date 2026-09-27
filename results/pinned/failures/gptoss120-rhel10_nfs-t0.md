- **NFS with TLS support (kTLS)** – RHEL 10 adds full TLS encryption for NFS RPC traffic. It uses the kernel TLS (kTLS) framework (currently a Technology Preview). Encryption secures data in‑flight and, when kTLS is active, can offload crypto to hardware, reducing CPU overhead compared with userspace TLS.  

- **IPv6 address handling in NFS URLs (ReaR bug fix)** – A parsing bug prevented `nfs://[IPv6‑addr]/…` URLs from being accepted by ReaR, causing backup/restore failures. The fix stops the shell from treating the brackets as metacharacters, allowing direct use of IPv6 NFS URLs without work‑arounds.  

- **Increased default `rsize`/`wsize` limits** – The client and server now default to a maximum read/write size of **1 048 576 bytes** (up from earlier, lower defaults). Larger I/O chunks improve NFS throughput, especially over high‑latency or high‑bandwidth links.  

- **Client‑side NFS caching via FS‑Cache** – RHEL 10 ships the `cachefilesd` daemon and enables the `cachefiles` back‑end for FS‑Cache. When the mount option to use FS‑Cache is set, read requests can be satisfied from a local persistent cache (XFS, ext3/4), dramatically reducing network traffic and server load; the trade‑off is a modest increase in client CPU/IO for cache management.  

- **`sec=krb5p` security option noted** – Kerberos 5 with privacy (encryption) is available (`sec=krb5p`). It provides the strongest protection but adds the highest CPU cost, so administrators can weigh security versus performance when choosing NFS security flavors.  

- **`vers` mount option added for compatibility** – The alias `vers` (identical to `nfsvers`) is now recognized, simplifying scripts that specify NFS version without affecting performance.