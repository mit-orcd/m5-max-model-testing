

- **NFS with TLS support**  
  - **What changed:** NFS now supports Transport Layer Security (TLS) for encrypted communication between clients and servers.  
  - **Why it matters:** Enhances security by encrypting NFS traffic, preventing data tampering and eavesdropping.

- **Fix for IPv6 address handling in NFS URLs**  
  - **What changed:** ReaR now correctly interprets IPv6 addresses enclosed in square brackets in `nfs://` URLs.  
  - **Why it matters:** Resolves issues where NFS clients could not connect to servers using IPv6 addresses, allowing proper configuration of NFS mounts.

- **FS-Cache for NFS performance optimization**  
  - **What changed:** NFS clients can now use FS-Cache to locally cache frequently accessed files, reducing network latency and server load.  
  - **Why it matters:** Improves performance by enabling faster access to cached data, especially beneficial in high-latency or high-throughput environments.

- **Fix for `multipathd` crashing with ontap prioritizer**  
  - **What changed:** The error logging code in `multipathd` was fixed to prevent crashes when using the ontap prioritizer.  
  - **Why it matters:** Ensures stable operation of `multipathd` when configured with NetApp storage arrays, preventing unexpected service interruptions.

- **Fix for memory leak in `multipathd` with `enable_foreign`**  
  - **What changed:** Memory leaks in `multipathd` monitoring code were resolved when `enable_foreign` was set to monitor NVMe devices.  
  - **Why it matters:** Prevents excessive memory usage and ensures reliable monitoring of natively multipathed NVMe devices.

- **Fix for `fstrim` on LUKS2 root in ostree-based installations**  
  - **What changed:** `fstrim` (discard) is now enabled by default on LUKS2 root mounts during ostree-based installations.  
  - **Why it matters:** Improves performance and reduces wear on SSDs by enabling discard operations, preventing issues like unresponsive systems.

- **NFS mount options for performance tuning**  
  - **What changed:** NFS clients can configure `rsize` and `wsize` to set maximum read/write transfer sizes, up to 1,048,576 bytes.  
  - **Why it matters:** Allows optimization of NFS performance by adjusting data transfer sizes to match network and server capabilities.

- **NFS security options (`sec` parameter)**  
  - **What changed:** NFS clients can now specify security options (`sec=sys`, `sec=krb5`, `sec=krb5i`, `sec=krb5p`) to control authentication and encryption.  
  - **Why it matters:** Provides flexibility in balancing security and performance, with `sec=krb5p` offering the highest security at the cost of increased overhead.