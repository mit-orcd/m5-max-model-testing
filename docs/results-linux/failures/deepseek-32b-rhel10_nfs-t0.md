

- **NFS with TLS support**: NFS now supports TLS for RPC traffic, enhancing security by encrypting communication between clients and servers. This matters because it improves data security over the network.

- **ReaR handling of IPv6 addresses in NFS URLs**: A bug was fixed where ReaR would fail when using IPv6 addresses in NFS URLs. This matters because it allows proper use of IPv6 addresses with NFS, avoiding premature errors.

- **FS-Cache for NFS clients**: FS-Cache is enabled by default for NFS mounts, improving performance by caching NFS content locally. This matters because it reduces network load and improves access times for NFS clients.

- **NFS mount options updates**: The `rsize` and `wsize` options now have a maximum value of 1,048,576 bytes, improving performance. The `sec` option now includes more security options, affecting both security and performance. This matters because it allows for better customization of NFS mounts for different security and performance needs.