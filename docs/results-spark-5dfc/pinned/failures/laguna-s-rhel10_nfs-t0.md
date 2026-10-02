- **NFS with TLS support added (RHEL 10.0)** – TLS is now fully supported for NFS, encrypting RPC traffic between clients and servers. This improves security without requiring application changes, though it depends on kernel TLS (kTLS), which is still a Technology Preview.

- **NFS client now supports `vers` as an alias for `nfsvers` (RHEL 10.0)** – The `vers` mount option is now equivalent to `nfsvers`, improving compatibility with existing scripts and configurations that use the shorter option name.

- **Increased default and maximum `rsize`/`wsize` values for NFS (RHEL 10.0)** – The maximum transfer size for NFS read/write operations is now 1,048,576 bytes on both client and server, improving performance by allowing larger data transfers per operation.

- **IPv6 address support fixed in NFS URLs for ReaR (RHEL 10.0)** – Square brackets around IPv6 addresses in `nfs://` URLs are now correctly parsed, enabling backup and restore operations to NFS servers over IPv6 without workarounds.

- **FS-Cache integration with NFS remains supported (RHEL 10.0)** – Client-side caching of NFS content via FS-Cache and `cachefilesd` is documented and supported, allowing local caching of remote NFS data to reduce network and server load, with support for XFS, ext3, and ext4 as cache back ends.