- **NFS with TLS support**
  - What changed: NFS now supports TLS encryption for RPC traffic.
  - Why it matters: Enhances security by ensuring encrypted communication between NFS clients and servers.

- **IPv6 address support in NFS URLs**
  - What changed: Fixed parsing of IPv6 addresses enclosed in square brackets (e.g., `[::1]`) in `nfs://` URLs.
  - Why it matters: Enables proper use of IPv6 addresses in NFS mount commands, improving compatibility.

- **NFS mount options (existing documentation)**
  - What changed: No new changes, but documented options like `rsize`, `wsize`, `retrans`, `timeo`, and `sec` (for security) remain relevant for performance tuning.
  - Why it matters: These options allow tuning NFS client/server performance (e.g., read/write size, retries, encryption overhead).

- **FS-Cache for NFS client-side caching**
  - What changed: FS-Cache support for NFS remains available (no new changes, but reinforced in docs).
  - Why it matters: Improves NFS client performance by caching data locally, reducing network/server load.