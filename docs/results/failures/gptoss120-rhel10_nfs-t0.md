- **NFS with TLS (kTLS) support added** – NFS traffic can now be encrypted using kernel TLS (kTLS, Technology Preview). This improves security for RPC communication; the use of kTLS may add CPU overhead, so administrators can weigh the trade‑off between confidentiality and raw throughput.

- **Default maximum `rsize`/`wsize` raised to 1 MiB** – In RHEL 10 the client and server negotiate a maximum read/write size of 1 048 576 bytes (up from previous limits). Larger I/O chunks reduce the number of round‑trips and increase overall NFS throughput, especially on high‑latency or high‑bandwidth networks.

- **FS‑Cache client‑side caching for NFS** – The new FS‑Cache integration (via the `cachefiles` backend and `cachefilesd` daemon) lets NFS clients cache file data locally on ext3/ext4/XFS. This can dramatically cut network traffic and server load for read‑heavy workloads, though it may add a small local‑cache overhead on the client.

- **Bug fix: IPv6 address handling in `nfs://` URLs (ReaR)** – ReaR now correctly parses IPv6 literals enclosed in `[…]` for `nfs://` (and `sshfs://`) schemes. The previous mis‑interpretation caused backup/restore jobs that used NFS over IPv6 to abort. The fix restores reliable NFS client operation in automated backup scenarios.

- **Mount‑option alias `vers` added (compatibility)** – The `vers` option is now accepted as a synonym for `nfsvers`. While not a performance change, it smooths migration of scripts and tools that rely on the older name.