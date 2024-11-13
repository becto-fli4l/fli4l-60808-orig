/**
 * @file    test/except.c
 * Tests exception handling.
 * @author  Christoph Schulz
 * @since   2009-01-20
 * @version $Id$
 */

#include <netsrv/defs.h>
#include <netsrv/except.h>
#include <stdio.h>
#include "assert.h"

/**
 * Tests catching an exception within a function.
 */
int
func1 (void)
{
	EXCEPT_AWARE (int);
	TRY(1)
		THROW(1);
	EXCEPT(1)
		ASSERT (EXCEPTION() == 1, 8)
	ENDTRY(1)
	return 0;
}

/**
 * Tests throwing an exception from within a function.
 */
int
func2 (void)
{
	EXCEPT_AWARE (int);
	THROW(2);
}

/**
 * Tests rethrowing an exception being handled from within a function.
 */
int
func3 (void)
{
	EXCEPT_AWARE (int);
	TRY(1)
		THROW(3);
	EXCEPT(1)
		ASSERT (EXCEPTION() == 3, 10)
		RETHROW ();
	ENDTRY(1)
	return 0; /* never reached */
}

/**
 * Main application. Tests if exception handling works.
 * @return Zero if successful, an error code otherwise.
 */
int
main (int argc, char *argv[])
{
	EXCEPT_AWARE (int);
	UNUSED(argc);
	UNUSED(argv);

	TRY(1)
		THROW(42);
	EXCEPT(1)
		ASSERT (EXCEPTION() == 42, 1)
	ENDTRY(1)

	TRY(2)
		THROW(5);
	EXCEPT(2)
		ASSERT (EXCEPTION() == 5, 2)
		TRY(3)
			RETHROW();
		EXCEPT(3)
			ASSERT (EXCEPTION() == 5, 3)
		ENDTRY(3)
		ASSERT (EXCEPTION() == 5, 4)
		TRY(4)
			THROW(23);
		EXCEPT(4)
			ASSERT (EXCEPTION() == 23, 5)
		ENDTRY(4)
		ASSERT (EXCEPTION() == 5, 6)
	ENDTRY(2)

	ASSERT (func1() == 0, 7);
	ASSERT (func2() == 2, 9);
	ASSERT (func3() == 3, 11);
	return 0;
}
