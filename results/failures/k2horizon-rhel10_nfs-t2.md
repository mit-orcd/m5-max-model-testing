- NFS now supports TLS encryption for RPC traffic, improving security for NFS server/client communication; it depends on kernel TLS (kTLS), which is currently a Technology Preview, so adoption may be limited until kTLS is generally available.

- ReaR (which uses nfs:// URLs) now correctly parses IPv6 addresses enclosed in square brackets in BACKUP_URL/OUTPUT_URL, removing a failure that previously aborted backups when using IPv6 NFS destinations.

- NFS client/server maximum rsize/wsize is now 1,048,576 bytes in RHEL 10, up from previous defaults, allowing larger single I/O transfers and improved throughput for NFS mounts.

- sec=krb5p is documented as the most secure NFS security option (Kerberos authentication + integrity checking + traffic encryption), but it carries the most performance overhead, so it should be used only when security outweighs performance needs.