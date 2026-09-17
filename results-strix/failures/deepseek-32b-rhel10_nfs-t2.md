

- **NFS with TLS Support**: Introduced to enhance security by enabling encrypted communication between clients and servers using TLS, relying on kernel TLS (a Technology Preview). This improves data protection and can reduce overhead if kTLS is effective.

- **Client-Side Caching with FS-Cache**: Added to boost NFS performance by caching content locally, reducing network traffic and server load, while supporting multiple file systems like XFS, ext3, and ext4.

- **ReaR IPv6 Handling Fix**: Resolved a bug where using IPv6 addresses in NFS URLs caused errors, ensuring proper functionality for NFS setups utilizing IPv6 addresses.