#ifndef _RESOLVE_CLNT_H_INCLUDED_
#define _RESOLVE_CLNT_H_INCLUDED_

/*++
/* NAME
/*	resolve_clnt 3h
/* SUMMARY
/*	address resolver client
/* SYNOPSIS
/*	#include <resolve_clnt.h>
/* DESCRIPTION
/* .nf

 /*
  * Utility library.
  */
#include <vstring.h>

 /*
  * External interface.
  */
#define RESOLVE_ADDR	"resolve"

#define RESOLVE_FLAG_FINAL	(1<<0)	/* final delivery */
#define RESOLVE_FLAG_ROUTED	(1<<1)	/* routed destination */
#define RESOLVE_FLAG_ERROR	(1<<2)	/* bad destination syntax */
#define RESOLVE_FLAG_FAIL	(1<<3)	/* request failed */

#define RESOLVE_CLASS_LOCAL	(1<<8)	/* mydestination/inet_interfaces */
#define RESOLVE_CLASS_ERROR	(1<<9)	/* virtual_alias_domains */
#define RESOLVE_CLASS_VIRTUAL	(1<<10)	/* virtual_mailbox_domains */
#define RESOLVE_CLASS_RELAY	(1<<11)	/* relay_domains */
#define RESOLVE_CLASS_DEFAULT	(1<<12)	/* raise reject_unauth_destination */

typedef struct RESOLVE_REPLY {
    VSTRING *transport;
    VSTRING *nexthop;
    VSTRING *recipient;
    int     flags;
} RESOLVE_REPLY;

extern void resolve_clnt_init(RESOLVE_REPLY *);
extern void resolve_clnt_query(const char *, RESOLVE_REPLY *);
extern void resolve_clnt_free(RESOLVE_REPLY *);

#define RESOLVE_CLNT_ASSIGN(reply, transport, nexthop, recipient) { \
	(reply).transport = (transport); \
	(reply).nexthop = (nexthop); \
	(reply).recipient = (recipient); \
    }

/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*--*/

#endif
