- **NFS with TLS support** (Technology Preview in RHEL 10):
  - NFS now supports TLS for RPC traffic to encrypt communication between clients and servers.
  - Why it matters: Enhances NFS security by preventing traffic sniffing, though it may introduce performance overhead due to encryption.

- **NFS client IPv6 address handling** (Fixed in ReaR):
  - IPv6 addresses in NFS URLs (e.g., `nfs://[2001:db8:ca2:6::101]/root/REAR`) are now correctly parsed without errors.
  - Why it matters: Enables seamless use of IPv6 addresses in NFS configurations, avoiding premature termination or workarounds.

- **NFS mount options (`rsize`, `wsize`)**:
  - Default and maximum values for `rsize` and `wsize` increased to **1,048,576 bytes** (1 MiB) in RHEL 10.
  - Why it matters: Larger read/write sizes improve NFS performance by reducing the number of operations needed for data transfer.

- **NFS client-side caching with FS-Cache**:
  - NFS shares can now be mounted with **FS-Cache enabled** to cache data locally, reducing server load and improving performance.
  - Why it matters: Transparently improves NFS performance for repeated reads by serving data from local cache instead of the network.

- **Cache backends for FS-Cache**:
  - Supported cache backends are **XFS**, **ext3**, and **ext4** (must support block mapping and extended attributes).
  - Why it matters: Ensures compatibility with common local file systems for caching NFS content.

- **`cachefilesd` service configuration**:
  - The `cachefilesd` daemon manages client-side caching for NFS via FS-Cache.
  - Why it matters: Proper configuration is essential for enabling and optimizing NFS caching performance.