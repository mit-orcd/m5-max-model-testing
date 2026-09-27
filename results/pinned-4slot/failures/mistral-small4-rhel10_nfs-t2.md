- **NFS with TLS support** (EXCERPT 1):
  - NFS now supports fully encrypted communication using TLS for RPC traffic.
  - This matters for security but may introduce performance overhead compared to unencrypted NFS.

- **`fstrim` enabled by default on LUKS2 root in ostree-based installations** (EXCERPT 2):
  - `fstrim` (discards) is now automatically enabled in LUKS2 metadata for new encrypted NFS root installations.
  - This prevents potential performance issues (unresponsive systems, broken file chooser dialogs) in existing installations, requiring manual refresh (`cryptsetup --allow-discards --persistent refresh`).

- **`vers` mount option added for NFS compatibility** (EXCERPT 4):
  - The `vers` option is now supported as an alias for `nfsvers` for compatibility with older systems.
  - This ensures smoother mounting with NFS servers that expect the `vers` option.

- **Client-side NFS caching with FS-Cache** (EXCERPT 5):
  - NFS clients can now enable FS-Cache for improved performance by reducing server load and network bandwidth usage.
  - Requires a block-based cache back end (XFS, ext3, ext4) mounted under `/var/cache/fscache/` with extended attributes.
  - Performance trade-off: local caching may slow down individual file operations but reduces overall server and network I/O.