- **NFS over TLS support** – RHEL 10 now fully supports NFS with Transport Layer Security (TLS) for RPC traffic.  This adds end‑to‑end encryption for NFS clients and servers, improving data confidentiality and compliance, while introducing a modest performance overhead due to TLS processing.

- **Maximum rsize/wsize set to 1 MiB** – The default maximum read/write block size for NFS mounts is now 1,048,576 bytes.  Larger block sizes reduce the number of round‑trips for large files, boosting throughput on high‑bandwidth links and lowering latency for bulk I/O.

- **FS‑Cache integration for NFS** – RHEL 10 introduces client‑side caching of NFS data via the FS‑Cache framework (cachefilesd).  Cached reads are served locally, cutting network traffic, easing server load, and speeding up repeated access to the same files, at the cost of local storage space.

- **ReaR IPv6 nfs:// URL parsing bug fix** – Fixed a bug where ReaR mis‑parsed IPv6 addresses enclosed in brackets in `nfs://` URLs, causing backup operations to abort.  Now IPv6 addresses can be used directly in backup URLs, simplifying configuration and preventing accidental failures.