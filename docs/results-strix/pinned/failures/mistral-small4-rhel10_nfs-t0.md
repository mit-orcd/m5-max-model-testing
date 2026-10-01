- **NFS with TLS support** (Technology Preview in RHEL 10):
  - NFS now supports encrypted communication between clients and servers using TLS.
  - Requires kernel TLS (kTLS) support, which is also a Technology Preview.
  - Why it matters: Enhances NFS security by preventing traffic sniffing and ensuring encrypted RPC communication, though it may introduce performance overhead due to encryption.

- **NFS mount option fixes and IPv6 support** (RHEL 10):
  - `vers` option added for compatibility with `nfsvers`.
  - Fixed incorrect handling of square brackets in IPv6 addresses when using `nfs://` or `sshfs://` URLs (e.g., `nfs://[::1]/path`).
  - Why it matters: Ensures correct parsing of IPv6 addresses in NFS/sshfs URLs and improves compatibility with older NFS configurations.

- **NFS client-side caching (FS-Cache) improvements** (RHEL 10):
  - NFS shares can now be mounted with FS-Cache enabled for better performance and reduced server load.
  - Caching is transparent and does not require changes to the NFS share itself.
  - Supports partial caching of files (no need to read entire files upfront).
  - Cache errors are hidden from the client.
  - Requires a cache back end (ext3, ext4, or XFS) mounted at `/var/cache/fscache/` with extended attributes enabled.
  - Why it matters: Reduces network latency, improves performance for frequently accessed files, and decreases server load by caching data locally.

- **NFS mount options for performance tuning** (RHEL 10):
  - `rsize=num` and `wsize=num`: Set maximum bytes per read/write operation (default: 1,048,576 bytes).
  - `retrans=num`: Configures NFS client retry count for requests (default: 3 for UDP, 2 for TCP).
  - `timeo=num`: Sets timeout (in tenths of a second) before retrying requests (default: 600 for TCP).
  - `port=num`: Specifies NFS server port (default: 0 queries `rpcbind` for NFSv3, standard port 2049 for NFSv4).
  - `noacl`: Disables ACL processing for compatibility with older NFS servers.
  - `noexec`: Prevents execution of binaries (useful for non-Linux file systems).
  - `nosuid`: Disables `setuid`/`setgid` bits to prevent privilege escalation.
  - Why it matters: These options allow fine-tuning of NFS performance and security based on server/client capabilities and network conditions.

- **NFS security options (`sec=options`)** (RHEL 10):
  - `sec=sys`: Uses local UNIX UIDs/GIDs (default, least secure).
  - `sec=krb5`: Uses Kerberos V5 for authentication (more secure).
  - `sec=krb5i`: Adds integrity checking to Kerberos V5 (prevents tampering).
  - `sec=krb5p`: Encrypts NFS traffic (most secure, highest performance overhead).
  - Why it matters: Provides flexibility to balance security and performance needs.

- **Fixed NFS-related bugs** (RHEL 10):
  - `fstrim` now enabled by default on LUKS2 root in ostree-based installations (prevents unresponsive systems or broken file chooser dialogs).
  - Why it matters: Ensures proper discard handling for encrypted storage, improving system responsiveness.