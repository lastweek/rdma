/*
 * Copyright (c) 2018 Yizhou Shan <ys@purdue.edu>. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * This is ported based on rc_pingpong.c from rdma-core library.
 * Big thanks to all the awesome engineers.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <malloc.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <time.h>
#include <inttypes.h>
#include <infiniband/verbs.h>

static void usage(const char *argv0)
{
	printf("Usage:\n");
	printf("  %s            start a server and wait for connection\n", argv0);
	printf("  %s <host>     connect to server at <host>\n", argv0);
	printf("\n");
	printf("Options:\n");
	printf("  -h, --help             list help information\n");
	printf("  -p, --port=<port>      listen on/connect to port <port> (default 18515)\n");
	printf("  -d, --ib-dev=<dev>     use IB device <dev> (default first device found)\n");
	printf("  -i, --ib-port=<port>   use port <port> of IB device (default 1)\n");
	printf("  -s, --size=<size>      size of message to exchange (default 4096)\n");
	printf("  -r, --rx-depth=<dep>   number of receives to post at a time (default 500)\n");
	printf("  -n, --iters=<iters>    number of exchanges (default 1000)\n");
	printf("  -g, --gid-idx=<gid index> local port gid index\n");
}

int main(int argc, char *argv[])
{
	struct ibv_device      **dev_list;
	struct ibv_device	*ib_dev;
	struct ibv_context	*ib_context;
	char                    *ib_devname = NULL;
	char                    *servername = NULL;
	unsigned int             port = 18515;
	int                      ib_port = 1;
	unsigned int             size = 4096;
	unsigned int             rx_depth = 500;
	unsigned int             iters = 1000;
	int                      rcnt, scnt;
	int			 gidx = -1;
	char			 gid[33];

	while (1) {
		int c;

		static struct option long_options[] = {
			{ .name = "help",     .has_arg = 0, .val = 'h' },
			{ .name = "port",     .has_arg = 1, .val = 'p' },
			{ .name = "ib-dev",   .has_arg = 1, .val = 'd' },
			{ .name = "ib-port",  .has_arg = 1, .val = 'i' },
			{ .name = "size",     .has_arg = 1, .val = 's' },
			{ .name = "rx-depth", .has_arg = 1, .val = 'r' },
			{ .name = "iters",    .has_arg = 1, .val = 'n' },
			{ .name = "gid-idx",  .has_arg = 1, .val = 'g' },
			{}
		};

		c = getopt_long(argc, argv, "p:d:i:s:r:n:g",
				long_options, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage(argv[0]);
			return -1;
		case 'p':
			port = strtoul(optarg, NULL, 0);
			if (port > 65535) {
				usage(argv[0]);
				return -1;
			}
			break;

		case 'd':
			ib_devname = strdupa(optarg);
			break;

		case 'i':
			ib_port = strtol(optarg, NULL, 0);
			if (ib_port < 1) {
				usage(argv[0]);
				return -1;
			}
			break;

		case 's':
			size = strtoul(optarg, NULL, 0);
			break;

		case 'r':
			rx_depth = strtoul(optarg, NULL, 0);
			break;

		case 'n':
			iters = strtoul(optarg, NULL, 0);
			break;

		case 'g':
			gidx = strtol(optarg, NULL, 0);
			break;

		default:
			usage(argv[0]);
			return -1;
		}
	}

	if (optind == argc - 1)
		servername = strdupa(argv[optind]);
	else if (optind < argc) {
		usage(argv[0]);
		return -1;
	}

	dev_list = ibv_get_device_list(NULL);
	if (!dev_list) {
		perror("Failed to get IB devices list");
		return -1;
	}

	if (!ib_devname) {
		ib_dev = *dev_list;
		if (!ib_dev) {
			fprintf(stderr, "No IB devices found\n");
			return -1;
		}
	} else {
		int i;
		for (i = 0; dev_list[i]; ++i)
			if (!strcmp(ibv_get_device_name(dev_list[i]), ib_devname))
				break;
		ib_dev = dev_list[i];
		if (!ib_dev) {
			fprintf(stderr, "IB device %s not found\n", ib_devname);
			return -1;
		}
	}

	printf("INFO: Use IB device %s\n", ibv_get_device_name(ib_dev));

	ib_context = ibv_open_device(ib_dev);
	if (!ib_context) {
		fprintf(stderr, "Couldn't get context for %s\n",
			ibv_get_device_name(ib_dev));
		return -1;
	}

close_device:
	ibv_close_device(ib_context);
	return 0;
}
