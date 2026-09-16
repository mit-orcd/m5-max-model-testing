- **NFS with TLS support** – RHEL 10 now fully supports TLS for RPC traffic on NFS servers and clients.  The change encrypts all NFS packets, improving security; it may add a small CPU overhead on both sides, but enables encrypted file‑system access over potentially untrusted networks.

- **IPv6 bracket parsing bug in ReaR** – A bug that caused ReaR to reject `nfs://[IPv6]/…` URLs (treating the brackets as shell metacharacters) has been fixed.  This restores the ability to use IPv6 NFS endpoints in backup/restore scripts without quoting or escaping, improving reliability of automated backups.

- **FS‑Cache client‑side NFS caching** – RHEL 10 introduces FS‑Cache for NFS, with the `cachefiles` back‑end and `cachefilesd` daemon.  When enabled, read/write traffic is served from a local cache (ext3/ext4/XFS), reducing network traffic and server load and speeding up client access.  The feature is optional and requires a block‑based filesystem with extended attributes under `/var/cache/fscache/`.

- **Default rsize/wsize maximum of 1 MiB** – The NFS client and server now default to the largest supported block size (1,048,576 bytes) for reads and writes.  Using the full block size improves throughput by reducing the number of round‑trips per transfer, especially on high‑latency networks.

(Other mount options listed in the documentation are informational only and do not represent bug fixes or performance changes.)