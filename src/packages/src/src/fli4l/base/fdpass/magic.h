/**
 * @file    magic.h
 * Definition of IPC key.
 * @author  Christoph Schulz
 * @since   2015-02-16
 * @version $Id$
 */

/// Used for inter-process communication.
#define MAGIC ((((unsigned) 'C') << 8) + (unsigned) 'S')
