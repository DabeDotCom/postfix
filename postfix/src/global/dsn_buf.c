/*++
/* NAME
/*	dsbuf 3
/* SUMMARY
/*	delivery status buffer
/* SYNOPSIS
/*	#include <dsn_buf.h>
/*
/*	typedef struct {
/* .in +4
/*		VSTRING *status;	/* RFC 3463 */
/*		VSTRING *action;	/* RFC 3464 */
/*		VSTRING	*mtype;		/* dns */
/*		VSTRING	*mname;		/* host or domain */
/*		VSTRING	*dtype;		/* smtp, x-unix */
/*		int	dcode;		/* RFC 2821, sysexits.h */
/*		VSTRING *dtext;		/* RFC 2821, sysexits.h */
/*		VSTRING	*reason;	/* informal text */
/* .in -4
/*	} DSN_BUF;
/*
/*	DSN_BUF	*dsb_create(void)
/*
/*	DSN_BUF	*dsb_update(dsb, status, action, mtype, mname, dtype, dcode,
/*				dtext, reason_fmt, ...)
/*	DSN_BUF	*dsb;
/*	const char *status;
/*	const char *action;
/*	const char *mtype;
/*	const char *mname;
/*	const char *dtype;
/*	int	dcode;
/*	const char *dtext;
/*	const char *reason_fmt;
/*
/*	DSN_BUF	*dsb_simple(dsb, status, reason_fmt, ...)
/*	DSN_BUF	*dsb;
/*	const char *status;
/*	const char *reason_fmt;
/*
/*	DSN_BUF	*dsb_unix(dsb, status, dcode, dtext, reason_fmt, ...)
/*	DSN_BUF	*dsb;
/*	const char *status;
/*	const char *reason_fmt;
/*
/*	DSN_BUF	*dsb_smtp(dsb, status, dcode, dtext, reason_fmt, ...)
/*	DSN_BUF	*dsb;
/*	const char *status;
/*	const char *reason_fmt;
/*
/*	DSN_BUF	*dsb_formal(dsb, status, action, mtype, mname, dtype, dcode,
/*				dtext)
/*	DSN_BUF	*dsb;
/*	const char *status;
/*	const char *action;
/*	const char *mtype;
/*	const char *mname;
/*	const char *dtype;
/*	int	dcode;
/*	const char *dtext;
/*
/*	DSN_BUF	*dsb_status(dsb, status)
/*	DSN_BUF	*dsb;
/*	const char *status;
/*
/*	void	dsb_reset(dsb)
/*
/*	void	dsb_free(dsb)
/* DESCRIPTION
/*	This module implements a simple to update delivery status
/*	buffer for Postfix-internal use. Typically it is populated
/*	in course of delivery attempt, and then formatted into a
/*	DSN structure for external notification.
/*
/*	dsb_create() creates initialized storage for formal RFC 3464
/*	attributes, and human-readable informal text.
/*
/*	dsb_update() updates all fields.
/*
/*	dsb_simple() updates the status and informal text, and resets all
/*	other fields to defaults.
/*
/*	dsb_unix() updates the status, diagnostic code, diagnostic
/*	text, and informal text, sets the diagnostic type to UNIX,
/*	and resets all other fields to defaults.
/*
/*	dsb_smtp() does the same for SMTP style diagnostics.
/*
/*	dsb_formal() updates all fields except the informal text.
/*
/*	dsb_status() updates the status field, and resets all
/*	formal fields to defaults.
/*
/*	dsb_reset() resets all fields in a DSN_BUF structure without
/*	deallocating memory.
/*
/*	dsb_free() recycles the storage that was allocated by
/*	dsb_create(), and so on.
/*
/*	Arguments:
/* .IP dsb
/*	Delivery status buffer.
/* .IP status
/*	RFC 3463 "enhanced" status code.
/* .IP action
/*	RFC 3464 action code; specify DSB_DEF_ACTION to derive the
/*	action from the status value. The only values that really
/*	matter here are "expanded" and "relayed"; all other values
/*	are already implied by the context.
/* .IP mtype
/*	The only valid type is DSB_MTYPE_DNS.  The macro DSB_SKIP_RMTA
/*	conveniently expands into a null argument list for the
/*	remote MTA type and name.
/* .IP mname
/*	Remote MTA name.
/* .IP dtype
/*	DSB_DTYPE_SMTP or DSB_DTYPE_UNIX.  The macro DSB_SKIP_REPLY
/*	conveniently expands into a null argument list for the reply
/*	type, code and text.
/* .IP dcode
/*	Numerical reply code. The reply code is reset when dtype is
/*	DSB_SKIP_REPLY.
/* .IP dtext
/*	The reply text. The reply text is reset when dtype is
/*	DSB_SKIP_REPLY.
/* .IP reply_fmt
/*	The reply text format. The reply text is reset when type
/*	is DSB_SKIP_REPLY.
/* .IP reason_fmt
/*	The informal reason format.
/* SEE ALSO
/*	msg(3) diagnostics interface
/* DIAGNOSTICS
/*	Fatal: out of memory.
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

/* System library. */

#include <sys_defs.h>
#include <stdlib.h>			/* 44BSD stdarg.h uses abort() */
#include <stdarg.h>
#include <string.h>

/* Utility library. */

#include <msg.h>
#include <mymalloc.h>
#include <vstring.h>

/* Global library. */

#include <dsn_buf.h>

/* Application-specific. */

#define STR(x)	vstring_str(x)

/* dsb_create - create delivery status buffer */

DSN_BUF *dsb_create(void)
{
    DSN_BUF *dsb;

    /*
     * Some fields aren't needed until we want to report an error.
     */
    dsb = (DSN_BUF *) mymalloc(sizeof(*dsb));
    dsb->status = vstring_alloc(10);
    dsb->action = vstring_alloc(10);
    dsb->mtype = vstring_alloc(10);
    dsb->mname = vstring_alloc(100);
    dsb->dtype = vstring_alloc(10);
    dsb->dcode = 0;
    dsb->dtext = vstring_alloc(100);
    dsb->reason = vstring_alloc(100);

    return (dsb);
}

/* dsb_free - destroy storage */

void    dsb_free(DSN_BUF *dsb)
{
    vstring_free(dsb->status);
    vstring_free(dsb->action);
    vstring_free(dsb->mtype);
    vstring_free(dsb->mname);
    vstring_free(dsb->dtype);
    vstring_free(dsb->dtext);
    vstring_free(dsb->reason);
    myfree((char *) dsb);
}

#define DSB_TRUNCATE(s) (STR(s)[0] = 0)

#define DSB_ACTION(dsb, stat, act) \
    vstring_strcpy((dsb)->action, (act) && *(act) ? (act) : "")

#define DSB_MTA(dsb, type, name) do { \
    if ((type) == 0) { \
	DSB_TRUNCATE((dsb)->mtype); \
	DSB_TRUNCATE((dsb)->mname); \
    } else { \
	vstring_strcpy((dsb)->mtype, (type)); \
	vstring_strcpy((dsb)->mname, (name)); \
    } \
} while (0)

/* dsb_update - update formal attributes and informal text */

DSN_BUF *dsb_update(DSN_BUF *dsb, const char *status, const char *action,
		            const char *mtype, const char *mname,
		            const char *dtype, int dcode, const char *dtext,
		            const char *format,...)
{
    va_list ap;

    vstring_strcpy(dsb->status, status);
    DSB_ACTION(dsb, status, action);
    DSB_MTA(dsb, mtype, mname);
    if (dtype == 0) {
	DSB_TRUNCATE(dsb->dtype);
	dsb->dcode = 0;
	DSB_TRUNCATE(dsb->dtext);
    } else {
	vstring_strcpy(dsb->dtype, dtype);
	dsb->dcode = dcode;
	vstring_strcpy(dsb->dtext, dtext);
    }
    va_start(ap, format);
    vstring_vsprintf(dsb->reason, format, ap);
    va_end(ap);

    return (dsb);
}

/* dsb_simple - update status and text */

DSN_BUF *dsb_simple(DSN_BUF *dsb, const char *status, const char *format,...)
{
    va_list ap;

    vstring_strcpy(dsb->status, status);
    DSB_TRUNCATE(dsb->action);
    DSB_TRUNCATE(dsb->mtype);
    DSB_TRUNCATE(dsb->mname);
    DSB_TRUNCATE(dsb->dtype);
    dsb->dcode = 0;
    DSB_TRUNCATE(dsb->dtext);
    va_start(ap, format);
    vstring_vsprintf(dsb->reason, format, ap);
    va_end(ap);

    return (dsb);
}

/* dsb_unix - update status, UNIX diagnostic and text */

DSN_BUF *dsb_unix(DSN_BUF *dsb, const char *status, int dcode,
		          const char *dtext, const char *format,...)
{
    va_list ap;

    vstring_strcpy(dsb->status, status);
    DSB_TRUNCATE(dsb->action);
    DSB_TRUNCATE(dsb->mtype);
    DSB_TRUNCATE(dsb->mname);
    vstring_strcpy(dsb->dtype, DSB_DTYPE_UNIX);
    dsb->dcode = dcode;
    vstring_strcpy(dsb->dtext, dtext);
    va_start(ap, format);
    vstring_vsprintf(dsb->reason, format, ap);
    va_end(ap);

    return (dsb);
}

/* dsb_smtp - update status, SMTP diagnostic and text */

DSN_BUF *dsb_smtp(DSN_BUF *dsb, const char *status, int dcode,
		          const char *dtext, const char *format,...)
{
    va_list ap;

    vstring_strcpy(dsb->status, status);
    DSB_TRUNCATE(dsb->action);
    DSB_TRUNCATE(dsb->mtype);
    DSB_TRUNCATE(dsb->mname);
    vstring_strcpy(dsb->dtype, DSB_DTYPE_SMTP);
    dsb->dcode = dcode;
    vstring_strcpy(dsb->dtext, dtext);
    va_start(ap, format);
    vstring_vsprintf(dsb->reason, format, ap);
    va_end(ap);

    return (dsb);
}

/* dsb_formal - update the formal fields */

DSN_BUF *dsb_formal(DSN_BUF *dsb, const char *status, const char *action,
		            const char *mtype, const char *mname,
		            const char *dtype, int dcode,
		            const char *dtext)
{
    vstring_strcpy(dsb->status, status);
    DSB_ACTION(dsb, status, action);
    DSB_MTA(dsb, mtype, mname);
    if (dtype == 0) {
	DSB_TRUNCATE(dsb->dtype);
	dsb->dcode = 0;
	DSB_TRUNCATE(dsb->dtext);
    } else {
	vstring_strcpy(dsb->dtype, dtype);
	dsb->dcode = dcode;
	vstring_strcpy(dsb->dtext, dtext);
    }
    return (dsb);
}

/* dsb_status - update the status, reset other formal fields */

DSN_BUF *dsb_status(DSN_BUF *dsb, const char *status)
{
    vstring_strcpy(dsb->status, status);
    DSB_TRUNCATE(dsb->action);
    DSB_TRUNCATE(dsb->mtype);
    DSB_TRUNCATE(dsb->mname);
    DSB_TRUNCATE(dsb->dtype);
    dsb->dcode = 0;
    DSB_TRUNCATE(dsb->dtext);
    return (dsb);
}

/* dsb_reset - reset all fields */

void    dsb_reset(DSN_BUF *dsb)
{
    DSB_TRUNCATE(dsb->status);
    DSB_TRUNCATE(dsb->action);
    DSB_TRUNCATE(dsb->mtype);
    DSB_TRUNCATE(dsb->mname);
    DSB_TRUNCATE(dsb->dtype);
    dsb->dcode = 0;
    DSB_TRUNCATE(dsb->dtext);
    DSB_TRUNCATE(dsb->reason);
}