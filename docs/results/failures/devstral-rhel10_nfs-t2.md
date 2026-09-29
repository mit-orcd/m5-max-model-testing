- **NFS with TLS support**
  - Added full support for TLS encryption in NFS (RPC-over-TLS).
  - Enhances security by encrypting client-server communication.
  - Relies on kernel TLS (kTLS), which is a Technology Preview in RHEL 10.

- **CIFS client special file creation via NFS reparse points**
  - CIFS client can now create native SMB symlinks and special files (char/block devices, pipes, sockets) through NFS or WSL reparse points using the `reparse=default|nfs|wsl` mount option.
  - Improves interoperability between NFS and SMB environments.

- **IPv6 address support in NFS URLs**
  - Fixed parsing of IPv6 addresses (enclosed in square brackets) in `nfs://` URLs (e.g., `nfs://[::1]/path`).
  - Previously required workarounds like escaping brackets; now works natively.
  - Affects tools like ReaR using NFS URLs with IPv6.

- **NFS mount options documentation updates**
  - Confirmed `vers` and `nfsvers` are synonymous.
  - Detailed performance-related options:
    - `retrans=num`: Retry attempts before recovery (default: 3 for UDP, 2 for TCP).
    - `timeo=num`: Timeout in tenths of a second (default: 600 for TCP, linear backoff up to 600s).
    - `rsize/wsize=num`: Max bytes per read/write (default max: 1,048,576 bytes).
    - `sec=krb5[k|i|p]`: Security modes (krb5p adds encryption but increases overhead).
  - Options like `noacl`, `nolock`, `noexec`, `nosuid` for compatibility/performance tuning.

- **Client-side caching (FS-Cache) for NFS**
  - FS-Cache (via `cachefilesd`) improves NFS performance by caching data locally.
  - Reduces server/network load and enables partial caching of large files.
  - Supports ext3/ext4/XFS as cache backends (mounted at `/var/cache/fscache/`).
  - Transparent to users but may slow clients slightly due to cache management overhead.

- **Multipath and NVMe fixes (indirectly affecting storage performance)**
  - Fixed `multipathd` crash with `ontap` prioritizer (NetApp storage).
  - Resolved memory leak in native NVMe multipathing with `enable_foreign` monitoring.
  - Improved stability for high-availability NFS deployments.