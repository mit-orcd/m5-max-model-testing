*   **New Feature: NFS with TLS Support**
    *   **What Changed:** RHEL 10 now fully supports NFS with Transport Layer Security (TLS), relying on kernel TLS (kTLS) to encrypt RPC traffic between clients and servers.
    *   **Why It Matters:** This enhances NFS security by ensuring encrypted communication, protecting data integrity and confidentiality during transmission.

*   **Bug Fix: ReaR URL Parsing for IPv6 NFS Addresses**
    *   **What Changed:** The ReaR tool was fixed to correctly interpret square brackets around IPv6 addresses in `nfs://` URLs (e.g., `nfs://[::1]/...`), which previously caused premature abortion with an "Invalid scheme" error.
    *   **Why It Matters:** Users can now successfully use IPv6 addresses in NFS backup or output URLs without needing complex quoting workarounds, ensuring reliable automated recovery and backup processes.

*   **Performance/Configuration Update: Increased Default `rsize` and `wsize`**
    *   **What Changed:** The maximum byte size for single NFS read (`rsize`) and write (`wsize`) operations has been increased to 1,048,576 bytes (1 MB) in RHEL 10, whereas previous versions used the largest value supported by both client and server without this specific cap.
    *   **Why It Matters:** Larger transfer sizes can improve throughput and reduce overhead for large file transfers, potentially enhancing NFS performance for compatible clients and servers.

*   **Feature: NFS Client-Side Caching via FS-Cache**
    *   **What Changed:** NFS clients can now leverage FS-Cache (using the `cachefiles` backend) to cache NFS content locally on supported file systems (XFS, ext3, ext4).
    *   **Why It Matters:** This reduces network bandwidth consumption and server load by satisfying read requests locally. However, it is a trade-off that may slow down the client itself if the local cache I/O is slower than network access, so it should be configured based on specific performance requirements.