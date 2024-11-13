/**
 * @file    include/netsrv/except.h
 * NetSrv: Exception handling in C.
 * @author  Christoph Schulz
 * @since   2009-01-20
 * @version $Id$
 */

#ifndef INC_NETSRV_EXCEPT_H_
#define INC_NETSRV_EXCEPT_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <netsrv/defs.h>

#if !defined (NETSRV_MAX_NESTED_EXCEPTIONS)
/**
 * The maximum number of nested exceptions per function. A number of 2 allows
 * an exception handler to itself contain a try block with a nested exception
 * handler. Setting this value to zero is not allowed.
 * @note
 *   Increasing the value enlarges the stack size needed for each exception-
 *   aware non-void function, as it effectively determines the number of usable
 *   exception slots.
 */
#define NETSRV_MAX_NESTED_EXCEPTIONS 2
#endif

#if NETSRV_MAX_NESTED_EXCEPTIONS == 0
#error netsrv: NETSRV_MAX_NESTED_EXCEPTIONS must not be zero
#endif

/**
 * Exception handler.
 */
struct NETSRV_TRY_REGION
{
	/** Pointer to the exception handler (aka catch block). */
	void *handler;
	/** Pointer to the surrounding try region. */
	struct NETSRV_TRY_REGION *next;
};

/**
 * Initializes the exception frame, pointing to the default exception handler
 * which simply exits the function, returning the current exception.
 * @param RET
 *   The return exception to use in order to leave this function.
 * @internal
 */
#define EXCEPT_AWARE_COMMON(RET) \
	struct NETSRV_TRY_REGION _h = \
		{ && _except_handler_exit, (struct NETSRV_TRY_REGION *) NULL }; \
	struct NETSRV_TRY_REGION *_except = &_h; \
	goto _start; \
	_except_handler_exit: \
		RET; \
	_start:

/**
 * Marks the function with return type T as exception-aware. This macro has to
 * be used after the declaration of all top-level local variables.
 * @param T
 *   The return type of the function.
 */
#define EXCEPT_AWARE(T) \
	T _except_slots [NETSRV_MAX_NESTED_EXCEPTIONS]; \
	int _cur_except_slot = -1; \
	EXCEPT_AWARE_COMMON(return _except_slots [_cur_except_slot])

/**
 * Marks the function with return type void as exception-aware.
 */
#define EXCEPT_AWARE_VOID() \
	EXCEPT_AWARE_COMMON(return)

/**
 * Starts a try block. A try block has a body of code from which exceptions are
 * caught, and one associated exception handler (defined by EXCEPT()) for
 * handling those exceptions.
 * @param N
 *   The index of the try block. Within one function, all try block indices must
 *   be unique.
 */
#define TRY(N) \
	{ \
		struct NETSRV_TRY_REGION _h ## N = { && _except_handler ## N, _except }; \
		_except = &_h ## N;

/**
 * Throws an exception. Invokes the execution of the exception handler of the
 * nearest enclosing try block, i.e. a try block that has been entered via TRY()
 * but not left via ENDTRY(). If no enclosing try block exists, the function is
 * exited, returning the expression. If NETSRV_MAX_NESTED_EXCEPTIONS exceptions
 * are already pending (i.e. they have been caught by an exception handler but
 * that handler has not been left yet), an error message is printed to stderr
 * and the program is aborted.
 * @param E
 *   The expression to throw. This expression must conform to the return type of
 *   the function.
 * @note
 *   If the exception is to be passed to the exception handler of the
 *   surrounding active try block (if any), RETHROW() has to be used.
 */
#define THROW(E) \
	do { \
		/* cannot modify _cur_except_slot before assigning E to the slot,
		   as E might reference _cur_except_slot (e.g. in RETHROW()) */ \
		if (_cur_except_slot + 1 == NETSRV_MAX_NESTED_EXCEPTIONS) { \
			fprintf (stderr, netsrv_msg_exception_slots_exhaused, NETSRV_MAX_NESTED_EXCEPTIONS + 1); \
			abort (); \
		} \
		_except_slots [_cur_except_slot + 1] = (E); \
		++_cur_except_slot; \
		goto *_except->handler; \
	} while (0)

/**
 * Throws an exception. See THROW() for a more detailed explanation.
 * @note
 *   Calling THROWVOID() never leads to an abort as void functions do not need
 *   any exception slots.
 */
#define THROWVOID() \
	goto *_except->handler

/**
 * Rethrows the exception being currently handled by passing control to the
 * exception handler of the nearest enclosing try block. If no such try block
 * exists, the function is exited, returning the last exception thrown as the
 * result. If neither an enclosing try block nor an exception currently being
 * handled exists, the function is exited with an unpredictable return code.
 * @note
 *   This macro is equivalent to THROW(EXCEPTION()).
 */
#define RETHROW() \
	THROW (EXCEPTION())

/**
 * Returns the current exception. This is only meaningful in exception handlers
 * of non-void functions.
 * @return
 *   The value of the last exception thrown.
 * @warning
 *   If this macro is not called from within an exception handler, the resulting
 *   behaviour is undefined.
 */
#define EXCEPTION() \
	_except_slots [_cur_except_slot]

/**
 * Defines an exception handler. An exception handler is associated with one
 * try block and is invoked if the body of code being controlled by the try
 * block throws an exception via THROW(). An exception handler can be left
 * normally or by calling RETHROW(). In the first case, the control flow resumes
 * just after the end of the try block the exception handler is in. In the
 * second case, the control is passed to the exception handler of the nearest
 * enclosing try block. (See RETHROW() for a more detailed explanation.)
 * @param N
 *   The index of the exception handler. This index must be identical to the
 *   index of the associated try block.
 * @note
 *   An exception handler is not a "finally block", i.e. using "return" to exit
 *   a function does not invoke any exception handlers.
 */
#define EXCEPT(N) \
		goto _try_end ## N; \
	_except_handler ## N: \
		_except = _h ## N.next;

/**
 * Finishes a try block.
 * @param N
 *   The index of the try block to be finished.
 */
#define ENDTRY(N) \
		--_cur_except_slot; \
	_try_end ## N: \
		_except = _h ## N.next; \
	}

/**
 * Error message displayed when the number of exception slots has been exhausted
 * due to too many nested exceptions.
 */
extern const char netsrv_msg_exception_slots_exhaused [];

#endif
