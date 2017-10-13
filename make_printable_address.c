int make_printable_address(const struct sockaddr_in6 *const addr,
                           const socklen_t addr_len,
                           char const* buffer,
                           const size_t buffer_size) {
    char printable[INET6_ADDRSTRLEN];
    assert(addr_len == sizeof(*addr));
    if(inet_ntop(addr->sin6_family, &(addr->sin6_addr), printable, sizeof(printable)) == printable) {
        snprintf(buffer, buffer_size, "%s port %d", printable, ntohs(addr->sin6_port));
    } else {
        perror("inet_ntop");
        snprintf(buffer, buffer_size, "unparseable address");
    }

    return strdup(buffer);

}
