

- **NFS with TLS support**: NFS now supports TLS for encrypted communication, enhancing security by encrypting RPC traffic between clients and servers. This relies on kernel TLS (kTLS), which is available as a Technology Preview.
  - *Why it matters*: Improves security by encrypting data in transit, reducing the risk of data interception.

- **ReaR bug fix for IPv6 addresses in nfs:// URLs**: ReaR (Recovery and Reinstallation) now correctly interprets IPv6 addresses in `nfs://` URLs when enclosed in square brackets, resolving a bug that caused premature abortion of operations.
  - *Why it matters*: Enables proper handling of IPv6 addresses in NFS URLs, avoiding errors during backups or installations.

- **FS-Cache for NFS client-side caching**: NFS clients can now leverage FS-Cache to improve performance by caching NFS content locally, reducing network traffic and server load.
  - *Why it matters*: Enhances performance by reducing network latency and server load, especially for read-heavy workloads.

- **Fix for NFS mount options compatibility**: The `vers` option is included as an alias for `nfsvers` for compatibility with older systems.
  - *Why it matters*: Ensures compatibility with systems that use `vers` instead of `nfsvers`, simplifying NFS configuration.

- **Improved NFS security options**: NFS now supports enhanced security options, including Kerberos V5 (krb5), krb5i (with integrity checking), and krb5p (with encryption), providing more secure communication options.
  - *Why it matters*: Offers stronger security for NFS traffic, protecting against data tampering and unauthorized access.