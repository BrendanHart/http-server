#ifndef MAKE_PRINTABLE_ADDRESS_H
#define MAKE_PRINTABLE_ADDRESS_H

#include <netinet/in.h>
char* make_printable_address(const struct sockaddr_in6 *const addr,
                           const socklen_t addr_len,
                           char const* buffer,
                           const size_t buffer_size);

#endif /* MAKE_PRINTABLE_ADDRESS_H */
