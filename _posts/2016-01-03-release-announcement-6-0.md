---
layout:      post
title:       Release Notes 6.0
date:        2016-01-03 22:35:45
author:      Balasys Development Team
categories:
  - blog
  - release notes
---

Functionality
-------------

### IPv6 support

#### Protocol support

Support for IPv6 in every component of Zorp supports IPv6 addresses, including zones, subnets, NAT policies, and so on.

### NAT64 and NAT46 support

Network address translation is supported between IPv4 and IPv6 addresses, according to [RFC6052](https://tools.ietf.org/html/rfc6052). This solution is fully compatible with the DNS64 feature of the BIND domain name server.

### Automatic Protocol Recognition

Zorp can inspect the incoming traffic, automatically determine the protocol used in the connection, and start a specified service. Currently it can detect HTTP, SSH, and SSL traffic. For HTTPS connections, you can also select a service based on the certificate of the server.

### Using hostnames in zones

You can directly use hostnames in zones. During startup, Zorp automatically resolves these hostnames to IP (both IPv4 and IPv6) addresses, and updates them periodically to follow any changes in the IP addresses related to the hostname.

### Server Name Indication

Support of the Server Name Indication (SNI) TLS extension, as described in [RFC 6066](https://tools.ietf.org/html/rfc6066). You can configure a mapping between hostnames and certificates, and if the peer sends an SNI request, Zorp automatically selects the matching certificate to show to the peer.

Configuration
-------------

### Simplified services and rules

A new concept called Rules has been introduced. Rules offers a new, simplified view of selecting which service is started when a connection request is received. Rules decide which service to start based on various parameters of the connection request, including client and server IP address, port, protocol, VPN connection, and so on.

### Reusable Encryption policies

Encryption policies are encryption settings (including SSL/TLS settings, certificates, and so on) that are easily reusable between Services and firewall rules. Also, the Zorp SSL framework has been redesigned to make configuration easier and clearer, by allowing you to configure encryption settings based on the scenario you need, for example, ClientOnlyEncryption, ForwardStartTLS, and so on.

Maintenance
-----------

### Single log message as connection summary

Single log messages contain all relevant information about the traffic passing through the firewall. This results in better traceability of traffic and more consistent access to information.

### Extended usage statistics on firewall rules

Usage statistics provide counters for firewall rules, zones and services using the kzorp-client utility.
