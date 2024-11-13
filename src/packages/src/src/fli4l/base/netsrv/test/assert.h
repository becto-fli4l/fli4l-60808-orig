/**
 * @file    test/assert.h
 * Simple assertion macro.
 * @author  Christoph Schulz
 * @since   2009-01-20
 * @version $Id$
 */

#ifndef INC_NETSRV_TEST_ASSERT_H_
#define INC_NETSRV_TEST_ASSERT_H_

#define ASSERT(expr, rc) \
	if (!(expr)) { fprintf (stderr, "[%d] %s\n", __LINE__, # expr); return rc; }

#endif
