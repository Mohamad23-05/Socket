#ifndef PACKET_H
#define PACKET_H

#define MAX_TEXT_LEN 1024

struct packet {
    char text[MAX_TEXT_LEN];
    int sNumber;
};

#endif /* PACKET_H */
