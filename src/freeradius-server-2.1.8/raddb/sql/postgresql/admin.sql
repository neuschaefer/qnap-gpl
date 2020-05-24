/*
 * admin.sql -- PostgreSQL commands for creating the RADIUS user.
 *
 *	WARNING: You should change 'localhost' and 'radpass'
 *		 to something else.  Also update raddb/sql.conf
 *		 with the new RADIUS password.
 *
 *	WARNING: This example file is untested.  Use at your own risk.
 *		 Please send any bug fixes to the mailing list.
 *
 *	$Id: admin.sql,v 1.1.1.1 2010/03/11 07:11:43 richardchang Exp $
 */

/*
 *  Create default administrator for RADIUS
 */
CREATE USER radius WITH PASSWORD 'radpass';

/*
 * The server can read any table in SQL
 */
GRANT SELECT ON radcheck TO radius;
GRANT SELECT ON radreply TO radius;
GRANT SELECT ON radgroupcheck TO radius;
GRANT SELECT ON radgroupreply TO radius;
GRANT SELECT ON radusergroup TO radius;

/*
 * The server can write to the accounting and post-auth logging table.
 */
GRANT ALL on radius.radacct TO radius;
GRANT ALL on radius.radpostauth TO radius;
