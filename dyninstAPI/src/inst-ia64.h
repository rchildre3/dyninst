/*
 * Copyright (c) 1996 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: inst-ia64.h,v 1.4 2002/06/03 18:17:14 tlmiller Exp $

#ifndef INST_IA64_H
#define INST_IA64_H

#include "common/h/Types.h"	// Address
#include "arch-ia64.h"		// instruction
class process;

class InsnAddr {
	public:
		/* prefix increment */
		InsnAddr operator++ ();

		/* prefix decrement */
		InsnAddr operator-- ();

		/* postfix increment */
		InsnAddr operator++ (int dummy);

		/* postfix decrement */
		InsnAddr operator-- (int dummy);

		/* sum of two InsnAddrs */
		friend InsnAddr operator + ( InsnAddr lhs, InsnAddr rhs );

		/* difference of two InsnAddrs */
		friend InsnAddr operator - ( InsnAddr lhs, InsnAddr rhs );

		friend bool operator < ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator <= ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator > ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator >= ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator == ( InsnAddr lhs, InsnAddr rhs );
		friend bool operator != ( InsnAddr lhs, InsnAddr rhs );

		static InsnAddr generateFromAlignedDataAddress( Address addr, process * p );
		bool writeMyBundleFrom( const unsigned char * savedCodeBuffer );
		bool saveMyBundleTo( unsigned char * savedCodeBuffer );
		bool saveBundlesTo( unsigned char * savedCodeBuffer, unsigned int numberOfBundles );
		bool replaceBundleWith( const IA64_bundle & bundle );
		bool replaceBundlesWith( const IA64_bundle * replacementBundles, unsigned int numberOfReplacementBundles );
		bool writeStringAtOffset( unsigned int offsetInBundles, const char * string, unsigned int length );

	private:
		InsnAddr( Address addr, process * p ) : encodedAddress( addr ), myProc( p ) { }
		Address encodedAddress;
		process * myProc;
}; /* end class InsnAddr */

#endif
