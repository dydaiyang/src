/*	$OpenBSD: lka_report.c,v 1.7 2018/11/08 13:21:00 gilles Exp $	*/

/*
 * Copyright (c) 2018 Gilles Chehade <gilles@poolp.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <errno.h>
#include <event.h>
#include <imsg.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smtpd.h"
#include "log.h"

#define	PROTOCOL_VERSION	1

static void
report_smtp_broadcast(const char *format, ...)
{
	va_list		ap;
	void		*hdl = NULL;
	const char	*reporter;

	va_start(ap, format);
	while (dict_iter(env->sc_smtp_reporters_dict, &hdl, &reporter, NULL)) {
		if (io_vprintf(lka_proc_get_io(reporter), format, ap) == -1)
			fatalx("failed to write to processor");
	}
	va_end(ap);
}

void
lka_report_smtp_link_connect(time_t tm, uint64_t reqid, const char *rdns,
    const struct sockaddr_storage *ss_src,
    const struct sockaddr_storage *ss_dest)
{
	char	src[NI_MAXHOST + 5];
	char	dest[NI_MAXHOST + 5];
	uint16_t	src_port = 0;
	uint16_t	dest_port = 0;

	if (ss_src->ss_family == AF_INET)
		src_port = ntohs(((const struct sockaddr_in *)ss_src)->sin_port);
	else if (ss_src->ss_family == AF_INET6)
		src_port = ntohs(((const struct sockaddr_in6 *)ss_src)->sin6_port);

	if (ss_dest->ss_family == AF_INET)
		dest_port = ntohs(((const struct sockaddr_in *)ss_dest)->sin_port);
	else if (ss_dest->ss_family == AF_INET6)
		dest_port = ntohs(((const struct sockaddr_in6 *)ss_dest)->sin6_port);

	(void)strlcpy(src, ss_to_text(ss_src), sizeof src);
	(void)strlcpy(dest, ss_to_text(ss_dest), sizeof dest);

	report_smtp_broadcast("report|%d|%zd|smtp-in|link-connect|"
	    "%016"PRIx64"|%s|%s:%d|%s:%d\n",
	    PROTOCOL_VERSION,
	    tm, reqid, rdns, src, src_port, dest, dest_port);
}

void
lka_report_smtp_link_disconnect(time_t tm, uint64_t reqid)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|link-disconnect|"
	    "%016"PRIx64"\n",
	    PROTOCOL_VERSION, tm, reqid);
}

void
lka_report_smtp_link_tls(time_t tm, uint64_t reqid, const char *ciphers)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|link-tls|"
	    "%016"PRIx64"|%s\n",
	    PROTOCOL_VERSION, tm, reqid, ciphers);
}

void
lka_report_smtp_tx_begin(time_t tm, uint64_t reqid, uint32_t msgid)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|tx-begin|"
	    "%016"PRIx64"|%08x\n",
	    PROTOCOL_VERSION, tm, reqid, msgid);
}

void
lka_report_smtp_tx_envelope(time_t tm, uint64_t reqid, uint32_t msgid, uint64_t evpid)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|tx-envelope|"
	    "%016"PRIx64"|%08x|%016"PRIx64"\n",
	    PROTOCOL_VERSION, tm, reqid, msgid, evpid);
}

void
lka_report_smtp_tx_commit(time_t tm, uint64_t reqid, uint32_t msgid, size_t msgsz)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|tx-commit|"
	    "%016"PRIx64"|%08x|%zd\n",
	    PROTOCOL_VERSION, tm, reqid, msgid, msgsz);
}

void
lka_report_smtp_tx_rollback(time_t tm, uint64_t reqid)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|tx-rollback|"
	    "%016"PRIx64"\n",
	    PROTOCOL_VERSION, tm, reqid);
}

void
lka_report_smtp_protocol_client(time_t tm, uint64_t reqid, const char *command)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|protocol-client|"
	    "%016"PRIx64"|%s\n",
	    PROTOCOL_VERSION, tm, reqid, command);
}

void
lka_report_smtp_protocol_server(time_t tm, uint64_t reqid, const char *response)
{
	report_smtp_broadcast("report|%d|%zd|smtp-in|protocol-server|"
	    "%016"PRIx64"|%s\n",
	    PROTOCOL_VERSION, tm, reqid, response);
}
