- **NFS with TLS support** (EXCERPT 1):
  - NFS now supports fully encrypted communication between clients and servers using TLS.
  - This enhances security but relies on kernel TLS (kTLS), which is a Technology Preview in RHEL 10.
  - Performance impact: Encrypted traffic may introduce overhead compared to unencrypted NFS.

- **Fixed NFS IPv6 address parsing in ReaR** (EXCERPT 3):
  - ReaR now correctly interprets square brackets in IPv6 addresses when used in NFS URLs (e.g., `nfs://[2001:db8:ca2:6::101]/root/REAR`).
  - Why it matters: Previously, using IPv6 addresses in NFS or sshfs URLs caused errors or required workarounds with backslashes.
  - Performance impact: Enables proper use of IPv6 addresses in NFS-related backup or output configurations without errors.

- **FS-Cache (client-side caching) for NFS** (EXCERPT 5):
  - NFS can now cache data locally using FS-Cache, reducing server load and improving performance for repeated reads.
  - Enabled via mount options; cached files are stored in a block-based file system (ext3, ext4, or XFS) under `/var/cache/fscache/`.
  - Why it matters: Transparent caching improves efficiency for NFS clients, especially in high-latency or bandwidth-constrained networks.
  - Performance impact: Reduces network traffic and server I/O but may slightly slow down the client due to cache management overhead.